#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")

#include <d2d1.h>
#include <d2d1helper.h>

#include "shared_com.hpp"

class D2DHost
{
	shared_com<ID2D1Factory> m_pDirect2dFactory;
	shared_com<ID2D1HwndRenderTarget> m_pRenderTarget;
	shared_com<ID2D1SolidColorBrush> m_pLightSlateGrayBrush;
	shared_com<ID2D1SolidColorBrush> m_pCornflowerBlueBrush;

	HWND hWindow;

	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources(HWND hWindow, int xs, int ys);

	int xres;
	int yres;

public:
	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen);
	void Shutdown() {}

	D2DHost();
	~D2DHost();
};

