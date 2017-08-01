struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
}; 

cbuffer data
{
    float4 values;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 v = values*0.25f;
    float y = input.texcoord.y;
    if(y < v.x)
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
    y -= v.x;

    if (y < v.y)
        return float4(0.0f, 1.0f, 0.0f, 1.0f);
    y -= v.y;

    if (y < v.z)
        return float4(0.0f, 0.0f, 1.0f, 1.0f);
    y -= v.z;

    if (y < v.w)
        return float4(1.0f, 0.0f, 1.0f, 1.0f);
    y -= v.w;

    return float4(0.f, 0.f, 0.f, 1.f);
}