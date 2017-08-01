#include "stdafx.h"
#include "D3DHost.h"
#include "Recorder.h"
#include "SurfaceProcessor.h"

#define CHECK_CONTEXT if(!m_deviceContext.get()) return

D3DHost::D3DHost() : m_swapChain(nullptr),
                     m_device(nullptr),
	                 m_deviceContext(nullptr),
	                 m_renderTargetView(nullptr),
	                 m_depthStencilBuffer(nullptr),
	                 m_depthStencilState(nullptr),
	                 m_depthStencilView(nullptr),
	                 m_rasterState(nullptr)
{}

struct vec2
{
	float x;
	float y;
};

struct vec3
{
	float x;
	float y;
	float z;
};

struct vec4
{
	float x;
	float y;
	float z;
	float w;
};

struct VertexType
{
	vec2 position;
	vec2 texcoord;
};

D3DHost::~D3DHost()
{
}

HRESULT CreateSwapChain(IDXGIFactory1* dxgiFactory, ID3D11Device* md3dDevice, DXGI_SWAP_CHAIN_DESC* sd, IDXGISwapChain** mSwapChain)
{
	return dxgiFactory->CreateSwapChain(md3dDevice, sd, mSwapChain);
}

bool D3DHost::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen)
{
	HRESULT result;

	m_downscale = true;
	if (m_downscale)
	{
		screenWidth /= 2;
		screenHeight /= 2;
	}

	res_x = screenWidth;
	res_y = screenHeight;

	//	fullscreen = false;

	shared_com<IDXGIFactory2> factory;
	std::vector<shared_com<IDXGIAdapter1>> vAdapters;
	shared_com<IDXGIOutput> adapterOutput = nullptr;

	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;

	shared_com<ID3D11Texture2D> backBufferPtr;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;

	accumulated_time = 0.0;
	frame_time = 1.0 / 30;

	// Store the vsync setting.
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&factory);
	if (FAILED(result))
	{
		LOG("FU1\r\n");
		return false;
	}

	IDXGIAdapter1* adapter;
	// Use the factory to create an adapter for the primary graphics interface (video card).
	for (unsigned int i = 0;; i++)
	{
		result = factory->EnumAdapters1(i, &adapter);

		if (FAILED(result))
			break;

		vAdapters.emplace_back(adapter);
	}

	if (vAdapters.empty()) return false;

	// Enumerate the primary adapter output (monitor).
	for (auto& l_adapter : vAdapters)
	{
		result = l_adapter->EnumOutputs(0, &adapterOutput);
		adapter = l_adapter.get();
		if (SUCCEEDED(result)) break;
	}

	if (FAILED(result))
	{
		LOG("FU3\r\n");
		return false;
	}


	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		LOG("FU4\r\n");
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	auto displayModeList = std::unique_ptr<DXGI_MODE_DESC[]>(new DXGI_MODE_DESC[numModes]);
	if (!displayModeList)
	{
		LOG("FU5\r\n");
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.get());
	if (FAILED(result))
	{
		LOG("FU6\r\n");
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}


	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		LOG("FU7\r\n");
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		LOG("FU8\r\n");
		return false;
	}

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B5G6R5_UNORM;

	// Set the refresh rate of the back buffer.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = !fullscreen;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_DESC1 sd1{};
	sd1.BufferCount = swapChainDesc.BufferCount;
	sd1.BufferUsage = swapChainDesc.BufferUsage;
	sd1.Flags = swapChainDesc.Flags;
	sd1.Format = swapChainDesc.BufferDesc.Format;
	sd1.Height = swapChainDesc.BufferDesc.Height;
	sd1.Width = swapChainDesc.BufferDesc.Width;
	sd1.Stereo = false;
	sd1.Scaling = DXGI_SCALING_STRETCH;
	sd1.SampleDesc = swapChainDesc.SampleDesc;
	sd1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd1{};
	fd1.RefreshRate = swapChainDesc.BufferDesc.RefreshRate;
	fd1.Scaling = swapChainDesc.BufferDesc.Scaling;
	fd1.ScanlineOrdering = swapChainDesc.BufferDesc.ScanlineOrdering;
	fd1.Windowed = swapChainDesc.Windowed;
	
	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	for (auto &adapter : vAdapters)
	{
			// Create the swap chain, Direct3D device, and Direct3D device context.
			//result = D3D11CreateDeviceAndSwapChain(adapter, driverTypes[driverTypeIndex], NULL, 0, &featureLevel, 1,
			//	D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);

			result = D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_device, nullptr, &m_deviceContext);

			if (SUCCEEDED(result))
			{
				result = factory->CreateSwapChainForHwnd
				  (m_device.get(), hwnd, &sd1, &fd1, nullptr, &m_swapChain);
					//CreateSwapChain(m_device.get(), &swapChainDesc, &m_swapChain);
				if (FAILED(result))
				{
					LOG("FU9\r\n");
					return false;
				}
				else break;
			}
	}

	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG("FU10\r\n");
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr.get(), NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		LOG("FU11\r\n");
		return false;
	}

	
	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		LOG("FU12\r\n");
		return false;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		LOG("FU13\r\n");
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.get(), 1);
	
	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer.get(), &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		LOG("FU14\r\n");
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView.get());
	
	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		LOG("FU15\r\n");
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState.get());
	
	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);

	InitializeShaders();

	mod_square = CreateModel();

	return true;
}

void D3DHost::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	m_rasterState = nullptr;
	m_depthStencilView = nullptr;
	m_depthStencilState = nullptr;
	m_depthStencilBuffer = nullptr;
	m_renderTargetView = nullptr;
	m_deviceContext = nullptr;
	m_device = nullptr;
    m_swapChain = nullptr;

	return;
}

void D3DHost::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.get(), color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_deviceContext->IASetInputLayout(m_layout.get());

	m_deviceContext->PSSetShader(m_ps_16bit_downscale.get(), nullptr, 0);
	m_deviceContext->VSSetShader(m_vs_square.get(), nullptr, 0);

	SetModel(mod_square);

	return;
}

void D3DHost::SetDebugMonitorValues(float a, float b, float c, float d)
{
    DebugmonBuffer db{};
    db.v1 = a;
    db.v2 = b;
    db.v3 = c;
    db.v4 = d;

    m_deviceContext->UpdateSubresource(m_debugmon_buffer.get(), 0, nullptr, &db, 0, 0);
}

void D3DHost::DrawDebugMonitor()
{
    m_deviceContext->PSSetShader(m_ps_debugmon.get(), nullptr, 0);
    m_deviceContext->PSSetConstantBuffers(0, 1, &m_debugmon_buffer);
    m_deviceContext->VSSetShader(m_vs_debugmon.get(), nullptr, 0);

    SetModel(mod_square);
    m_deviceContext->DrawIndexed(mod_square.indexCount, 0, 0);
}

void D3DHost::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

ID3D11Device* D3DHost::GetDevice()
{
	return m_device.get();
}


ID3D11DeviceContext* D3DHost::GetDeviceContext()
{
	return m_deviceContext.get();
}

void D3DHost::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

void D3DHost::SetResolution(int screenWidth, int screenHeight)
{
	DXGI_MODE_DESC params{};
	params.Width = screenWidth;
	params.Height = screenHeight;
	//m_swapChain->ResizeTarget();
}

Texture D3DHost::CreateTexture(int x_size, int y_size, TextureFormat format, int flags)
{
    if (!m_device.get()) return{};
	Texture result;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = result.width = x_size;
	desc.Height = result.height = y_size;
	desc.MipLevels = desc.ArraySize = 1;
	
	result.flags = flags;

	switch (format)
	{
	case TF_16BIT:	desc.Format = DXGI_FORMAT_R16_UINT; break;
	case TF_32BIT:	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
	}
	
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = m_device->CreateTexture2D(&desc, NULL, &result.pTexture);

	if (hr != S_OK)
	{
		LOG("FU\r\n");
		return result;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	srv_desc.Texture2D.MostDetailedMip = 0;


	hr = m_device->CreateShaderResourceView(result.pTexture.get(), &srv_desc, &result.pSRV);


	if (hr != S_OK)
	{
		LOG("FU\r\n");
		return result;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};

	uav_desc.Format = desc.Format;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	srv_desc.Texture2D.MostDetailedMip = 0;

	m_device->CreateUnorderedAccessView(result.pTexture.get(), &uav_desc, &result.pUAV);

	if (hr != S_OK)
	{
		LOG("FU\r\n");
		return result;
	}

	if (flags & F_STAGING)
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;

		if (flags & F_CPU_WRITE)
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}

		if (flags & F_CPU_READ)
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		//desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

		HRESULT hr = m_device->CreateTexture2D(&desc, NULL, &result.pCPUResource);
		if (hr != S_OK)
		{
			LOG("FU\r\n");
			return result;
		}
	}
//	if (flags & F_GDI) desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	
	return result;
}

void D3DHost::SetTexture(Texture & t)
{
    CHECK_CONTEXT;
	ID3D11UnorderedAccessView* pNullUAV = NULL;
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, NULL);

	m_deviceContext->PSSetShaderResources(0, 1, &t.pSRV);
}

bool D3DHost::MapTexture(Texture & t, RECT* pArea)
{
    CHECK_CONTEXT false;
	D3D11_MAPPED_SUBRESOURCE subr;
	D3D11_MAP map_flags;

	force_log(UpdateTimer(), ": LOCK ", t.handle, ' ', t.GetXSize(), 'x', t.GetYSize());

	if (pArea)
		force_log_i("GPU map rect ", t.handle, " (", pArea->left, ", ", pArea->top, ", ",
			pArea->right, ", ", pArea->bottom, ") from ",t.GetXSize(),'x',t.GetYSize(),"\r\n");
	else
		force_log_i("GPU map full ", t.handle, "\r\n");
	/*if (t.bMapped)
	{
		t.bLocked = true;
		force_log(" (S)\r\n");
		return true;
	}*/

	force_log("\r\n");

	if (pArea)
	{
		RM::Rect rArea{ RM::Point{pArea->left,pArea->top}, RM::Point{pArea->right,pArea->bottom} };
		ValidateTextureRectCPU(t, rArea);
		t.rLocked.push_back(rArea);
	}
	else
		ValidateTextureCPU(t);

	if (t.IsCPUReadable())
	{
		if (t.IsCPUWriteable())
		{
			map_flags = D3D11_MAP_READ_WRITE;
		}
		else
		{
			map_flags = D3D11_MAP_READ;
		}
	}
	else
	{
		if (t.IsCPUWriteable())
		{
			map_flags = D3D11_MAP_WRITE;
		}
		else
			return false;
	}


	HRESULT hr = m_deviceContext->Map(t.pCPUResource.get(), 0, map_flags, 0, &subr);
	//m_deviceContext->Map()
	if (hr != S_OK)
	{
		hr = m_device->GetDeviceRemovedReason();
		return false;
	}
	t.pMemory = subr.pData;
	t.pitch   = subr.RowPitch;
	//t.bLocked = true;
	//t.bMapped = true;
	t.bLocked = true;

	force_log(UpdateTimer(), ": ---\r\n");

	return true;
}

long long extern g_datarec;

void D3DHost::Unlock(Texture& t)
{
    CHECK_CONTEXT;
	force_log_i("GPU unmap ",t.handle,"\r\n");
	t.bLocked = false;
	if (t.rLocked.empty())
	{
		t.GPU.InvalidateAll(t.width, t.height);

		if (global_is_recording)
		{
			RecStructModifySurface rs(t.handle);
            rs.data_rec = g_datarec;
            g_datarec += t.width*t.height;
			record_struct(rs);
			record_stream((WORD*)t.GetAddress(), t.width, t.height, t.GetPitch() / 2);
            gSP.ModifyRect(t.handle, t.FullRect());
		}
	}
	else
	{
		for (RM::Rect r : t.rLocked)
	    {
			t.GPU.InvalidateRect(r);

			if (global_is_recording)
			{
				RECT rr = r.toRECT();
				RecStructModifySubSurface rs(t.handle, RecRect(&rr));
                rs.data_rec = g_datarec;
                g_datarec += rs.rect.getSize();
				record_struct(rs);
    			WORD* pS = (WORD*)t.GetAddress();
				pS += (t.GetPitch() / 2)*rr.top + rr.left;
				record_stream(pS, r.Width(), r.Height(), t.GetPitch() / 2);
                gSP.ModifyRect(t.handle, &rr);
			}
		}
		t.rLocked.clear();
    }

	m_deviceContext->Unmap(t.pCPUResource.get(), 0);
}

bool D3DHost::Unmap(Texture & t)
{
	/*if (!t.bLocked) return true;

	t.bLocked = false;
	
	if (t.IsCPUWriteable())
	{
		t.GPU.InvalidateAll(t.width, t.height);
	}
	*/
	Unlock(t);
	return true;
}

void D3DHost::CopyTexture(Texture & dst, Texture & src)
{
    CHECK_CONTEXT;
	force_log_i("Copy full from ", src.handle, " to ",dst.handle,"\r\n");

	ID3D11Resource* pSrcResource = src.GPU.IsValid() ? src.pTexture.get() : src.pCPUResource.get();
	m_deviceContext->CopyResource(dst.pTexture.get(), pSrcResource);
	dst.CPU.InvalidateAll(dst.width, dst.height);
	dst.GPU.Validate();
}

void D3DHost::CopySubTexture(Texture & dst, Texture & src, int x, int y, RECT rct)
{
    CHECK_CONTEXT;
	force_log_i("Copy rect from ", src.handle, " (",rct.left,", ",rct.top," - ",rct.right-rct.left,'x',rct.bottom-rct.top,") to ",
		dst.handle, "(",x,", ",y,")\r\n");

	int blit_sx = rct.right - rct.left;
	int blit_sy = rct.bottom - rct.top;

	D3D11_BOX SrcBox;
	SrcBox.front = 0;
	SrcBox.back = 1;
	SrcBox.left = rct.left;
	SrcBox.right = rct.right;
	SrcBox.top = rct.top;
	SrcBox.bottom = rct.bottom;

	RM::Rect dst_rct(x, y, blit_sx, blit_sy);

	ValidateTextureRectGPU(src, rct);
	ValidateTextureRectGPU(dst, dst_rct);

	m_deviceContext->CopySubresourceRegion(dst.pTexture.get(), 0, x, y, 0,
		src.pTexture.get(), 0, &SrcBox);

	dst.CPU.InvalidateRect(x, y, x + blit_sx, y + blit_sy);

	return;
	/*if (dst.bForceCPU)
	{
		ValidateTextureCPU(src);
		ValidateTextureCPU(dst);
		// CPU copy
		return;
	}*/

	//if (dst.bForceGPU)
		ValidateTextureGPU(src);

//	force_log_r("COPY SUB ", dst.handle, ' ', src.handle, " (",src.flags,") - ", rct.right - rct.left, 'x', rct.bottom - rct.top, "\r\n");


	if (dst.GPU.IsValid() && src.GPU.IsValid())
	{
		force_log(" (GPU)\r\n");

		ValidateTextureGPU(src);

		m_deviceContext->CopySubresourceRegion(dst.pTexture.get(), 0, x, y, 0,
			src.pTexture.get(), 0, &SrcBox);

		dst.CPU.InvalidateRect(x,y,x + blit_sx, y + blit_sy);

		force_log(UpdateTimer(), ": ---\r\n");

		return;
	}

	if (dst.CPU.IsValid() && src.CPU.IsValid())
	{
		force_log(" (CPU)\r\n");
		// CPU copy
		return;
	}

	if (dst.GPU.IsValid())
	{
		int dstarea = dst.CPU.zone.GetArea();
		int srcarea = src.GPU.zone.GetArea();
		if (dstarea < srcarea)
		{
			force_log(" (CPU)\r\n");
			ValidateTextureCPU(dst);
			// CPU copy
		}
		else
		{
			force_log(" (GPU)\r\n");
			ValidateTextureGPU(src);
			m_deviceContext->CopySubresourceRegion(dst.pTexture.get(), 0, x, y, 0,
				src.pTexture.get(), 0, &SrcBox);

			dst.CPU.InvalidateRect(x, y, x + blit_sx, y + blit_sy);
		}
		return;
	}

	int dstarea = dst.GPU.zone.GetArea();
	int srcarea = src.CPU.zone.GetArea();

	if (dstarea > srcarea)
	{
		force_log(" (CPU)\r\n");
		ValidateTextureCPU(src);
		// CPU copy
	}
	else
	{
		force_log(" (GPU)\r\n");
		ValidateTextureGPU(dst);
		m_deviceContext->CopySubresourceRegion(dst.pTexture.get(), 0, x, y, 0,
			src.pTexture.get(), 0, &SrcBox);

		dst.CPU.InvalidateRect(x, y, x + blit_sx, y + blit_sy);
	}

	return;
}

void D3DHost::BlitTransparent(Texture & dst, Texture & src, int x, int y, RECT rct, DWORD colorkey)
{
    CHECK_CONTEXT;

	force_log_i("Transparent blit from ", src.handle, " (", rct.left, ", ", rct.top, " - ", rct.right - rct.left, 'x', rct.bottom - rct.top, ") to ",
		dst.handle, "(", x, ", ", y, ")\r\n");

	int blit_sx = rct.right - rct.left;
	int blit_sy = rct.bottom - rct.top;

	if ((blit_sx < 0) || (blit_sy < 0))
	{
		return;
	}

	D3D11_BOX SrcBox;
	SrcBox.front = 0;
	SrcBox.back = 1;
	SrcBox.left = rct.left;
	SrcBox.right = rct.right;
	SrcBox.top = rct.top;
	SrcBox.bottom = rct.bottom;

	RM::Rect dst_rct(x, y, blit_sx, blit_sy);

	ValidateTextureRectGPU(src, rct);
	ValidateTextureRectGPU(dst, dst_rct);

	BlitFXBuffer fx{};
	fx.doff_x = x;
	fx.doff_y = y;
	fx.soff_x = rct.left;
	fx.soff_y = rct.top;
	fx.ckey = colorkey;

	m_deviceContext->UpdateSubresource(m_blit_fx.get(), 0, nullptr, &fx, 0, 0);

	ID3D11ShaderResourceView* pNullSRV = NULL;
	m_deviceContext->PSSetShaderResources(0, 1, &pNullSRV);
	m_deviceContext->CSSetShaderResources(0, 1, &pNullSRV);

	ID3D11UnorderedAccessView* pNullUAV = NULL;
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, NULL);

	m_deviceContext->CSSetShader(m_cs_blit.get(), nullptr, 0);
	m_deviceContext->CSSetShaderResources(0, 1, &src.pSRV);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &dst.pUAV, nullptr);
	m_deviceContext->CSSetConstantBuffers(0, 1, &m_blit_fx);
	m_deviceContext->Dispatch(blit_sx, blit_sy, 1);

	dst.CPU.InvalidateRect(x, y, x + blit_sx, y + blit_sy);

	return;
}

void D3DHost::BlitMirrored(Texture & dst, Texture & src, int x, int y, RECT rct, DWORD colorkey)
{
    CHECK_CONTEXT;

	force_log_i("Mirrored blit from ", src.handle, " (", rct.left, ", ", rct.top, " - ", rct.right - rct.left, 'x', rct.bottom - rct.top, ") to ",
		dst.handle, "(", x, ", ", y, ")\r\n");

	int blit_sx = rct.right - rct.left;
	int blit_sy = rct.bottom - rct.top;


	if ((blit_sx < 0) || (blit_sy < 0))
	{
		return;
	}

	D3D11_BOX SrcBox;
	SrcBox.front = 0;
	SrcBox.back = 1;
	SrcBox.left = rct.left;
	SrcBox.right = rct.right;
	SrcBox.top = rct.top;
	SrcBox.bottom = rct.bottom;

	RM::Rect dst_rct(x, y, blit_sx, blit_sy);

	ValidateTextureRectGPU(src, rct);
	ValidateTextureRectGPU(dst, dst_rct);

	BlitFXBuffer fx{};
	fx.doff_x = x;
	fx.doff_y = y;
	fx.soff_x = rct.left;
	fx.soff_y = rct.top;
	fx.ckey = colorkey;

	m_deviceContext->UpdateSubresource(m_blit_fx.get(), 0, nullptr, &fx, 0, 0);

	ID3D11ShaderResourceView* pNullSRV = NULL;
	m_deviceContext->PSSetShaderResources(0, 1, &pNullSRV);
	m_deviceContext->CSSetShaderResources(0, 1, &pNullSRV);

	ID3D11UnorderedAccessView* pNullUAV = NULL;
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, NULL);

	m_deviceContext->CSSetShader(m_cs_mirror.get(), nullptr, 0);
	m_deviceContext->CSSetShaderResources(0, 1, &src.pSRV);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &dst.pUAV, nullptr);
	m_deviceContext->CSSetConstantBuffers(0, 1, &m_blit_fx);
	m_deviceContext->Dispatch(blit_sx, blit_sy, 1);

	dst.CPU.InvalidateRect(x, y, x + blit_sx, y + blit_sy);

	return;
}

Model D3DHost::CreateModel()
{
    CHECK_CONTEXT{};

	Model result;

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr;
	
	// Set the number of vertices in the vertex array.
	result.vertexCount = 4;

	// Set the number of indices in the index array.
	result.indexCount = 4;

	// Create the vertex array.
	std::vector<VertexType> vertices(result.vertexCount);
	std::vector<unsigned long> indices(result.indexCount);

	
	// Load the vertex array with data.
	vertices[0] = VertexType{ { -1.0f, -1.0f },  { 0.f,1.f } };  // Top left.
	vertices[1] = VertexType{ {  1.0f, -1.0f },  { 1.f,1.f } };  // Top left.
	vertices[2] = VertexType{ {  1.0f,  1.0f },  { 1.f,0.f } };  // Top left.
	vertices[3] = VertexType{ {  -1.0f,  1.0f },  { 0.f,0.f } };  // Top left.

	// Load the index array with data.
	indices[0] = 0;  
	indices[1] = 3;  
	indices[2] = 1;  
	indices[3] = 2; 
	//indices[4] = 0;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * result.vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	hr = m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &result.m_vertexBuffer);
	
	if (FAILED(hr))
	{
		result.m_vertexBuffer = nullptr;
		return result;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * result.indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = m_device->CreateBuffer(&indexBufferDesc, &indexData, &result.m_indexBuffer);
	if (FAILED(hr))
	{
		result.m_vertexBuffer = nullptr;
		return result;
	}

	return result;
}

void D3DHost::SetModel(Model & m)
{
    CHECK_CONTEXT;
    unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_deviceContext->IASetVertexBuffers(0, 1, &m.m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	m_deviceContext->IASetIndexBuffer(m.m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}


void D3DHost::ForceFrame(Texture & t)
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	
    static LARGE_INTEGER start_time = current_time;

	static const LARGE_INTEGER baseline = []()
	{
		LARGE_INTEGER r;
		QueryPerformanceFrequency(&r);
		return r;
	}();

    int bss = global_encoding_buffer.Size();
    FORCE_LOG(bss);
    FORCE_LOG(' ');

    bss = global_processing_buffer.Size();
    FORCE_LOG(bss);
    FORCE_LOG(' ');

    bss = global_recording_buffer.Size();
    FORCE_LOG(bss);
    FORCE_LOG("\r\n");

	double fr = static_cast<double>(current_time.QuadPart) / baseline.QuadPart;

    double ip;
    fr = 60.0*modf(fr/60.0, &ip);

	RecStructFrameTimestamp rs{ static_cast<int>(fr*10000)&0x7FFFFFFF };
	record_struct(rs);

    CHECK_CONTEXT;

	force_log_i("=========== frame =========== \r\n");
	ValidateTextureGPU(t);

	float r = (rand() % 1024) / 1024.f;
	float g = (rand() % 1024) / 1024.f;
	float b = (rand() % 1024) / 1024.f;

	BeginScene(r, g, b, 0.0f);

	SetTexture(t);
	m_deviceContext->DrawIndexed(mod_square.indexCount, 0, 0);

#ifdef DEBUG_MONITOR


    SetDebugMonitorValues(global_encoding_buffer.Size()/static_cast<float>(global_encoding_buffer.max_size()),
        global_processing_buffer.Size() / static_cast<float>(global_processing_buffer.max_size()), 
        global_recording_buffer.Size() / static_cast<float>(global_recording_buffer.max_size()), 0.f);

    DrawDebugMonitor();

#endif

	EndScene();

	if (global_is_recording_ps)
	{
		ValidateTextureCPU(t);
		MapTexture(t, nullptr);

		record_stream((WORD*)t.GetAddress(), t.width, t.height, t.GetPitch() / 2);
        gSP.ModifyRect(t.handle, t.FullRect());

		Unmap(t);
	}
}


void D3DHost::Frame(Texture & t)
{
	if (CheckTimer())
	{
		ForceFrame(t);
	}
}

void D3DHost::FillTexture(Texture & t, DWORD color, RECT * pRect)
{
    CHECK_CONTEXT;

	int w, h;
	BlitFXBuffer fx{};

	if (pRect)
	{
		w = pRect->right - pRect->left;
		h = pRect->bottom - pRect->top;

		w = std::min<int>(w, t.width - pRect->left);
		h = std::min<int>(h, t.height - pRect->top);

		fx.doff_x = pRect->left;
		fx.doff_y = pRect->top;

	}
	else
	{
		w = t.width;
		h = t.height;
	}

	if ((w < 0) || (h < 0))
	{
		return;
	}

	fx.ckey = color;

	m_deviceContext->UpdateSubresource(m_blit_fx.get(), 0, nullptr, &fx, 0, 0);
	
	ID3D11ShaderResourceView* pNullSRV = NULL;
	m_deviceContext->PSSetShaderResources(0, 1, &pNullSRV);
	m_deviceContext->CSSetShaderResources(0, 1, &pNullSRV);

	m_deviceContext->CSSetShader(m_cs_fill.get(), nullptr, 0);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &t.pUAV, nullptr);
	m_deviceContext->CSSetConstantBuffers(0, 1, &m_blit_fx);
	m_deviceContext->Dispatch(w, h, 1);

	t.CPU.InvalidateRect(fx.doff_x, fx.doff_y, fx.doff_x + w, fx.doff_y + h);

	return;
}

void D3DHost::UpdateSubtexture(Texture & t, LPRECT rct, LPVOID memory, DWORD pitch)
{
    CHECK_CONTEXT;
    ValidateTextureCPU(t);
	D3D11_BOX SrcBox;
	SrcBox.front = 0;
	SrcBox.back = 1;
	if (rct)
	{
		SrcBox.left = rct->left;
		SrcBox.right = rct->right;
		SrcBox.top = rct->top;
		SrcBox.bottom = rct->bottom;
	}
	else
	{
		SrcBox.left = 0;
		SrcBox.right = t.width;
		SrcBox.top = 0;
		SrcBox.bottom = t.height;
	}
	m_deviceContext->UpdateSubresource(t.pCPUResource.get(), 0, &SrcBox, memory, pitch, 0);
	t.GPU.InvalidateRect(SrcBox.left, SrcBox.top, SrcBox.right, SrcBox.bottom);
}

void D3DHost::ValidateTextureGPU(Texture & t)
{
	if (t.GPU.IsValid())
		return;

/*	if (t.bMapped)
	{
		Unlock(t);
	}*/

//	force_log_r("TO GPU: ", t.handle, "\r\n");
	Validate(t.pTexture.get(), t.pCPUResource.get(), t.GPU);
	force_log(UpdateTimer(), ": ---\r\n");

}

void D3DHost::ValidateTextureCPU(Texture & t)
{
	if (t.CPU.IsValid())
		return;


//	force_log_r("TO CPU: ", t.handle, "\r\n");
	Validate(t.pCPUResource.get(), t.pTexture.get(), t.CPU);
	force_log(UpdateTimer(), ": ---\r\n");

}

void D3DHost::Validate(ID3D11Resource * pDst, ID3D11Resource * pSrc, Invalidator & inv)
{
	if (inv.total)
	{
		m_deviceContext->CopyResource(pDst, pSrc);
	}
	else
	{
		for (RM::Rect r : inv.zone)
		{
			D3D11_BOX SrcBox;
			SrcBox.front = 0;
			SrcBox.back = 1;
			SrcBox.left = r.Left();
			SrcBox.right = r.Right();
			SrcBox.top = r.Top();
			SrcBox.bottom = r.Bottom();
			m_deviceContext->CopySubresourceRegion(pDst, 0, r.Left(), r.Top(), 0, pSrc, 0, &SrcBox);
		}
	}

	inv.Validate();
}

void D3DHost::ValidateTextureRectCPU(Texture & t, RM::Rect r)
{
	return ValidateTextureCPU(t);
	//if (t.CPU.IsValidRect(r))
	//	return;
	
	force_log_i("CPU Validating ", t.handle, "(", r.Left(),
		", ", r.Top(), ", ", r.Right(), ", ", r.Bottom(), ") of ",
		t.GetXSize(), 'x', t.GetYSize(), "\r\n");

	ValidateRect(t.pCPUResource.get(), t.pTexture.get(), r, t.CPU);
}

void D3DHost::ValidateTextureRectGPU(Texture & t, RM::Rect r)
{
	return ValidateTextureGPU(t);

	//if (t.GPU.IsValidRect(r))
	//{
	//	return;
	//}

	force_log_i("GPU Validating ", t.handle, "(",r.Left(),
		", ",r.Top(),", ",r.Right(),", ",r.Bottom(), ") of ",
		t.GetXSize(),'x',t.GetYSize(),"\r\n");

	ValidateRect(t.pTexture.get(), t.pCPUResource.get(), r, t.GPU);
}

void D3DHost::ValidateRect(ID3D11Resource * pDst, ID3D11Resource * pSrc, RM::Rect r, Invalidator & inv)
{
	RM::RectMerger e = inv.ExtractRect(r);
	force_log_i("Extracted : ", e.end() - e.begin(),"\r\n");
	for (auto rr : e)
	{
		force_log_i(rr.Left(), ", ",rr.Top(),", ", rr.Right(),", ", rr.Bottom(),"\r\n");
	}

	if (e.begin() != e.end())
	{
		RM::Rect r = *e.begin();
		for (RM::Rect l : e)
		{
			r = RM::CombineRects(r, l);
		}


		//for (RM::Rect r : e)
		{
			D3D11_BOX SrcBox;
			SrcBox.front = 0;
			SrcBox.back = 1;
			SrcBox.left = r.Left();
			SrcBox.right = r.Right();
			SrcBox.top = r.Top();
			SrcBox.bottom = r.Bottom();
			m_deviceContext->CopySubresourceRegion(pDst, 0, r.Left(), r.Top(), 0, pSrc, 0, &SrcBox);
		}
	}
}

bool D3DHost::CheckTimer()
{
	static const LARGE_INTEGER baseline = []()
	 {
		LARGE_INTEGER r;
		QueryPerformanceFrequency(&r);
		return r;
	}();
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	
	double passed = (current_time.QuadPart - timestamp.QuadPart) / static_cast<double>(baseline.QuadPart);

	double pp = passed;

	timestamp = current_time;

	if (passed >= frame_time)
	{
		passed = frame_time;
	}

	accumulated_time += passed;

	if (accumulated_time >= frame_time)
	{
		force_log("Time elapsed: ", pp, "\r\n");
		accumulated_time -= frame_time;
		return true;
	}

	return false;
}