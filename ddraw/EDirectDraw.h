//
//  EDirectDraw class
//  Emulates main DirectDraw class, inherits IDirectDraw interface
//

#pragma once
#include <ddraw.h>
#include "Graphics.h"
#include "COMRefCounter.hpp"
#include "ddraw_iid_list.h"

#define BLUE_MASK  0b0000000000011111
#define GREEN_MASK 0b0000011111100000
#define RED_MASK   0b1111100000000000

#define BLUE_SHIFT  0
#define GREEN_SHIFT 5
#define RED_SHIFT   11

#define BLUE_WIDTH  5
#define GREEN_WIDTH 6
#define RED_WIDTH   5

//DEFINE_GUID(IID_EDirectDraw, 0x6C14DB80, 0xF133, 0xDEAD, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x88, 0xE5, 0x60);

class EDirectDraw : public COMRefCounter<IDirectDraw7,
                                         IDirectDraw4,
                                         IDirectDraw2,
                                         IDirectDraw>
{
	Graphics mGraphics;
	int xres;
	int yres;
	volatile ULONG m_cRef;

	IDirectDraw7* pFB;
	int chandle = 0;

public:
	EDirectDraw(int xres, int yres, IDirectDraw7* pF = nullptr);
	~EDirectDraw();

	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)() ;
	STDMETHOD(CreateClipper)( DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR *) ;
	STDMETHOD(CreatePalette)( DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR *) ;
	STDMETHOD(CreateSurface)(  LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *) ;
	STDMETHOD(DuplicateSurface)( LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *) ;
	STDMETHOD(EnumDisplayModes)( DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2) ;
	STDMETHOD(EnumSurfaces)( DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7) ;
	STDMETHOD(FlipToGDISurface)() ;
	STDMETHOD(GetCaps)( LPDDCAPS, LPDDCAPS) ;
	STDMETHOD(GetDisplayMode)( LPDDSURFACEDESC2) ;
	STDMETHOD(GetFourCCCodes)(  LPDWORD, LPDWORD) ;
	STDMETHOD(GetGDISurface)( LPDIRECTDRAWSURFACE7 FAR *) ;
	STDMETHOD(GetMonitorFrequency)( LPDWORD) ;
	STDMETHOD(GetScanLine)( LPDWORD) ;
	STDMETHOD(GetVerticalBlankStatus)( LPBOOL) ;
	STDMETHOD(Initialize)( GUID FAR *) ;
	STDMETHOD(RestoreDisplayMode)() ;
	STDMETHOD(SetCooperativeLevel)( HWND, DWORD) ;
	STDMETHOD(SetDisplayMode)( DWORD, DWORD, DWORD, DWORD, DWORD) ;
	STDMETHOD(WaitForVerticalBlank)( DWORD, HANDLE) ;
	/*** Added in the v2 interface ***/
	STDMETHOD(GetAvailableVidMem)( LPDDSCAPS2, LPDWORD, LPDWORD) ;
	/*** Added in the V4 Interface ***/
	STDMETHOD(GetSurfaceFromDC) ( HDC, LPDIRECTDRAWSURFACE7 *) ;
	STDMETHOD(RestoreAllSurfaces)() ;
	STDMETHOD(TestCooperativeLevel)() ;
	STDMETHOD(GetDeviceIdentifier)( LPDDDEVICEIDENTIFIER2, DWORD) ;
	STDMETHOD(StartModeTest)( LPSIZE, DWORD, DWORD) ;
	STDMETHOD(EvaluateMode)( DWORD, DWORD *) ;

	void FillCaps(LPDDCAPS caps);
	IDirectDrawSurface7* CreateSurface(int x_size, int y_size, int flags = 0);
	IDirectDrawSurface7* CreatePrimarySurface(int x_size, int y_size, int flags = 0);
	IDirectDrawSurface7* CreateCPUSurface(int x_size, int y_size, int flags = 0);

};

