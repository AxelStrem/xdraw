#include "stdafx.h"
#include "ECPUSurface.h"
#include <algorithm>
#include <utility>
#include <numeric>
#include <memory>
#include <iterator>
#include "Recorder.h"
#include "SurfaceProcessor.h"

long long g_datarec = 0;

HRESULT SoftwareBlit(WORD* pDstOffset, int dst_pitch, WORD* pSrcOffset, int src_pitch, int blit_w, int blit_h, DWORD flags, LPDDBLTFX fx)
{
	for (int i = 0; i < blit_h; i++)
	{
  		memcpy(pDstOffset, pSrcOffset, sizeof(WORD)*blit_w);
		pSrcOffset += src_pitch;
		pDstOffset += dst_pitch;
	}
	return DD_OK;
}

HRESULT SoftwareBlitKeyed(WORD* pDstOffset, int dst_pitch, WORD* pSrcOffset, int src_pitch, int blit_w, int blit_h, WORD key_low)
{
	for (int i = 0; i < blit_h; i++)
	{
		for (int j = 0; j < blit_w; j++)
		{
			WORD w = pSrcOffset[j];
			if (w != key_low)
			{
				pDstOffset[j] = w;
			}
		}
		pSrcOffset += src_pitch;
		pDstOffset += dst_pitch;
	}
	return DD_OK;
}


HRESULT SoftwareBlitMirrored(WORD* pDstOffset, int dst_pitch, WORD* pSrcOffset, int src_pitch, int blit_w, int blit_h, WORD key_low)
{
	for (int i = 0; i < blit_h; i++)
	{
		for (int j = 0; j < blit_w; j++)
		{
			WORD w = pSrcOffset[blit_w - 1 -j];
			if (w != key_low)
			{
				pDstOffset[j] = w;
			}
		}
		pSrcOffset += src_pitch;
		pDstOffset += dst_pitch;
	}
	return DD_OK;
}


ECPUSurface::ECPUSurface(int x_size, int y_size, Graphics * host, Handle handle_, int flags) : ESurface(host, handle_, flags), xs(x_size), ys(y_size)
{
	data.resize(xs*ys);
}

ECPUSurface::~ECPUSurface()
{
}

STDMETHODIMP ECPUSurface::Lock(LPRECT rct, LPDDSURFACEDESC2 desc, DWORD flags, HANDLE h)
{
	GetSurfaceDesc(desc);

	if (h)
	{
		RECT rh = *(reinterpret_cast<RECT*>(h));
		rh.bottom += rh.top;
		rh.right += rh.left;
		lock_rects.push_back(rh);
	}

	if ((mIndex == 3)&&(!h))
	{
		OutputDebugString(L"LOL");
	}

	desc->dwFlags |= DDSD_PITCH;
	desc->lpSurface = data.data();
	desc->lPitch = xs*2;
	return DD_OK;
}

STDMETHODIMP ECPUSurface::Unlock(LPRECT)
{
	if (lock_rects.size())
	{
	/*	if (mIndex == 3)
		{
			RECT r = lock_rects.back();
			FillSurface(0xF000, &r);
		}*/
	/*	else
		if (mIndex == 3)
		{
			RECT r = lock_rects.back();
			FillSurface(0x001F, &r);
		}*/	
	/*	else
		{
			RECT r = lock_rects.back();
			WORD x = (743*mIndex) & 0xFFFF;
			FillSurface(x, &r);
		}*/
		if (global_is_recording)
		{
			RECT r = lock_rects.back();
			if (r.right > xs) r.right = xs;
			if (r.left > xs) r.left = xs;
			if (r.top > ys) r.top = ys;
			if (r.bottom > ys) r.bottom = ys;

			WORD* pS = data.data();
			pS += xs*r.top + r.left;

            RecStructModifySubSurface rs(mHandle, &r);
            rs.data_rec = g_datarec;
            g_datarec += rs.rect.getSize();
            record_struct(rs);

			record_stream(pS, r.right - r.left,
				r.bottom - r.top, xs);

            gSP.ModifyRect(mHandle, &r);
		}
		lock_rects.clear();
	}
	else if (global_is_recording)
	{
		RecStructModifySurface rs(mHandle);
        rs.data_rec = g_datarec;
        g_datarec += xs*ys;
		record_struct(rs);

		WORD* pS = data.data();
		record_stream(pS, xs, ys, xs);

        gSP.ModifyRect(mHandle, RecRect{ 0,0,xs,ys });
	}

	return DD_OK;
}

STDMETHODIMP ECPUSurface::Blt(LPRECT pDstRect, LPDIRECTDRAWSURFACE7 pSurf, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx)
{
//	force_log_r("BLT ECPU : ", _flags, "\r\n");

	if (!pSurf)
	{
		if (flags&DDBLT_COLORFILL)
		{
			if (global_is_recording)
			{
				RecStructFillSurface rs(mHandle, flags, pDstRect, fx->dwFillColor);
				record_struct(rs);
			}

			FillSurface(static_cast<WORD>(fx->dwFillColor), pDstRect);
		}
		 return DD_OK;
	}

	ESurface* pS = reinterpret_cast<ESurface*>(pSurf);

	return pS->BlitToCPUSurface(this, pDstRect, pSrcRect, flags, fx);
}

STDMETHODIMP ECPUSurface::BltBatch(LPDDBLTBATCH, DWORD, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ECPUSurface::BltFast(DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pSurf, LPRECT pSrcRect, DWORD flags)
{
	//force_log_r("BLTFAST ECPU: ", _flags, "\r\n");

	if (!pSurf)
	{
		return DD_OK;
	}

	ESurface* pS = reinterpret_cast<ESurface*>(pSurf);
	return pS->FastBlitToCPUSurface(this, x, y, pSrcRect, flags);
}


DWORD PrepareOffsets(WORD** pDstOffset, WORD** pSrcOffset, int dst_pitch, int src_pitch, int& blit_w, int& blit_h, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx)
{
	if (pDstRect)
	{
		/*if (pDstRect->bottom < pDstRect->top)
		{
			std::swap(pDstRect->bottom, pDstRect->top);
		}

		if (pDstRect->right < pDstRect->left)
		{
			std::swap(pDstRect->right, pDstRect->left);
		}*/

		*pDstOffset += pDstRect->top * dst_pitch + pDstRect->left;
		blit_w = std::min<int>(blit_w, pDstRect->right - pDstRect->left);
		blit_h = std::min<int>(blit_h, pDstRect->bottom - pDstRect->top);
	}

	if (pSrcRect)
	{
		/*if (pSrcRect->bottom < pSrcRect->top)
		{
			std::swap(pSrcRect->bottom, pSrcRect->top);
		}

		if (pSrcRect->right < pSrcRect->left)
		{
			std::swap(pSrcRect->right, pSrcRect->left);
		}*/

		*pSrcOffset += pSrcRect->top * src_pitch + pSrcRect->left;

		blit_w = std::min<int>(blit_w, pSrcRect->right - pSrcRect->left);
		blit_h = std::min<int>(blit_h, pSrcRect->bottom - pSrcRect->top);
	}

	return flags;
}


HRESULT ECPUSurface::BlitToCPUSurface(ECPUSurface * pSurf, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx)
{
	WORD* pDstOffset = pSurf->data.data();
	WORD* pSrcOffset = data.data();

	std::vector<WORD> tmp;
	if (pSurf == this)
	{
	//	Snapshot("E:\s3.bmp");
	//	FillSurface(0x000F, nullptr);
	//	return DD_OK;

		tmp = std::move(data);
		data.clear();
		data.resize(xs*ys);
		pDstOffset = data.data();
		pSrcOffset = tmp.data();
	}
	
	int blit_w = xs;
	int blit_h = ys;
	//RECT src_rect = *pSrcRect;
	//RECT dst_rect = *pDstRect;

	//flags = PrepareOffsets(&pDstOffset, &pSrcOffset, pSurf->xs, xs, blit_w, blit_h, &src_rect, &dst_rect, flags, fx);
	int dst_pitch = pSurf->xs;
	int src_pitch = xs;

	if (pDstRect)
	{
		pDstOffset += pDstRect->top * dst_pitch + pDstRect->left;
		blit_w = std::min<int>(blit_w, pDstRect->right - pDstRect->left);
		blit_h = std::min<int>(blit_h, pDstRect->bottom - pDstRect->top);
	}

	if (pSrcRect)
	{
		pSrcOffset += pSrcRect->top * src_pitch + pSrcRect->left;

		blit_w = std::min<int>(blit_w, pSrcRect->right - pSrcRect->left);
		blit_h = std::min<int>(blit_h, pSrcRect->bottom - pSrcRect->top);
	}

	/*static std::set<DWORD> all_flags;
	if (all_flags.find(flags) == all_flags.end())
	{
		all_flags.insert(flags);
		force_log_r("new blit flags : ", flags, "\r\n");
	}*/

	if ((flags & DDBLT_KEYSRC) && color_keyed)
	{
		if ((flags & DDBLT_DDFX) && (fx->dwDDFX & DDBLTFX_MIRRORLEFTRIGHT))
		{
			SoftwareBlitMirrored(pDstOffset, pSurf->xs, pSrcOffset, xs, blit_w, blit_h, static_cast<WORD>(key_low));

			if (global_is_recording)
			{
				RecStructMirrorBlit rs(pSurf->mHandle, mHandle, flags, pDstRect, pSrcRect, static_cast<WORD>(key_low));
				record_struct(rs);
			}
		}
		else
		{
			SoftwareBlitKeyed(pDstOffset, pSurf->xs, pSrcOffset, xs, blit_w, blit_h, static_cast<WORD>(key_low));
			if (global_is_recording)
			{
				RecStructKeyedBlit rs(pSurf->mHandle, mHandle, flags, pDstRect, pSrcRect, static_cast<WORD>(key_low));
				record_struct(rs);
			}
		}
	}
	else
	{
		SoftwareBlit(pDstOffset, pSurf->xs, pSrcOffset, xs, blit_w, blit_h, flags, fx);
		if (global_is_recording)
		{
			RecStructBlit rs(pSurf->mHandle, mHandle, flags, pDstRect, pSrcRect);
			record_struct(rs);
		}
	}

	
	return DD_OK;
}

HRESULT ECPUSurface::FastBlitToCPUSurface(ECPUSurface * pSurf, DWORD x, DWORD y, LPRECT pSrcRect, DWORD flags)
{
	RECT rct{ static_cast<LONG>(x),static_cast<LONG>(y),xs,ys };
	if (pSrcRect)
	{
		rct.right = x + (pSrcRect->right - pSrcRect->left);
		rct.bottom = y + (pSrcRect->bottom - pSrcRect->top);
	}
	return BlitToCPUSurface(pSurf, &rct, pSrcRect, flags, nullptr);
}


HRESULT ECPUSurface::BlitToESurface(ESurface * pSurf, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx)
{
	auto smem = pHost->LockSurface(pSurf->mHandle, pDstRect);

	//DDSURFACEDESC2 tmp{};
	//pSurf->Lock(pDstRect, &tmp, 0, 0);

	WORD* pDstOffset = reinterpret_cast<WORD*>(smem.pMem);
	WORD* pSrcOffset = data.data();

	int blit_w = xs;
	int blit_h = ys;

	//RECT src_rect = *pSrcRect;
	//RECT dst_rect = *pDstRect;

	//flags = PrepareOffsets(&pDstOffset, &pSrcOffset, (tmp.lPitch / sizeof(WORD)), xs, blit_w, blit_h, &src_rect, &dst_rect, flags, fx);

	int dst_pitch = (smem.pitch / sizeof(WORD));
	int src_pitch = xs;

	if (pDstRect)
	{
		pDstOffset += pDstRect->top * dst_pitch + pDstRect->left;
		blit_w = std::min<int>(blit_w, pDstRect->right - pDstRect->left);
		blit_h = std::min<int>(blit_h, pDstRect->bottom - pDstRect->top);
	}

	if (pSrcRect)
	{
	    pSrcOffset += pSrcRect->top * src_pitch + pSrcRect->left;

		blit_w = std::min<int>(blit_w, pSrcRect->right - pSrcRect->left);
		blit_h = std::min<int>(blit_h, pSrcRect->bottom - pSrcRect->top);
	}
	

	if ((flags & DDBLT_KEYSRC) && color_keyed)
	{
		SoftwareBlitKeyed(pDstOffset, (smem.pitch / sizeof(WORD)), pSrcOffset, xs, blit_w, blit_h, static_cast<WORD>(key_low));

		if (global_is_recording)
		{
			RecStructKeyedBlit rs(pSurf->mHandle, mHandle, flags, pDstRect, pSrcRect, key_low);
			record_struct(rs);
		}
	}
	else
	{
		SoftwareBlit(pDstOffset, (smem.pitch / sizeof(WORD)), pSrcOffset, xs, blit_w, blit_h, flags, fx);

		if (global_is_recording)
		{
			RecStructBlit rs(pSurf->mHandle, mHandle, flags, pDstRect, pSrcRect);
			record_struct(rs);
		}
	}

	pSurf->Unlock(pDstRect);
	return DD_OK;
}

HRESULT ECPUSurface::FastBlitToESurface(ESurface * pSurf, DWORD x, DWORD y, LPRECT pSrcRect, DWORD flags)
{
    RECT rct{ static_cast<LONG>(x),static_cast<LONG>(y),xs,ys };
	if (pSrcRect)
	{
		rct.right = x + (pSrcRect->right - pSrcRect->left);
		rct.bottom = y + (pSrcRect->bottom - pSrcRect->top);
	}
	/*static std::set<DWORD> all_flags;
	if (all_flags.find(flags) == all_flags.end())
	{
		all_flags.insert(flags);
		force_log_r("new fastblit flags : ", flags, "\r\n");
	}*/
	return BlitToESurface(pSurf, &rct, pSrcRect, flags, nullptr);
}

void ECPUSurface::FillSurface(WORD color, LPRECT pRect)
{
	if(!pRect) for (WORD& w : data) w = color;
	else
	{
		int w = pRect->right - pRect->left;
		int h = pRect->bottom - pRect->top;

		w = std::min<int>(w, xs - pRect->left);
		h = std::min<int>(h, ys - pRect->top);

		int offset = pRect->top*xs + pRect->left;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
				data[offset + j] = color;
			offset += xs;
		}
	}
//	memset(data.data(), (color<<16) | color, data.size()*sizeof(WORD));
}

COLOR_3B convert16to32(WORD b16)
{
	COLOR_3B result;
	result.r = ((b16&RED_MASK) >> RED_SHIFT) << (8 - RED_WIDTH);
	result.g = ((b16&GREEN_MASK) >> GREEN_SHIFT) << (8 - GREEN_WIDTH);
	result.b = ((b16&BLUE_MASK) >> BLUE_SHIFT) << (8 - BLUE_WIDTH);
	return result;
}

void ECPUSurface::Snapshot(std::string fname)
{
	b.SetSize(xs, ys);
	for (auto& p : b)
	{
		p.value = convert16to32(data[p.x + p.y*xs]);
	}
	b.Save(fname);
}

void ECPUSurface::SaveBMP(std::string fname)
{
	b.SetSize(xs, ys);
	WORD* pmem = reinterpret_cast<WORD*>(data.data());
	WORD* lmem = pmem;
	for (auto p : b)
	{
		if (p.x == 0)
		{
			lmem = pmem;
			pmem += xs;
		}
		WORD w = *lmem++;
		p.value.r = ((w & RED_MASK) >> RED_SHIFT) << (8 - RED_WIDTH);
		p.value.g = ((w & GREEN_MASK) >> GREEN_SHIFT) << (8 - GREEN_WIDTH);
		p.value.b = ((w & BLUE_MASK) >> BLUE_SHIFT) << (8 - BLUE_WIDTH);
	}

	b.Save(fname); 
}
