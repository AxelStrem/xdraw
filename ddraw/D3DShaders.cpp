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