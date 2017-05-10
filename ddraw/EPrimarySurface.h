#pragma once
#include "ESurface.h"



class EPrimarySurface :
	public ESurface
{
	
public:
	EPrimarySurface(Graphics * host, Handle h, int flags);
	~EPrimarySurface();

	STDMETHOD(Lock)(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	STDMETHOD(Unlock)(LPRECT);

	STDMETHOD(Blt)(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);

	void Update();
};

