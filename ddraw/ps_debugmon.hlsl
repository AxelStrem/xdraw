struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
}; 

float4 main(PixelInputType input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}