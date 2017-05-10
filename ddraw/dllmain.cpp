// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ddraw_interface.h"

#include "EDirectDraw.h"

#pragma comment(lib,"ddraw.lib")

#define PROLOGUE 
#define EPILOGUE(x) return x

extern "C" {

	__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule,
		DWORD  ul_reason_for_call,
		LPVOID lpReserved
	)
	{
#pragma comment(linker, "/EXPORT:DllMain=_DllMain@12")

		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			return TRUE;
		case DLL_THREAD_ATTACH:
			return TRUE;
		case DLL_THREAD_DETACH:
			return TRUE;
		case DLL_PROCESS_DETACH:
			return TRUE;
		}
		return TRUE;
	}

	__declspec(dllexport) HRESULT __stdcall XDrawCreate_(GUID* lpGUID, LPDIRECTDRAW7* lplpDD, IUnknown* pUnkOuter)
	{
#pragma comment(linker, "/EXPORT:XDrawCreate=_XDrawCreate_@12")

		PROLOGUE;

		

		*lplpDD = new EDirectDraw(1920, 1080);

		HRESULT hResult = E_NOTIMPL;

		EPILOGUE(hResult);
	}


	__declspec(dllexport) HRESULT __stdcall XDrawCreateEx_(GUID* lpGUID, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
	{
#pragma comment(linker, "/EXPORT:XDrawCreateEx=_XDrawCreateEx_@16")
		
		IDirectDraw7* pFB;
		
		DirectDrawCreateEx(lpGUID, (void**)&pFB, iid, pUnkOuter);

		*lplpDD = new EDirectDraw(1920, 1080, pFB);
		return S_OK;
	}

	__declspec(dllexport) HRESULT  __stdcall XDrawCreateClipper_(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter)
	{
#pragma comment(linker, "/EXPORT:XDrawCreateClipper=_XDrawCreateClipper_@12")

		PROLOGUE;
		HRESULT hResult = E_NOTIMPL;

		EPILOGUE(hResult);
	}

	__declspec(dllexport) HRESULT  __stdcall XDrawLockRect_(LPRECT rect)
	{
#pragma comment(linker, "/EXPORT:XDrawLockRect=_XDrawLockRect_@4")

		PROLOGUE;
		HRESULT hResult = E_NOTIMPL;

		EPILOGUE(hResult);
	}

}