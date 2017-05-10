#include "stdafx.h"
#include "EGammaControl.h"


EGammaControl::EGammaControl(EDirectDraw* pD) : pDevice(pD)
{
}


EGammaControl::~EGammaControl()
{
}

STDMETHODIMP EGammaControl::GetGammaRamp(DWORD, LPDDGAMMARAMP)
{
	return E_NOTIMPL;
}

STDMETHODIMP EGammaControl::SetGammaRamp(DWORD, LPDDGAMMARAMP)
{
	return E_NOTIMPL;
}
