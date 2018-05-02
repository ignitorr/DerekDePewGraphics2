Texture2D tex : register(t0);
SamplerState sampl : register(s0);

struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPos : WORLDPOS;
	float3 norm : NORMAL;
};


// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
};

float4 main(OUTPUT_VERTEX vert) : SV_TARGET
{
	return tex.Sample(sampl, vert.texOut);
	//return texCUBE(sampl, vert.texOut);
}