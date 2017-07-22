#pragma once
#include "ESurface.h"

#include <vector>

class ECPUSurface :
	public ESurface
{
	std::vector<WORD> data;
	int xs;
	int ys;
	std::vector<RECT> lock_rects;
public:
	ECPUSurface(int x_size, int y_size, Graphics* host, Handle handle_, int flags);
	~ECPUSurface();

	STDMETHOD(Lock)(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	STDMETHOD(Unlock)(LPRECT);

	STDMETHOD(Blt)(LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);

	virtual HRESULT BlitToCPUSurface(ECPUSurface* pSurf, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx);
	virtual HRESULT FastBlitToCPUSurface(ECPUSurface* pSurf, DWORD x, DWORD y, LPRECT pSrcRect, DWORD flags);

	virtual HRESULT BlitToESurface(ESurface* pSurf, LPRECT pDstRect, LPRECT pSrcRect, DWORD flags, LPDDBLTFX fx);
	virtual HRESULT FastBlitToESurface(ESurface* pSurf, DWORD x, DWORD y, LPRECT pSrcRect, DWORD flags);

	void FillSurface(WORD color, LPRECT pRect);
	virtual void Snapshot(std::string fname);

	int GetWidth() { return xs; }
	int GetHeight() { return ys; }

	void SaveBMP(std::string fname);
};

