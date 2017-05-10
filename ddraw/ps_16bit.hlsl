//////////////
// TYPEDEFS //
//////////////

static const int BLUE_MASK = 31;
static const int  GREEN_MASK = 2016;
static const int  RED_MASK = 63488;

static const int  BLUE_SHIFT = 0;
static const int  GREEN_SHIFT = 5;
static const int  RED_SHIFT = 11;

static const int  BLUE_WIDTH = 5;
static const int  GREEN_WIDTH = 6;
static const int  RED_WIDTH = 5;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

Texture2D<uint> shaderTexture;

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float xs,ys;
	shaderTexture.GetDimensions(xs, ys);

//	input.texcoord.x = (0.5*sin(input.texcoord.x * 5) + 0.5*cos(input.texcoord.y * 7))*0.5f + 0.5f;
//	input.texcoord.y = (0.3*sin(input.texcoord.x * 2) + 0.7*cos(input.texcoord.y * 4))*0.5f + 0.5f;

	/*input.texcoord.x = 1 / (1.f - input.texcoord.x) - 1.f;
	input.texcoord.y = 1 / (1.f - input.texcoord.y) - 1.f;
	*/

	/*float xx = input.texcoord.x;
	input.texcoord.x *= input.texcoord.y;
	input.texcoord.y /= xx;

	input.texcoord -= floor(input.texcoord);*/


	uint w = shaderTexture.Load(int3((input.texcoord.x*xs),(input.texcoord.y*ys),0)).r;

	int3 col;
	col.r = ((w & RED_MASK) >> RED_SHIFT) << (8 - RED_WIDTH);
	col.g = ((w & GREEN_MASK) >> GREEN_SHIFT) << (8 - GREEN_WIDTH);
	col.b = ((w & BLUE_MASK) >> BLUE_SHIFT) << (8 - BLUE_WIDTH);
/*
	float3x3 m;
	m[0] = float3((0.5*sin(input.texcoord.x * 10) + 0.5*cos(input.texcoord.y * 20))*0.7f,
		(0.5*sin(input.texcoord.x * 20) + 0.5*cos(input.texcoord.y * 40))*0.7f,
		(0.5*sin(input.texcoord.x * 30) + 0.5*cos(input.texcoord.y * 10))*0.7f);
	m[1] = float3((0.5*sin(20 + input.texcoord.x * 15) + 0.5*cos(5+input.texcoord.y * 0.3))*0.7f,
		(0.5*sin(15+input.texcoord.x * 25) + 0.5*cos(6+input.texcoord.y * 04))*0.7f,
		(0.5*sin(10+input.texcoord.x * 35) + 0.5*cos(7+input.texcoord.y * 05))*0.7f);
	//m[2] = float3((0.5*sin(17+input.texcoord.x * 12) + 0.5*cos(1+input.texcoord.y * 1.1))*0.5f + 0.5f,
	//	(0.5*sin(7+input.texcoord.x * 23) + 0.5*cos(2+input.texcoord.y * 39))*0.5f + 0.5f,
	//	(0.5*sin(input.texcoord.x * 34) + 0.5*cos(1+input.texcoord.y * 11))*0.5f + 0.5f);
	
	m[2] = 1.f - (m[0] + m[1]);*/

	float3 cf = col / 255.f;
	//cf = mul(cf, m);
    return float4(cf,0.f);
}
