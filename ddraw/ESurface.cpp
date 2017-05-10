#include "stdafx.h"
#include "ESurface.h"
#include "EGammaControl.h"

int frame = 0;

ESurface::ESurface(Graphics* host, Handle h, int flags_) : pHost(host),mHandle(h),flags(flags_)
{
	caps.dwCaps = DDSCAPS_3DDEVICE;
	if (flags & S_PRIMARY)
	{
		caps.dwCaps |= DDSCAPS_VISIBLE;
	}

	if (flags & S_SYSMEM)
	{
		caps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}
	else
	{
		caps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	}
	auto s = host->GetSurfaceData(h);
	b.SetSize(s.x, s.y);
}


ESurface::~ESurface()
{
	pHost->DestroySurface(mHandle);
}


STDMETHODIMP ESurface::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!ppvObj)
		return E_INVALIDARG;
	*ppvObj = NULL;

	/*if (riid == IID_IDirectDrawGammaControl)
	{
		(*ppvObj) = new EGammaControl(nullptr);
		return DD_OK;
	}*/

	return COMRefCounter::QueryInterface(riid, ppvObj);
}

STDMETHODIMP ESurface::AddAttachedSurface(LPDIRECTDRAWSURFACE7)
{
	force_log("ATTACH\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::AddOverlayDirtyRect(LPRECT)
{
	force_log("OVERLAYDIRTYRECT\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::Blt(LPRECT rect, LPDIRECTDRAWSURFACE7 pSource, LPRECT rect_src, DWORD flags, LPDDBLTFX opts)
{
//	force_log_r("BLT-E : ", _flags, "\r\n");

	if (flags & DDBLT_COLORFILL)
	{
		if (rect)
		{
			RECT r = *rect;
			auto s = pHost->GetSurfaceData(mHandle);
			r.right = std::min<int>(r.right, s.x);
			r.bottom = std::min<int>(r.right, s.y);
			r.left = std::min<int>(r.left, s.x);
			r.top = std::min<int>(r.top, s.y);
			pHost->FillSurface(mHandle, opts->dwFillColor, &r);
		}
		else
			pHost->FillSurface(mHandle, opts->dwFillColor, nullptr);
		return DD_OK;
	}

	ESurface* pSrc = reinterpret_cast<ESurface*>(pSource);
	if (!pSrc)
		return DD_OK;

	return pSrc->BlitToESurface(this, rect, rect_src, flags, opts);
}

STDMETHODIMP ESurface::BltBatch(LPDDBLTBATCH, DWORD, DWORD)
{
	force_log("BLTBATCH\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::BltFast(DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 src, LPRECT rect_src, DWORD flags)
{
//	force_log_r("BLTFAST-E : ", _flags, "\r\n");

	ESurface* pSrc = reinterpret_cast<ESurface*>(src);
	if (!pSrc)
		return DD_OK;

	return pSrc->FastBlitToESurface(this, x, y, rect_src, flags);
}

STDMETHODIMP ESurface::DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE7)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK7)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::Flip(LPDIRECTDRAWSURFACE7, DWORD)
{
	force_log("FLIPPPPPPP\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetAttachedSurface(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 *)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetBltStatus(DWORD)
{
	force_log("GETBLIT\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetCaps(LPDDSCAPS2 pCaps)
{
	(*pCaps) = caps;
	return DD_OK;
}

STDMETHODIMP ESurface::GetClipper(LPDIRECTDRAWCLIPPER *)
{
	force_log("GETCLIPPER\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetColorKey(DWORD, LPDDCOLORKEY)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetDC(HDC *hdc)
{
	LOG("ESurface::GetDC\r\n");

	*hdc = ::GetDC(NULL);
	return DD_OK;
}

STDMETHODIMP ESurface::GetFlipStatus(DWORD)
{
	force_log("FLIPSTAT\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetOverlayPosition(LPLONG, LPLONG)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetPalette(LPDIRECTDRAWPALETTE *)
{
	force_log("GETPAL\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetPixelFormat(LPDDPIXELFORMAT pFormat)
{
	LOG("ESurface::getPixelFormat\r\n");

	(*pFormat) = DDPIXELFORMAT{};

	pFormat->dwSize = sizeof(DDPIXELFORMAT);

	pFormat->dwAlphaBitDepth = 0;
	pFormat->dwBBitMask = BLUE_MASK;
	pFormat->dwGBitMask = GREEN_MASK;
	pFormat->dwRBitMask = RED_MASK;
	pFormat->dwRGBBitCount = 16;

	pFormat->dwFlags = DDPF_RGB;

	return DD_OK;
}

STDMETHODIMP ESurface::GetSurfaceDesc(LPDDSURFACEDESC2 desc)
{
	LOG("ESurface::GetDesc\r\n");

	desc->dwSize = sizeof(DDSURFACEDESC2);
	desc->dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	auto size = pHost->GetSurfaceData(mHandle);
	desc->dwWidth = size.x;
	desc->dwHeight = size.y;
	desc->ddsCaps.dwCaps = 268485120;
	GetPixelFormat(&(desc->ddpfPixelFormat));
	return DD_OK;
}

STDMETHODIMP ESurface::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC2)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::IsLost()
{
	LOG("ESurface::IsLost\r\n");

	return DD_OK;
}

STDMETHODIMP ESurface::Lock(LPRECT rct, LPDDSURFACEDESC2 desc, DWORD flags, HANDLE h)
{
	LOG("ESurface::Lock\r\n");
	SurfaceMemory data;
	if (h)
	{
		auto s = pHost->GetSurfaceData(mHandle);
		RECT lr = *(reinterpret_cast<RECT*>(h));
		lr.right += lr.left;
		lr.bottom += lr.top;

		if ((lr.right > s.x) || (lr.bottom > s.y))
		{
		//	OutputDebugString(L"fffuu");
			lr.right = std::min<int>(lr.right, s.x);
			lr.bottom = std::min<int>(lr.bottom, s.y);
			lr.left = std::min<int>(lr.left, s.x);
			lr.top = std::min<int>(lr.top, s.y);
		}
		
		data = pHost->LockSurface(mHandle, &lr);
	}
	else
		data = pHost->LockSurface(mHandle);

	if (!data.pMem)
		return DDERR_INVALIDPARAMS;

	GetSurfaceDesc(desc);
	desc->dwFlags |= DDSD_PITCH;
	desc->lpSurface = data.pMem;
	desc->lPitch = data.pitch;

#ifdef _DEBUG
	std::wostringstream oss;
	oss << L"Lock ";
	oss << std::hex << (int)(data.pMem);
	oss << L'\n';

	OutputDebugString(oss.str().c_str());
#endif


	LOG("ESurface::Lock() - ");
	LOG(std::ios_base::hex);
	LOG(((int)(pHost->GetSurfaceMemory(mHandle).pMem)));
	LOG("\r\n");

	return DD_OK;
}

STDMETHODIMP ESurface::ReleaseDC(HDC hdc)
{
	LOG("ESurface::ReleaseDC\r\n");

	::ReleaseDC(NULL, hdc);
	return DD_OK;
}

STDMETHODIMP ESurface::Restore()
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetClipper(LPDIRECTDRAWCLIPPER)
{
	force_log("SETCLIP\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetColorKey(DWORD flags, LPDDCOLORKEY key)
{
	LOG("ESurface::SetColorKey\r\n");
	// flags are ignored for now;
	// default to DDCKEY_SRCBLT
	key_low = key->dwColorSpaceLowValue;
	key_high = key->dwColorSpaceHighValue;
	color_keyed = true;
	return DD_OK;
}

STDMETHODIMP ESurface::SetOverlayPosition(LONG, LONG)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetPalette(LPDIRECTDRAWPALETTE)
{
	force_log("SETPAL\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::Unlock(LPRECT rct)
{

	LOG("ESurface::Unlock() - ");
	LOG(std::ios_base::hex);
	LOG((int)((pHost->GetSurfaceMemory(mHandle).pMem)));
	LOG("\r\n");
/*	auto m = pHost->GetSurfaceMemory(mHandle);
	WORD* pmem = reinterpret_cast<WORD*>(m.pMem);
	WORD* lmem = pmem;
	for (auto p : b)
	{
		if (p.x == 0)
		{
			lmem = pmem;
			pmem += m.pitch / sizeof(WORD);
		}
		WORD w = *lmem++;
		p.value.r = ((w & RED_MASK) >> RED_SHIFT) << (8 - RED_WIDTH);
		p.value.g = ((w & GREEN_MASK) >> GREEN_SHIFT) << (8 - GREEN_WIDTH);
		p.value.b = ((w & BLUE_MASK) >> BLUE_SHIFT) << (8 - BLUE_WIDTH);
	}

	std::string fn = "E:\\frame";
	fn = fn + std::to_string(frame++) + ".bmp";
	b.Save(fn);*/
    pHost->UnlockSurface(mHandle);
	return DD_OK;
}

STDMETHODIMP ESurface::UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::UpdateOverlayDisplay(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE7)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetDDInterface(LPVOID *)
{
	force_log("DDI\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::PageLock(DWORD)
{
	force_log("PLOCK\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::PageUnlock(DWORD)
{
	force_log("PFREE\r\n");

	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetSurfaceDesc(LPDDSURFACEDESC2, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetPrivateData(REFGUID, LPVOID, DWORD, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetPrivateData(REFGUID, LPVOID, LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::FreePrivateData(REFGUID)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetUniquenessValue(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::ChangeUniquenessValue()
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetPriority(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetPriority(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::SetLOD(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP ESurface::GetLOD(LPDWORD)
{
	return E_NOTIMPL;
}

HRESULT ESurface::BlitToCPUSurface(ECPUSurface * pSurf, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx)
{
	return E_NOTIMPL;
}

HRESULT ESurface::FastBlitToCPUSurface(ECPUSurface * pSurf, DWORD x, DWORD y, LPRECT pSrcRect, DWORD flags)
{
	return E_NOTIMPL;
}

HRESULT ESurface::BlitToESurface(ESurface * pSurf, LPRECT rect, LPRECT rect_src, DWORD flags, LPDDBLTFX fx)
{
	Handle sHandle = pSurf->mHandle;

	RECT r{};

	auto ds = pHost->GetSurfaceData(mHandle);
	auto ss = pHost->GetSurfaceData(sHandle);

	if (rect_src)
		r = *rect_src;
	else
		r = { 0,0,ss.x,ss.y };

	int x = 0;
	int y = 0;

	if (rect)
	{
		x = rect->left;
		y = rect->top;
	}

	if ((r.right == ds.x) && (r.bottom == ds.y) && (x == 0) && (y == 0) && (r.left == 0) && (r.right == 0))
		pHost->CopySurface(pSurf->mHandle, mHandle);
	else
		pHost->CopySubSurface(pSurf->mHandle, mHandle, x, y, r);

	return DD_OK;
}

HRESULT ESurface::FastBlitToESurface(ESurface * pSurf, DWORD x, DWORD y, LPRECT rect_src, DWORD flags)
{
	Handle sHandle = pSurf->mHandle;

	auto ds = pHost->GetSurfaceData(sHandle);
	auto ss = pHost->GetSurfaceData(mHandle);

	RECT r{};

	if (rect_src)
		r = *rect_src;
	else
		r = { 0,0,ss.x,ss.y };

	if ((r.right == ds.x) && (r.bottom == ds.y) && (x == 0) && (y == 0) && (r.left == 0) && (r.right == 0))
		pHost->CopySurface(pSurf->mHandle, mHandle);
	else
		pHost->CopySubSurface(pSurf->mHandle, mHandle, x, y, r);

	return DD_OK;
}

HRESULT ESurface::UpdateSubsurface(LPRECT pDstRect, LPVOID memory, DWORD pitch, DWORD flags, LPDDBLTFX fx)
{
	pHost->UpdateSubsurface(mHandle, pDstRect, memory, pitch);
	return DD_OK;
}

void ESurface::Snapshot(std::string fname)
{
}
