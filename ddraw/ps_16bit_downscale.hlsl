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

int3 tc = int3((input.texcoord.x*xs), (input.texcoord.y*ys), 0);

uint w1 = shaderTexture.Load(tc + int3(0, 0, 0)).r;
uint w2 = shaderTexture.Load(tc + int3(1, 0, 0)).r;
uint w3 = shaderTexture.Load(tc + int3(0, 1, 0)).r;
uint w4 = shaderTexture.Load(tc + int3(1, 1, 0)).r;


int3 col1;
col1.r = ((w1 & RED_MASK) >> RED_SHIFT) << (6 - RED_WIDTH);
col1.g = ((w1 & GREEN_MASK) >> GREEN_SHIFT) << (6 - GREEN_WIDTH);
col1.b = ((w1 & BLUE_MASK) >> BLUE_SHIFT) << (6 - BLUE_WIDTH);


int3 col2;
col2.r = ((w2 & RED_MASK) >> RED_SHIFT) << (6 - RED_WIDTH);
col2.g = ((w2 & GREEN_MASK) >> GREEN_SHIFT) << (6 - GREEN_WIDTH);
col2.b = ((w2 & BLUE_MASK) >> BLUE_SHIFT) << (6 - BLUE_WIDTH);


int3 col3;
col3.r = ((w3 & RED_MASK) >> RED_SHIFT) << (6 - RED_WIDTH);
col3.g = ((w3 & GREEN_MASK) >> GREEN_SHIFT) << (6 - GREEN_WIDTH);
col3.b = ((w3 & BLUE_MASK) >> BLUE_SHIFT) << (6 - BLUE_WIDTH);


int3 col4;
col4.r = ((w4 & RED_MASK) >> RED_SHIFT) << (6 - RED_WIDTH);
col4.g = ((w4 & GREEN_MASK) >> GREEN_SHIFT) << (6 - GREEN_WIDTH);
col4.b = ((w4 & BLUE_MASK) >> BLUE_SHIFT) << (6 - BLUE_WIDTH);

float3 cf = (col1 + col2 + col3 + col4) / 255.f;
//cf = mul(cf, m);
return float4(cf,0.f);
}
