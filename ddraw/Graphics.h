#pragma once

#include "d3dhost.h"
#include "unordered_vector.hpp"

const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

typedef int Handle;

#define S_PRIMARY 1
#define S_SYSMEM 16

struct SurfaceMemory
{
	void* pMem;
	int   pitch;
};

struct Size2D
{
	int x;
	int y;
};

class Graphics
{
	unordered_vector<Texture> vSurfaces;
public:
	Graphics();
	~Graphics();

	bool Initialize(HWND hWnd, int, int);
	void Shutdown();
	bool Frame();

	void SetDisplayMode(DWORD x_res, DWORD y_res, DWORD bit_depth);
	Handle CreateSurface(int xs, int ys, int flags = 0);
	SurfaceMemory LockSurface(Handle h, RECT* pR = nullptr);
	void          UnlockSurface(Handle h);
	Size2D GetSurfaceData(Handle h);
	SurfaceMemory GetSurfaceMemory(Handle h);

	void DestroySurface(Handle h);

	void CopySurface(Handle dst, Handle src);
	void CopySubSurface(Handle dst, Handle src, int x, int y, RECT src_rect);
	
	void FillSurface(Handle h, DWORD color, RECT* pRect = nullptr);
	void UpdateSubsurface(Handle h, LPRECT pDstRect, LPVOID memory, DWORD pitch);

	int VideoMemory();

	void UpdateFrame(Handle hPrimary);

private:
	bool Render();
	bool InitializeWindow(int&, int&);
	void ShutdownWindow();

private:
	HWND hWindow;
	D3DHost mDHost;
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;

	int xr, yr;
	int bit_depth;
	int ixr, iyr;
	TextureFormat internal_format;
};

