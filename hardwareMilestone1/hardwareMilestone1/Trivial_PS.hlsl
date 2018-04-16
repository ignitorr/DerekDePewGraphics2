Texture2D tex : register(t0);
SamplerState sampl : register(s0);

float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
	return tex.Sample(sampl, uv);
}

/*
float4 main(float4 colorFromRasterizer : COLOR) : SV_TARGET
{
	return colorFromRasterizer;
}
*/