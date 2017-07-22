Texture2D<uint> shaderTexture   : register(t0);
RWTexture2D<uint>   gOutput   : register(u0);
cbuffer fx : register(b0)
{
	int sox;
	int soy;
	int dox;
	int doy;
	int ckey;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int2 doffset = int2(dox, doy);
	int2 soffset = int2(sox, soy);
	uint sx, sy;
	gOutput.GetDimensions(sx, sy);
	int2 out_size = int2(sx, sy);
	shaderTexture.GetDimensions(sx, sy);
	int2 in_size = int2(sx, sy);

	doffset = (DTid.xy + doffset + out_size) % out_size;
	soffset = (DTid.xy + soffset + in_size) % in_size;

	int color = shaderTexture.Load(int3(soffset, 0));
	if (color != ckey)
		gOutput[doffset] = color;
}