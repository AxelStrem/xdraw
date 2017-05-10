#include "stdafx.h"
#include "EClipper.h"


EClipper::EClipper()
{
}


EClipper::~EClipper()
{
}

STDMETHODIMP EClipper::GetClipList(LPRECT, LPRGNDATA, LPDWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EClipper::GetHWnd(HWND *)
{
	return E_NOTIMPL;
}

STDMETHODIMP EClipper::Initialize(LPDIRECTDRAW, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EClipper::IsClipListChanged(BOOL *)
{
	return E_NOTIMPL;
}

STDMETHODIMP EClipper::SetClipList(LPRGNDATA, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP EClipper::SetHWnd(DWORD, HWND)
{
	return E_NOTIMPL;
}
