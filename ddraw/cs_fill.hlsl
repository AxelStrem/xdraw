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
void main(uint3 DTid : SV_DispatchThreadID)
{
	int2 doffset = int2(dox, doy);
	uint sx, sy;
	gOutput.GetDimensions(sx, sy);
	int2 out_size = int2(sx, sy);

	doffset = (DTid.xy + doffset + out_size) % out_size;

	gOutput[doffset] = ckey;
}