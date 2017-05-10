#include "stdafx.h"
#include "Graphics.h"


Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}

bool Graphics::Initialize(HWND hWnd, int screenWidth, int screenHeight)
{
	bool result;

	ixr = screenWidth;
	iyr = screenHeight;

	internal_format = TF_16BIT;
	bit_depth = 16;

	if (!hWnd)
		InitializeWindow(screenWidth, screenHeight);
	else
		hWindow = hWnd;

	result = mDHost.Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hWindow, FULL_SCREEN);
	
	if (!result)
	{
		MessageBox(hWindow, L"Could not initialize DirectX", L"Error", MB_OK);
		return false;
	}

	return true;
}
void Graphics::Shutdown()
{
	mDHost.Shutdown();
	ShutdownWindow();
}

bool Graphics::Frame()
{
	return Render();
}

void Graphics::SetDisplayMode(DWORD x_res, DWORD y_res, DWORD bd)
{
	if (bd != bit_depth)
	{

	}
	if ((x_res != xr) || (y_res != yr))
	{
		mDHost.SetResolution(x_res, y_res);
		xr = x_res;
		yr = y_res;
	}

	bit_depth = bd;
}

Handle Graphics::CreateSurface(int xs, int ys, int flags)
{
	if (xs == 0)
		xs = ixr;
	if (ys == 0)
		ys = iyr;
	force_log(UpdateTimer(), ": Create texture: ", xs, 'x', ys);
	Handle h = vSurfaces.insert(mDHost.CreateTexture(xs, ys, internal_format, flags | F_STAGING | F_CPU_WRITE | F_CPU_READ));
	force_log(" -> ", h, " --- ", UpdateTimer(), "\r\n");
	//if (flags&S_PRIMARY)
	//	vSurfaces[h].bForceGPU = true;
	vSurfaces[h].handle = h;
	return h;
}

SurfaceMemory Graphics::LockSurface(Handle h, RECT* pR)
{
	if (mDHost.MapTexture(vSurfaces[h], pR))
		return SurfaceMemory{ vSurfaces[h].GetAddress(), vSurfaces[h].GetPitch() };
	else
		return SurfaceMemory{ nullptr, 0 };
}

void Graphics::UnlockSurface(Handle h)
{
	mDHost.Unmap(vSurfaces[h]);
}

Size2D Graphics::GetSurfaceData(Handle h)
{
	if (h < 0)
		return Size2D{ 0, 0 };
	return Size2D{ vSurfaces[h].GetXSize(), vSurfaces[h].GetYSize() };
}

SurfaceMemory Graphics::GetSurfaceMemory(Handle h)
{
	return SurfaceMemory{ vSurfaces[h].GetAddress(), vSurfaces[h].GetPitch() };
}

void Graphics::DestroySurface(Handle h)
{
	vSurfaces.erase(h);
}

void Graphics::CopySurface(Handle dst, Handle src)
{
	force_log(UpdateTimer(),": COPY ",dst," ",src,"\r\n");
	mDHost.CopyTexture(vSurfaces[dst], vSurfaces[src]);
}

void Graphics::CopySubSurface(Handle dst, Handle src, int x, int y, RECT src_rect)
{
	mDHost.CopySubTexture(vSurfaces[dst], vSurfaces[src], x, y, src_rect);
}

void Graphics::FillSurface(Handle h, DWORD color, RECT * pRect)
{
	if(vSurfaces.has(h))
		mDHost.FillTexture(vSurfaces[h], color, pRect);
}

void Graphics::UpdateSubsurface(Handle h, LPRECT pDstRect, LPVOID memory, DWORD pitch)
{
	mDHost.UpdateSubtexture(vSurfaces[h], pDstRect, memory, pitch);
}

int Graphics::VideoMemory()
{
	return 1073741824*2;
}

void Graphics::UpdateFrame(Handle hPrimary)
{
	mDHost.Frame(vSurfaces[hPrimary]);
}

bool Graphics::Render()
{
	// Clear the buffers to begin the scene.
	//mDHost.BeginScene(0.5f, 0.5f, 0.5f, 1.0f);


	// Present the rendered scene to the screen.
	//mDHost.EndScene();

	return true;
}

bool Graphics::InitializeWindow(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"DDrawWrapper";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	hWindow = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(hWindow, SW_SHOW);
	SetForegroundWindow(hWindow);
	//SetFocus(m_hwnd);

	// Hide the mouse cursor.
	//ShowCursor(false);

	return true;
}

void Graphics::ShutdownWindow()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(hWindow);

	UnregisterClass(m_applicationName, m_hinstance);

	return;
}