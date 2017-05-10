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
	Update();
	return ESurface::Lock(lRect, pDesc, flags, h);
}

STDMETHODIMP EPrimarySurface::Unlock(LPRECT lRect)
{
	HRESULT hr = ESurface::Unlock(lRect);
	Update();
	return hr;
}

STDMETHODIMP EPrimarySurface::Blt(LPRECT dst_rect, LPDIRECTDRAWSURFACE7 pSrc, LPRECT src_rect, DWORD flags, LPDDBLTFX fx)
{
	ESurface* ps = reinterpret_cast<ESurface*>(pSrc);
	Update();
	HRESULT hr = ESurface::Blt(dst_rect, pSrc, src_rect, flags, fx);
	return hr;
}

STDMETHODIMP EPrimarySurface::BltBatch(LPDDBLTBATCH lBatch, DWORD flags, DWORD u)
{
	Update();
	HRESULT hr = ESurface::BltBatch(lBatch, flags, u);
	return hr;
}

STDMETHODIMP EPrimarySurface::BltFast(DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pSrc, LPRECT src_rect, DWORD flags)
{

	ESurface* ps = reinterpret_cast<ESurface*>(pSrc);
	Update();
	HRESULT hr = ESurface::BltFast(x, y, pSrc, src_rect, flags);
	return hr;
}

void EPrimarySurface::Update()
{
	pHost->UpdateFrame(mHandle);
}
