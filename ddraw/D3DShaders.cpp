#include "stdafx.h"
#include "D3DHost.h"

namespace vs_square
{
#include "vs_square.h"
}

namespace ps_16bit
{
#include "ps_16bit.h"
}

namespace ps_16bit_downscale
{
#include "ps_16bit_downscale.h"
}

namespace cs_blit
{
#include "cs_blit.h"
}

namespace cs_fill
{
#include "cs_fill.h"
}

namespace cs_mirror
{
#include "cs_mirror.h"
}

namespace vs_debugmon
{
#include "vs_debugmon.h"
}

namespace ps_debugmon
{
#include "ps_debugmon.h"
}


void D3DHost::InitializeShaders()
{
	HRESULT hr;
	hr = m_device->CreateVertexShader(vs_square::g_main, sizeof(vs_square::g_main), nullptr, &m_vs_square);
	if (FAILED(hr))
	{
		return;
	}
	hr = m_device->CreatePixelShader(ps_16bit::g_main, sizeof(ps_16bit::g_main), nullptr, &m_ps_16bit);
	if (FAILED(hr))
	{
		return;
	}

    hr = m_device->CreateVertexShader(vs_debugmon::g_main, sizeof(vs_debugmon::g_main), nullptr, &m_vs_debugmon);
    if (FAILED(hr))
    {
        return;
    }
    hr = m_device->CreatePixelShader(ps_debugmon::g_main, sizeof(ps_debugmon::g_main), nullptr, &m_ps_debugmon);
    if (FAILED(hr))
    {
        return;
    }

	hr = m_device->CreatePixelShader(ps_16bit_downscale::g_main, sizeof(ps_16bit_downscale::g_main), nullptr, &m_ps_16bit_downscale);
	if (FAILED(hr))
	{
		return;
	}

	hr = m_device->CreateComputeShader(cs_blit::g_main, sizeof(cs_blit::g_main), nullptr, &m_cs_blit);
	if (FAILED(hr))
	{
		return;
	}

	hr = m_device->CreateComputeShader(cs_fill::g_main, sizeof(cs_fill::g_main), nullptr, &m_cs_fill);
	if (FAILED(hr))
	{
		return;
	}

	hr = m_device->CreateComputeShader(cs_mirror::g_main, sizeof(cs_mirror::g_main), nullptr, &m_cs_mirror);
	if (FAILED(hr))
	{
		return;
	}

	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.ByteWidth = sizeof(BlitFXBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	BlitFXBuffer buf{};
	buf.ckey = 0x0FF0;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = &buf;

	hr = m_device->CreateBuffer(&cbDesc, &data, &m_blit_fx);
	if (FAILED(hr))
	{
		return;
	}

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Create the vertex input layout.
	hr = m_device->CreateInputLayout(polygonLayout, 2, vs_square::g_main, sizeof(vs_square::g_main), &m_layout);
	if (FAILED(hr))
	{
		return;
	}
}