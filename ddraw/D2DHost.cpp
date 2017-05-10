#include "stdafx.h"
#include "D2DHost.h"



HRESULT D2DHost::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

	return hr;
}

HRESULT D2DHost::CreateDeviceResources(HWND hw, int xs, int ys)
{
	HRESULT hr;
	
	xres = xs;
	yres = ys;
	
	D2D1_SIZE_U size = D2D1::SizeU(xres, yres);

	hWindow = hw;

	// Create a Direct2D render target.
	hr = m_pDirect2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWindow, size),
		&m_pRenderTarget
	);

	return hr;
}

bool D2DHost::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources(hwnd, screenWidth, screenHeight);
	return false;
}

D2DHost::D2DHost()
{
	
}


D2DHost::~D2DHost()
{
}
