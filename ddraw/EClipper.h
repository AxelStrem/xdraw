#pragma once

#include <ddraw.h>
#include "ESurface.h"

class EClipper : public COMRefCounter<IDirectDrawClipper>
{
public:
	EClipper();
	~EClipper();

	/*** IDirectDrawClipper methods ***/
	STDMETHOD(GetClipList)( LPRECT, LPRGNDATA, LPDWORD) ;
	STDMETHOD(GetHWnd)( HWND FAR *) ;
	STDMETHOD(Initialize)( LPDIRECTDRAW, DWORD) ;
	STDMETHOD(IsClipListChanged)( BOOL FAR *) ;
	STDMETHOD(SetClipList)( LPRGNDATA, DWORD) ;
	STDMETHOD(SetHWnd)( DWORD, HWND) ;
};

