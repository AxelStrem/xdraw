#pragma once
#include <ddraw.h>
#include "COMRefCounter.hpp"
#include "EDirectDraw.h"

class EGammaControl : public COMRefCounter<IDirectDrawGammaControl>
{
	EDirectDraw* pDevice;
public:
	EGammaControl(EDirectDraw* pD);
	~EGammaControl();

	STDMETHOD(GetGammaRamp)( DWORD, LPDDGAMMARAMP) ;
	STDMETHOD(SetGammaRamp)( DWORD, LPDDGAMMARAMP) ;
};

