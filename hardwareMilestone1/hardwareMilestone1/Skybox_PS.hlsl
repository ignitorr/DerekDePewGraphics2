TextureCube tex : register(t0);
SamplerState sampl : register(s0);

struct OUTPUT_VERTEX
{
	float3 texOut : TEXCOORD0;
	float4 coordinate : SV_POSITION;
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