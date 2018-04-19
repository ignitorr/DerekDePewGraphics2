Texture2D tex : register(t0);
SamplerState sampl : register(s0);

cbuffer THIS_IS_VRAM : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
	float4 lightDirection;
	float4 lightColor;
	float4 lightPos;
	float lightRad;
};

struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPos : WORLDPOS;
	float3 norm : NORMAL;
};

float4 main(OUTPUT_VERTEX vert) : SV_TARGET
{
	// DIRECTIONAL LIGHT CODE
	/*
	//float4 final = 0;
	float4 final = tex.Sample(sampl, vert.texOut);
	float4 ambient = final * float4(0.25, 0.25, 0.25, 1);
	final *= saturate(dot(normalize(-lightDirection.xyz), normalize(vert.norm)) * lightColor);
	final.a = 1;
	return saturate(final + ambient);
	*/
	/*
	*/
	// POINT LIGHT CODE
	float4 final = tex.Sample(sampl, vert.texOut);
	float4 ambient = final * float4(0.25, 0.25, 0.25, 1);
	float att = 1.0 - clamp(length(lightPos - vert.worldPos) / lightRad, 0, 1);
	float4 lightdir = normalize(lightPos - vert.worldPos);
	//lightdir.w = 1;
	float4 lightRatio = dot(lightdir, normalize(vert.norm));
	final *= saturate(lightRatio * lightColor);
	final.a = 1;
	//return saturate(final + ambient);
	return saturate((final * att) + ambient);
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