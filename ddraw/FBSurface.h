#pragma once
#include <ddraw.h>
#include "COMRefCounter.hpp"
#include "ddraw_iid_list.h"

class FBSurface : public COMRefCounter<IDirectDrawSurface7,
	IDirectDrawSurface4,
	IDirectDrawSurface3,
	IDirectDrawSurface2,
	IDirectDrawSurface>
{
	IDirectDrawSurface7* pFB;
public:
	bool bPrimary = false;
	int handle;

	FBSurface(IDirectDrawSurface7* pFallBack, int h);
	~FBSurface();

	STDMETHOD(AddAttachedSurface)( LPDIRECTDRAWSURFACE7) ;
	STDMETHOD(AddOverlayDirtyRect)( LPRECT) ;
	STDMETHOD(Blt)( LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX) ;
	STDMETHOD(BltBatch)( LPDDBLTBATCH, DWORD, DWORD) ;
	STDMETHOD(BltFast)( DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD) ;
	STDMETHOD(DeleteAttachedSurface)( DWORD, LPDIRECTDRAWSURFACE7) ;
	STDMETHOD(EnumAttachedSurfaces)( LPVOID, LPDDENUMSURFACESCALLBACK7) ;
	STDMETHOD(EnumOverlayZOrders)( DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7) ;
	STDMETHOD(Flip)( LPDIRECTDRAWSURFACE7, DWORD) ;
	STDMETHOD(GetAttachedSurface)( LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *) ;
	STDMETHOD(GetBltStatus)( DWORD) ;
	STDMETHOD(GetCaps)( LPDDSCAPS2) ;
	STDMETHOD(GetClipper)( LPDIRECTDRAWCLIPPER FAR*) ;
	STDMETHOD(GetColorKey)( DWORD, LPDDCOLORKEY) ;
	STDMETHOD(GetDC)( HDC FAR *) ;
	STDMETHOD(GetFlipStatus)( DWORD) ;
	STDMETHOD(GetOverlayPosition)( LPLONG, LPLONG) ;
	STDMETHOD(GetPalette)( LPDIRECTDRAWPALETTE FAR*) ;
	STDMETHOD(GetPixelFormat)( LPDDPIXELFORMAT) ;
	STDMETHOD(GetSurfaceDesc)( LPDDSURFACEDESC2) ;
	STDMETHOD(Initialize)( LPDIRECTDRAW, LPDDSURFACEDESC2) ;
	STDMETHOD(IsLost)() ;
	STDMETHOD(Lock)( LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE) ;
	STDMETHOD(ReleaseDC)( HDC) ;
	STDMETHOD(Restore)() ;
	STDMETHOD(SetClipper)( LPDIRECTDRAWCLIPPER) ;
	STDMETHOD(SetColorKey)( DWORD, LPDDCOLORKEY) ;
	STDMETHOD(SetOverlayPosition)( LONG, LONG) ;
	STDMETHOD(SetPalette)( LPDIRECTDRAWPALETTE) ;
	STDMETHOD(Unlock)( LPRECT) ;
	STDMETHOD(UpdateOverlay)( LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX) ;
	STDMETHOD(UpdateOverlayDisplay)( DWORD) ;
	STDMETHOD(UpdateOverlayZOrder)( DWORD, LPDIRECTDRAWSURFACE7) ;
	/*** Added in the v2 interface ***/
	STDMETHOD(GetDDInterface)( LPVOID FAR *) ;
	STDMETHOD(PageLock)( DWORD) ;
	STDMETHOD(PageUnlock)( DWORD) ;
	/*** Added in the v3 interface ***/
	STDMETHOD(SetSurfaceDesc)( LPDDSURFACEDESC2, DWORD) ;
	/*** Added in the v4 interface ***/
	STDMETHOD(SetPrivateData)( REFGUID, LPVOID, DWORD, DWORD) ;
	STDMETHOD(GetPrivateData)( REFGUID, LPVOID, LPDWORD) ;
	STDMETHOD(FreePrivateData)( REFGUID) ;
	STDMETHOD(GetUniquenessValue)( LPDWORD) ;
	STDMETHOD(ChangeUniquenessValue)() ;
	/*** Moved Texture7 methods here ***/
	STDMETHOD(SetPriority)( DWORD) ;
	STDMETHOD(GetPriority)( LPDWORD) ;
	STDMETHOD(SetLOD)( DWORD) ;
	STDMETHOD(GetLOD)( LPDWORD) ;
};

