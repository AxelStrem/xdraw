#include "stdafx.h"
#include "FBSurface.h"


FBSurface::FBSurface(IDirectDrawSurface7* pFallBack, int h)
	: pFB(pFallBack), handle(h)
{
}


FBSurface::~FBSurface()
{
}

STDMETHODIMP FBSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE7)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::AddOverlayDirtyRect(LPRECT)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::Blt(LPRECT pR, LPDIRECTDRAWSURFACE7 pS, LPRECT pSR, DWORD f, LPDDBLTFX fx)
{
	if (pS)
	{
		FBSurface* pTS = (FBSurface*)(pS);
		return pFB->Blt(pR, pTS->pFB, pSR, f, fx);
	}
	return DD_OK;
}

STDMETHODIMP FBSurface::BltBatch(LPDDBLTBATCH, DWORD, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::BltFast(DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pS, LPRECT pSR, DWORD f)
{
	FBSurface* pTS = (FBSurface*)(pS);
	return pFB->BltFast(x, y, pTS->pFB, pSR, f);
}

STDMETHODIMP FBSurface::DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE7)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK7)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::Flip(LPDIRECTDRAWSURFACE7, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetAttachedSurface(LPDDSCAPS2, LPDIRECTDRAWSURFACE7 *)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetBltStatus(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetCaps(LPDDSCAPS2 caps)
{
	return pFB->GetCaps(caps);
}

STDMETHODIMP FBSurface::GetClipper(LPDIRECTDRAWCLIPPER *)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetColorKey(DWORD, LPDDCOLORKEY)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetDC(HDC *)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetFlipStatus(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetOverlayPosition(LPLONG, LPLONG)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetPalette(LPDIRECTDRAWPALETTE *)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetPixelFormat(LPDDPIXELFORMAT pf)
{
	HRESULT hr = pFB->GetPixelFormat(pf);
	return hr;
}

STDMETHODIMP FBSurface::GetSurfaceDesc(LPDDSURFACEDESC2 pf)
{
	HRESULT hr = pFB->GetSurfaceDesc(pf);
	return hr;
}

STDMETHODIMP FBSurface::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC2)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::IsLost()
{
	return pFB->IsLost();
}

STDMETHODIMP FBSurface::Lock(LPRECT r, LPDDSURFACEDESC2 d, DWORD f, HANDLE h)
{
	HRESULT hr = pFB->Lock(r, d, f, h);
	return hr;
}

STDMETHODIMP FBSurface::ReleaseDC(HDC)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::Restore()
{
	return pFB->Restore();
}

STDMETHODIMP FBSurface::SetClipper(LPDIRECTDRAWCLIPPER)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetColorKey(DWORD f, LPDDCOLORKEY x)
{
	return pFB->SetColorKey(f, x);
}

STDMETHODIMP FBSurface::SetOverlayPosition(LONG, LONG)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetPalette(LPDIRECTDRAWPALETTE)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::Unlock(LPRECT r)
{
	return pFB->Unlock(r);
}

STDMETHODIMP FBSurface::UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::UpdateOverlayDisplay(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE7)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetDDInterface(LPVOID *)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::PageLock(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::PageUnlock(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetSurfaceDesc(LPDDSURFACEDESC2, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetPrivateData(REFGUID, LPVOID, DWORD, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetPrivateData(REFGUID, LPVOID, LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::FreePrivateData(REFGUID)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetUniquenessValue(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::ChangeUniquenessValue()
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetPriority(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetPriority(LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::SetLOD(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP FBSurface::GetLOD(LPDWORD)
{
	return E_NOTIMPL;
}
