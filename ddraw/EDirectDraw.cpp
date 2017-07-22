#include "stdafx.h"
#include "EDirectDraw.h"

#include "ESurface.h"
#include "EPrimarySurface.h"
#include "ECPUSurface.h"
#include "EClipper.h"
#include<d3d.h>

#include "FBSurface.h"

#include "Recorder.h"
#include "SurfaceProcessor.h"


void RunRecordingThread()
{
	while (true)
	{
       // global_recording_buffer.WaitForData();
		global_recording_buffer.Save();
        Sleep(10);
	}
}

Encoder          gEx{ global_recording_buffer };

void RunEncodingThread()
{
	while (true)
	{
       // global_encoding_buffer.WaitForData();
        global_processing_buffer.Encode(gEx);
        Sleep(10);
    }
}

void RunPreprocessingThread()
{
    while (true)
    {
      //  global_processing_buffer.WaitForData();
        //gSP.Process();
        gSP.ClearQueue();
        Sleep(10);
    }
}

EDirectDraw::EDirectDraw(int xr, int yr, IDirectDraw7* pF) : xres(xr), yres(yr), m_cRef(1),
   pFB(pF)
{
	//IID_IDirect3DRGBDevice
	//IDirect3DDevice7
	//IID_IDirectDrawGammaControl
	CLEAR_LOG;
	LOG("EDirectDraw created\r\n");

	if (global_is_recording)
	{
		std::thread recorder_thread(RunRecordingThread);
		recorder_thread.detach();

		std::thread encorder_thread(RunEncodingThread);
		encorder_thread.detach();

        //std::thread preproc_thread(RunPreprocessingThread);
       // preproc_thread.detach();
	}
}


EDirectDraw::~EDirectDraw()
{
	mGraphics.Shutdown();
}


STDMETHODIMP EDirectDraw::Compact()
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::CreateClipper(DWORD flags, LPDIRECTDRAWCLIPPER *pClipper, IUnknown *)
{
	(*pClipper) = new EClipper();
	force_log("CLIPPER\r\n");
	return DD_OK;
}

STDMETHODIMP EDirectDraw::CreatePalette(DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE *, IUnknown *)
{
	force_log("CLIPPER\r\n");

	return E_NOTIMPL;
}

//{ F5049E77 - 4861 - 11D2 - A407 - 00A0C90629A8 }

STDMETHODIMP EDirectDraw::CreateSurface(LPDDSURFACEDESC2 pDesc, LPDIRECTDRAWSURFACE7 *pSurf, IUnknown *piu)
{
	/*pFB->CreateSurface(pDesc, pSurf, piu);
	FBSurface *pS = new FBSurface(*pSurf, chandle++);
	if (pDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
	{
		pS->bPrimary = true;
	}
	(*pSurf) = pS;
	return DD_OK;*/
	LOG("EDirectDraw::CreateSurface - ");
	//DDSCAPS_ALPHA;
	int xs = pDesc->dwWidth;// pDesc->dw
	int ys = pDesc->dwHeight;
	int bb = pDesc->dwDepth;
	static int index = 0;

	if (pDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
	{
		LOG("Primary\r\n");

		auto surf = CreatePrimarySurface(xs,ys,S_PRIMARY);
		((ESurface*)(surf))->SetFlags(pDesc->ddsCaps.dwCaps);
		((ESurface*)(surf))->mIndex = index++;
		*pSurf = surf;


		return DD_OK;
	}

	if (pDesc->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	{
		auto surf = CreateCPUSurface(xs, ys, S_SYSMEM);
		((ESurface*)(surf))->SetFlags(pDesc->ddsCaps.dwCaps);
		((ESurface*)(surf))->mIndex = index++;
		*pSurf = surf;
		return DD_OK;
	}

	LOG("Regular\r\n");

	auto surf = CreateSurface(xs,ys);
	((ESurface*)(surf))->SetFlags(pDesc->ddsCaps.dwCaps);
	((ESurface*)(surf))->mIndex = index++;
	*pSurf = surf;
//	surf->QueryInterface(GUID{}, (LPVOID*)pSurf);
	return DD_OK;
}

STDMETHODIMP EDirectDraw::DuplicateSurface(LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 *)
{
	force_log("DUPE\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2)
{
	force_log("ENUM\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::EnumSurfaces(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::FlipToGDISurface()
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetCaps(LPDDCAPS hard, LPDDCAPS soft)
{
	/*HRESULT hr = pFB->GetCaps(hard, soft);
	DDCAPS cc{};
	FillCaps(&cc);
	return hr;*/
	LOG("EDirectDraw::GetCaps\r\n");

	IDirectDrawGammaControl;

    if (hard)
		FillCaps(hard);
	if (soft)
		FillCaps(soft);

	return DD_OK;
}

STDMETHODIMP EDirectDraw::GetDisplayMode(LPDDSURFACEDESC2)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetFourCCCodes(LPDWORD, LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetGDISurface(LPDIRECTDRAWSURFACE7 *)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetMonitorFrequency(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetScanLine(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetVerticalBlankStatus(LPBOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::Initialize(GUID *)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::RestoreDisplayMode()
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::SetCooperativeLevel(HWND hWindow, DWORD dwFlags)
{
	//return pFB->SetCooperativeLevel(hWindow, dwFlags);
   LOG("EDirectDraw::SetCooperativeLevel\r\n");
   if (mGraphics.Initialize(hWindow, xres, yres))
   {
       mGraphics.InitDisplay();
	   //mGraphics.RunFrameCycle(0, 30);
	   return DD_OK;
   }
	LOG("EDirectDraw::SetCooperativeLevel FAILED\r\n");
	return E_INVALIDARG;
}

STDMETHODIMP EDirectDraw::SetDisplayMode(DWORD dwH, DWORD dwW, DWORD dwBPP, DWORD f, DWORD x)
{
	//return pFB->SetDisplayMode(dwH, dwW, dwBPP, f, x);
	LOG("EDirectDraw::SetDisplayMode\r\n");
	mGraphics.SetDisplayMode(dwH, dwW, dwBPP);
	return DD_OK;
}

STDMETHODIMP EDirectDraw::WaitForVerticalBlank(DWORD, HANDLE)
{
	force_log("WFW\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::GetAvailableVidMem(LPDDSCAPS2 caps, LPDWORD total, LPDWORD free)
{
	//return pFB->GetAvailableVidMem(caps, total, free);
	LOG("EDirectDraw::GetAvailableVidMem\r\n");

	*total = mGraphics.VideoMemory();
	*free  = mGraphics.VideoMemory();

	LOG(*total);
	LOG("\r\n");
	LOG(*free);
	LOG("\r\n");
	
	return DD_OK;
}

STDMETHODIMP EDirectDraw::GetSurfaceFromDC(HDC, LPDIRECTDRAWSURFACE7 *)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::RestoreAllSurfaces()
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::TestCooperativeLevel()
{
	return pFB->TestCooperativeLevel();
}

STDMETHODIMP EDirectDraw::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::StartModeTest(LPSIZE, DWORD, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EDirectDraw::EvaluateMode(DWORD, DWORD *)
{
	return E_NOTIMPL;
}
//{69C11C3E - B46B - 11D1 - AD7A - 00C04FC29B4E}
void EDirectDraw::FillCaps(LPDDCAPS caps)
{
	DDCAPS &c = (*caps); // 52436c
	c = DDCAPS{};
	c.dwSize = sizeof(c);
	/*c.dwCaps = DDCAPS_3D | DDCAPS_ALPHA | DDCAPS_BLT | DDCAPS_BLTCOLORFILL | DDCAPS_BLTDEPTHFILL
						 | DDCAPS_BLTFOURCC | DDCAPS_BLTQUEUE | DDCAPS_BLTSTRETCH 
						 | DDCAPS_CANCLIP | DDCAPS_CANCLIPSTRETCHED | DDCAPS_COLORKEY
						 | DDCAPS_OVERLAY | DDCAPS_OVERLAYFOURCC | DDCAPS_OVERLAYSTRETCH
						 | DDCAPS_PALETTE | DDCAPS_PALETTEVSYNC | DDCAPS_READSCANLINE
		                 | DDCAPS_VBI | DDCAPS_ZBLTS | DDCAPS_ZOVERLAYS | DDCAPS_BANKSWITCHED;*/
	c.dwCaps = DDCAPS_3D | DDCAPS_BLT | DDCAPS_BLTQUEUE | DDCAPS_BLTFOURCC
		| DDCAPS_BLTSTRETCH | DDCAPS_GDI | DDCAPS_OVERLAY | DDCAPS_OVERLAYCANTCLIP
		| DDCAPS_OVERLAYFOURCC | DDCAPS_OVERLAYSTRETCH | DDCAPS_READSCANLINE | DDCAPS_ZBLTS
		| DDCAPS_COLORKEY | DDCAPS_ALPHA | DDCAPS_COLORKEYHWASSIST | DDCAPS_BLTCOLORFILL | DDCAPS_CANBLTSYSMEM;
	// No GDI sharing here

	//c.dwCaps2 = DDCAPS2_CANMANAGETEXTURE | DDCAPS2_CANMANAGERESOURCE | DDCAPS2_WIDESURFACES | DDCAPS2_DYNAMICTEXTURES;
	/*c.dwCaps2 = DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_NONLOCALVIDMEM 
		| DDCAPS2_WIDESURFACES | DDCAPS2_CANFLIPODDEVEN | DDCAPS2_CANFLIPODDEVEN 
		| DDCAPS2_PRIMARYGAMMA | DDCAPS2_CANRENDERWINDOWED | DDCAPS2_FLIPINTERVAL
		| DDCAPS2_FLIPNOVSYNC | DDCAPS2_DYNAMICTEXTURES | DDCAPS2_CANSHARERESOURCE;
	*/
	c.dwCaps2 = 2691346992;
	c.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER 
		| DDSCAPS_OFFSCREENPLAIN | DDSCAPS_OVERLAY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_TEXTURE 
		| DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_ZBUFFER 
		| DDSCAPS_OWNDC	| DDSCAPS_MIPMAP | DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM;
	
	c.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP;

	c.ddsOldCaps.dwCaps = 809923324;

	c.dwCKeyCaps = 0x2e310;
	c.dwFXCaps = 0x22AD54E0;


	c.dwFXAlphaCaps = 0;// DDFXALPHACAPS_BLTALPHAEDGEBLEND | DDFXALPHACAPS_BLTALPHASURFACES;
	c.dwPalCaps = 0;// DDPCAPS_8BIT | DDPCAPS_ALPHA | DDPCAPS_ALLOW256 | DDPCAPS_VSYNC;
	c.dwAlphaBltConstBitDepths = c.dwAlphaBltPixelBitDepths = c.dwAlphaBltSurfaceBitDepths = 0;// DDBD_8;

	c.dwVidMemTotal = mGraphics.VideoMemory();
	c.dwVidMemFree = mGraphics.VideoMemory();

	c.dwSVBCaps = 0x00000040;
	c.dwVSBCaps = 0x00000040;

	c.dwNLVBCaps = 0x85d27fc1;
	c.dwNLVBCaps2 = 0xa062b230;
	c.dwNLVBCKeyCaps = 0x0002e310;
	c.dwNLVBFXCaps = 0x22ad54e0;

	c.dwMaxVisibleOverlays = 1;
	c.dwCurrVisibleOverlays = 0;
	c.dwNumFourCCCodes = 0x12;
	c.dwAlignBoundarySrc = 0;
	c.dwAlignSizeSrc = 0;
	c.dwAlignBoundaryDest = 0;
	c.dwAlignSizeDest = 0;
	c.dwAlignStrideAlign = 0;
	
	c.dwMinOverlayStretch = 0x20;
	c.dwMaxOverlayStretch = 0x001f4000;

	c.dwMaxVideoPorts = 0;
	c.dwCurrVideoPorts = 0;
}

IDirectDrawSurface7 * EDirectDraw::CreateSurface(int xs, int ys, int flags)
{
	Handle ps = mGraphics.CreateSurface(xs,ys,flags);

	if (global_is_recording)
	{
		RecStructCreateSurface rs(ps, flags, { xs,ys });
		record_struct(rs);
	}

	return new ESurface(&mGraphics, ps, flags);
}

IDirectDrawSurface7 * EDirectDraw::CreatePrimarySurface(int x_size, int y_size, int flags)
{
	Handle ps = mGraphics.CreateSurface(x_size, y_size, flags);

	if (global_is_recording)
	{
		RecStructCreateSurface rs(ps, flags, { xres,yres });
		record_struct(rs);
        gSP.CreateTexture(ps, xres, yres);
	}

	mGraphics.SetDisplayReady(true);
	return new EPrimarySurface(&mGraphics, ps, flags);
}

IDirectDrawSurface7 * EDirectDraw::CreateCPUSurface(int x_size, int y_size, int flags)
{
	Handle ps = mGraphics.GetNewHandle();

	if (global_is_recording)
	{
		RecStructCreateSurface rs(ps, flags, { x_size,y_size });
		record_struct(rs);
        gSP.CreateTexture(ps, x_size, y_size);
	}

	return new ECPUSurface(x_size, y_size, &mGraphics, ps, flags);
}
