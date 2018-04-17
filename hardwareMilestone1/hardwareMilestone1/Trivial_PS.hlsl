Texture2D tex : register(t0);
SamplerState sampl : register(s0);

cbuffer THIS_IS_VRAM : register(b0)
{
	//float4 constantColor;
	//float2 constantOffset;
	//float2 padding;
	matrix world;
	matrix view;
	matrix proj;
	float4 lightDirection;
	float4 lightColor;
};

struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	float3 norm : NORMAL;
};

float4 main(OUTPUT_VERTEX vert) : SV_TARGET
{

	//float4 final = 0;
	float4 final = tex.Sample(sampl, vert.texOut);
	float4 ambient = final * float4(0.5, 0.5, 0.5, 1);
	final *= saturate(dot(normalize(-lightDirection.xyz), normalize(vert.norm)) * lightColor);
	//final = lightColor;
	final.a = 1;
	return saturate(final + ambient);
}

/*
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
	return tex.Sample(sampl, uv);
}
*/

/*
float4 main(float4 colorFromRasterizer : COLOR) : SV_TARGET
{
	return colorFromRasterizer;
}
*/