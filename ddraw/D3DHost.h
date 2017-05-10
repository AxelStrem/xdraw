#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3d11_1.h>

#include "shared_com.hpp"
#include "RectMerger.h"

#define F_GDI           0x00000001
#define F_STAGING       0x00000002
#define F_CPU_WRITE     0x00000004
#define F_CPU_READ      0x00000008

enum TextureFormat
{
	TF_32BIT, TF_16BIT
};

struct Invalidator
{
	RM::RectMerger zone;
	bool           total;

	void InvalidateAll(int sx, int sy)
	{
		total = true;
		zone.Clear();
		zone.AddRect(RM::Rect(0, 0, sx, sy));
	}

	void InvalidateRect(int left, int top, int right, int bottom)
	{
		zone.AddRect(RM::Rect(RM::Point{ left,top }, RM::Point{ right, bottom }));
	}

	void InvalidateRect(RM::Rect r)
	{
		zone.AddRect(r);
	}

	void Validate()
	{
		total = false;
		zone.Clear();
	}

	bool IsValid()
	{
		return ((!total) && (zone.IsEmpty()));
	}

	bool IsValidRect(RM::Rect r)
	{
		return ((!total) && (!zone.Intersects(r)));
	}

	RM::RectMerger ExtractRect(RM::Rect r)
	{
		RM::RectMerger result;
		if (!total)
		{
			result = zone.ExtractRect(r);
			return result;
		}
		else
		{
			int tmp_area = zone.GetArea();
			result = zone.ExtractRect(r);
			if (zone.GetArea() < tmp_area)
				total = false;
			return result;
		}
	}
};

class Texture
{
	shared_com<ID3D11Texture2D> pTexture;
	shared_com<ID3D11Texture2D> pCPUResource;
	
	shared_com<ID3D11ShaderResourceView> pSRV;

	void* pMemory;
	int pitch;
	int flags;

	int width;
	int height;

	//bool bLocked = false;
	//bool bMapped = false;

	std::vector<WORD> data;
	std::vector<RM::Rect> rLocked;

	Invalidator GPU;
	Invalidator CPU;

	friend class D3DHost;
public:
	bool bForceGPU = false;
	bool bForceCPU = false;
	bool bLocked = false;

	int   GetPitch()   const { return pitch; }
	void* GetAddress() const { return pMemory; }

	bool IsStaging() { return (flags & F_STAGING)!=0; }
	bool IsCPUReadable() { return (flags & F_CPU_READ)!=0; }
	bool IsCPUWriteable() { return (flags & F_CPU_WRITE)!=0; }

	int GetXSize() { return width; }
	int GetYSize() { return height; }

	int handle = 0;
};

class Model
{
	shared_com<ID3D11Buffer> m_vertexBuffer;
	shared_com<ID3D11Buffer> m_indexBuffer;

	int vertexCount;
	int indexCount;

	friend class D3DHost;
public:

};

class D3DHost
{
public:
	D3DHost();
	~D3DHost();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetVideoCardInfo(char*, int&);
	void SetResolution(int screenWidth, int screenHeight);

	Texture       CreateTexture(int x_size, int y_size, TextureFormat format, int flags);
	void          SetTexture(Texture& t);
	bool          MapTexture(Texture& t, RECT* pArea);
	bool          Unmap(Texture& t);

	void          Unlock(Texture& t);

	void          CopyTexture(Texture& dst, Texture& src);
	void          CopySubTexture(Texture& dst, Texture& src, int x, int y, RECT rct);

	Model         CreateModel();
	void          SetModel(Model& m);

	void          Frame(Texture& t);

	void          FillTexture(Texture& t, DWORD color, RECT* pRect);
	void          UpdateSubtexture(Texture& t, LPRECT pDstRect, LPVOID memory, DWORD pitch);

private:
	void ValidateTextureGPU(Texture& t);
	void ValidateTextureCPU(Texture& t);
	void Validate(ID3D11Resource* pDst, ID3D11Resource* pSrc, Invalidator& inv);

	void ValidateTextureRectCPU(Texture& t, RM::Rect r);
	void ValidateTextureRectGPU(Texture& t, RM::Rect r);
	void ValidateRect(ID3D11Resource* pDst, ID3D11Resource* pSrc, RM::Rect r, Invalidator& inv);

	bool CheckTimer();

	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];

	LARGE_INTEGER timestamp;
	double accumulated_time;
	double frame_time;

	Model mod_square;
	
	shared_com<IDXGISwapChain> m_swapChain;
	shared_com<ID3D11Device> m_device;
	shared_com<ID3D11Device1> m_device1;
	shared_com<ID3D11DeviceContext> m_deviceContext;
	shared_com<ID3D11RenderTargetView> m_renderTargetView;
	shared_com<ID3D11Texture2D> m_depthStencilBuffer;
	shared_com<ID3D11DepthStencilState> m_depthStencilState;
	shared_com<ID3D11DepthStencilView> m_depthStencilView;
	shared_com<ID3D11RasterizerState> m_rasterState;

	shared_com<ID3D11VertexShader> m_vs_square;
	shared_com<ID3D11PixelShader> m_ps_16bit;

	shared_com<ID3D11InputLayout> m_layout;
	
	void InitializeShaders();
};


