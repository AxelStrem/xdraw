#include "stdafx.h"
#include "EPrimarySurface.h"


EPrimarySurface::EPrimarySurface(Graphics* host, Handle h, int flags) : ESurface(host, h, flags)
{
}


EPrimarySurface::~EPrimarySurface()
{
}

STDMETHODIMP EPrimarySurface::Lock(LPRECT lRect, LPDDSURFACEDESC2 pDesc, DWORD flags, HANDLE h)
{
	pHost->LockDisplay();
	Update();
	return ESurface::Lock(lRect, pDesc, flags, h);
}

STDMETHODIMP EPrimarySurface::Unlock(LPRECT lRect)
{
	HRESULT hr = ESurface::Unlock(lRect);
	Update();
	pHost->UnlockDisplay();
	return hr;
}

STDMETHODIMP EPrimarySurface::Blt(LPRECT dst_rect, LPDIRECTDRAWSURFACE7 pSrc, LPRECT src_rect, DWORD flags, LPDDBLTFX fx)
{
	pHost->LockDisplay();
	ESurface* ps = reinterpret_cast<ESurface*>(pSrc);
	Update();
	HRESULT hr = ESurface::Blt(dst_rect, pSrc, src_rect, flags, fx);
	pHost->UnlockDisplay();
	return hr;
}

STDMETHODIMP EPrimarySurface::BltBatch(LPDDBLTBATCH lBatch, DWORD flags, DWORD u)
{
	pHost->LockDisplay();
	Update();
	HRESULT hr = ESurface::BltBatch(lBatch, flags, u);
	pHost->UnlockDisplay();
	return hr;
}

STDMETHODIMP EPrimarySurface::BltFast(DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pSrc, LPRECT src_rect, DWORD flags)
{
	pHost->LockDisplay();
	ESurface* ps = reinterpret_cast<ESurface*>(pSrc);
	Update();
	HRESULT hr = ESurface::BltFast(x, y, pSrc, src_rect, flags);
	pHost->UnlockDisplay();
	return hr;
}

STDMETHODIMP EPrimarySurface::GetDC(HDC * hdc)
{
	pHost->LockDisplay();
	ForceUpdate();
	HRESULT hr = ESurface::GetDC(hdc);
	pHost->UnlockDisplay();
	return hr;
}

void EPrimarySurface::Update()
{
	pHost->UpdateFrame(mHandle);
}

void EPrimarySurface::ForceUpdate()
{
	pHost->ForceUpdateFrame(mHandle);
}
