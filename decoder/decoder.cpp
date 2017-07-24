// decoder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ddraw/Recorder.h"
#include "../ddraw/ECPUSurface.h"

#include <string>
#include <iostream>

std::ifstream gf_control;
std::ifstream gf_stream;

std::vector<char> g_control;
std::map<int, std::unique_ptr<ECPUSurface>> g_surfaces;
std::map<int, int> g_surf_frames;

CircularBuffer g_streambuf;

const int file_chunk = 4 * 1024;
long long file_left = 0;
long long file_read = 0;

const int frame_length = 10000 / 30;

Encoder g_encoder(g_streambuf);

void ReadStreamThread()
{
	while (true)
	{
		int sz = static_cast<int>(std::min<long long>(file_chunk, file_left));
		if (g_streambuf.Space() > sz)
		{
			g_streambuf.ReadFromStream(gf_stream, sz);
			file_left -= sz;
		}
	}
}

void ProcessStruct(RecStructCreateSurface rs)
{
	g_surfaces[rs.handle] = std::make_unique<ECPUSurface>(rs.size.x, rs.size.y, nullptr, rs.handle, 0);
}

void ProcessStruct(RecStructFillSurface rs)
{
	if (rs.rect.sx > 0)
	{
		RECT tmp = rs.rect.toRECT();
		g_surfaces[rs.handle]->FillSurface(rs.color, &tmp);
	}
	else
	{
		g_surfaces[rs.handle]->FillSurface(rs.color, nullptr);
	}
}

void ProcessStruct(RecStructBlit rs)
{
	RECT sr = rs.s_rect.toRECT();
	RECT dr = rs.d_rect.toRECT();
	RECT* psr = (rs.s_rect.sx > 0) ? &sr : nullptr;
	RECT* pdr = (rs.d_rect.sx > 0) ? &dr : nullptr;
	g_surfaces[rs.s_handle]->BlitToCPUSurface(g_surfaces[rs.d_handle].get(), pdr, psr, rs.flags, nullptr);
}

void ProcessStruct(RecStructMirrorBlit rs)
{
	DDCOLORKEY ck{};
	ck.dwColorSpaceHighValue = rs.color;
	ck.dwColorSpaceLowValue = rs.color;
	g_surfaces[rs.s_handle]->SetColorKey(0, &ck);

	RECT sr = rs.s_rect.toRECT();
	RECT dr = rs.d_rect.toRECT();
	RECT* psr = (rs.s_rect.sx > 0) ? &sr : nullptr;
	RECT* pdr = (rs.d_rect.sx > 0) ? &dr : nullptr;

	DDBLTFX fx{};
	fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;

	g_surfaces[rs.s_handle]->BlitToCPUSurface(g_surfaces[rs.d_handle].get(), pdr, psr, rs.flags | DDBLT_KEYSRC, &fx);
}

void ProcessStruct(RecStructKeyedBlit rs)
{
	DDCOLORKEY ck{};
	ck.dwColorSpaceHighValue = rs.color;
	ck.dwColorSpaceLowValue = rs.color;
	g_surfaces[rs.s_handle]->SetColorKey(0, &ck);

	RECT sr = rs.s_rect.toRECT();
	RECT dr = rs.d_rect.toRECT();
	RECT* psr = (rs.s_rect.sx > 0) ? &sr : nullptr;
	RECT* pdr = (rs.d_rect.sx > 0) ? &dr : nullptr;

	g_surfaces[rs.s_handle]->BlitToCPUSurface(g_surfaces[rs.d_handle].get(), pdr, psr, rs.flags | DDBLT_KEYSRC, nullptr);
}

void ProcessStruct(RecStructModifySubSurface rs)
{
	std::vector<WORD> data(rs.rect.sx*rs.rect.sy);
	g_encoder.Decode(data.data(), data.size());
	DDSURFACEDESC2 desc{};
	g_surfaces[rs.handle]->Lock(nullptr, &desc, 0, 0);
	WORD* pDst = (WORD*)desc.lpSurface;
	WORD* pSrc = (WORD*)data.data();
	pDst += (rs.rect.y * desc.lPitch/sizeof(WORD)) + rs.rect.x;

    file_read += data.size();

	for (int i = 0; i < rs.rect.sy; i++)
	{
		for (int j = 0; j < rs.rect.sx; j++)
		{
			pDst[j] = *(pSrc++);
		}
		pDst += desc.lPitch/sizeof(WORD);
	}
	g_surfaces[rs.handle]->Unlock(nullptr);

    std::string s = "E:\\surf" + std::to_string(rs.handle) + "_f" + std::to_string(g_surf_frames[rs.handle]++) + ".bmp";
 //   g_surfaces[rs.handle]->SaveBMP(s);
}

void ProcessStruct(RecStructModifySurface rs)
{
	int sx = g_surfaces[rs.handle]->GetWidth();
	int sy = g_surfaces[rs.handle]->GetHeight();
	
    file_read += sx*sy;

	DDSURFACEDESC2 desc{};
	g_surfaces[rs.handle]->Lock(nullptr, &desc, 0, 0);
	WORD* pDst = (WORD*)desc.lpSurface;

	g_encoder.Decode(pDst, sx*sy);

	g_surfaces[rs.handle]->Unlock(nullptr);

    std::string s = "E:\\surf" + std::to_string(rs.handle) + "_f" + std::to_string(g_surf_frames[rs.handle]++) + ".bmp";
    g_surfaces[rs.handle]->SaveBMP(s);
}

void ProcessStruct(RecStructFrameTimestamp rs)
{
	static int ls = rs.timestamp;
    static int i = 0;
	int dt = rs.timestamp - ls;
	while (dt > (frame_length))
	{
		dt -= frame_length;
		ls += frame_length;

		g_surfaces[0]->SaveBMP("E:\\frame"+std::to_string(i++)+".bmp");
	}
}


bool ReadControls(char* pSrc, char* pEnd)
{
	while (pSrc < pEnd)
	{
		char opcode = *pSrc;
		switch (opcode)
		{
		case RS_CREATE_SURFACE:
			ProcessStruct(*reinterpret_cast<RecStructCreateSurface*>(pSrc));
			pSrc += sizeof(RecStructCreateSurface);
			break;
		case RS_FILL_SURFACE: ProcessStruct(*reinterpret_cast<RecStructFillSurface*>(pSrc));
			pSrc += sizeof(RecStructFillSurface);
			break; 
		case RS_BLIT: ProcessStruct(*reinterpret_cast<RecStructBlit*>(pSrc));
			pSrc += sizeof(RecStructBlit);
			break;
		case RS_BLIT_MIRRORED: ProcessStruct(*reinterpret_cast<RecStructMirrorBlit*>(pSrc));
			pSrc += sizeof(RecStructMirrorBlit);
			break;
		case RS_BLIT_KEYED: ProcessStruct(*reinterpret_cast<RecStructKeyedBlit*>(pSrc));
			pSrc += sizeof(RecStructKeyedBlit);
			break;
		case RS_MODIFY_SUB: ProcessStruct(*reinterpret_cast<RecStructModifySubSurface*>(pSrc));
			pSrc += sizeof(RecStructModifySubSurface);
			break;
		case RS_MODIFY: ProcessStruct(*reinterpret_cast<RecStructModifySurface*>(pSrc));
			pSrc += sizeof(RecStructModifySurface);
			break;
		case RS_TIMESTAMP: ProcessStruct(*reinterpret_cast<RecStructFrameTimestamp*>(pSrc));
			pSrc += sizeof(RecStructFrameTimestamp);
			break;
		default: return false;
		}
	}
	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 3) return 1;
	std::string if_name = argv[1];
	std::string ifs_name = if_name;
	ifs_name.back() = 's';

	gf_control.open(if_name.c_str(), std::ios_base::binary | std::ios_base::ate);
	if (!gf_control)
		return 1;

	gf_stream.open(ifs_name.c_str(), std::ios_base::binary | std::ios_base::ate);
	if (!gf_stream)
		return 1;

	int control_size = static_cast<int>(gf_control.tellg());
	gf_control.close();
	gf_control.clear();

	file_left = gf_stream.tellg();
    long long file_orig = file_left;
	gf_stream.close();
	gf_stream.clear();

	g_control.resize(control_size);
	gf_control.open(if_name.c_str(), std::ios_base::binary);
	gf_control.read(g_control.data(), control_size);
	gf_control.close();

	gf_stream.open(ifs_name.c_str(), std::ios_base::binary);

	std::thread stream_thread(ReadStreamThread);
	stream_thread.detach();

   /* RecStructCreateSurface rsc(0, 0, RecVect{ 1920 * 2,1080 * 2 });
    ProcessStruct(rsc);

    RecStructModifySurface rss(0);
    while(true)
        ProcessStruct(rss);*/
	bool check = ReadControls(g_control.data(), g_control.data() + control_size);

    file_orig -= file_left;
    return 0;
}

