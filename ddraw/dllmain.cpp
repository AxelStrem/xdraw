// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ddraw_interface.h"

#include "EDirectDraw.h"
#include "Recorder.h"

#pragma comment(lib,"ddraw.lib")

#define MAX_STR 1024

#define PROLOGUE 
#define EPILOGUE(x) return x

std::string   global_out_path;



std::thread   global_recorder_thread;

void InitRecFile()
{
	std::ifstream f;
	std::string fname;
	int fnum = 0;
	do
	{
		fname = global_out_path + "rec" + std::to_string(fnum++) + ".arc";
		f.open(fname.c_str(), std::ios_base::binary);
		if (!f)
			break;
		f.close();
		f.clear();
	} while (true);
	f.close();
	f.clear();

	global_out_file.open(fname.c_str(), std::ios_base::binary);
	fname.back() = 's';
	global_out_file_stream.open(fname.c_str(), std::ios_base::binary);
    global_is_recording = false;// true;
}

void LoadConfig(void)
{
	std::ifstream cfg("xdraw.cfg");
	while (cfg)
	{
		char tmp[MAX_STR];
		cfg.getline(tmp, MAX_STR);
		std::string s(tmp);
		std::istringstream iss(s);
		std::string cmd;
		iss >> cmd;

		if (cmd.find("rec", 0) != std::string::npos)
		{
			iss >> global_out_path;
			InitRecFile();
		}
	};
}

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
			LoadConfig();
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

		

		*lplpDD = new EDirectDraw(1920*2, 1080*2);

		HRESULT hResult = E_NOTIMPL;

		EPILOGUE(hResult);
	}


	__declspec(dllexport) HRESULT __stdcall XDrawCreateEx_(GUID* lpGUID, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
	{
#pragma comment(linker, "/EXPORT:XDrawCreateEx=_XDrawCreateEx_@16")
		
		IDirectDraw7* pFB;
		
		DirectDrawCreateEx(lpGUID, (void**)&pFB, iid, pUnkOuter);

		*lplpDD = new EDirectDraw(1920*2, 1080*2, pFB);
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