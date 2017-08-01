/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float2 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
    PixelInputType output;


    // Change the position vector to be 4 units for proper matrix calculations.
    output.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position.z = 0.4f;
    output.position.xy = (input.position + 1.f)*0.15f - 1.f;
    // Store the input color for the pixel shader to use.
    output.texcoord = input.texcoord;

    return output;
}
