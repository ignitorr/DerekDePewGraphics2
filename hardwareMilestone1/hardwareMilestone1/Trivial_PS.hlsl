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
	float coneRatio;
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
	// POINT LIGHT CODE
	float4 final = tex.Sample(sampl, vert.texOut);
	float4 ambient = final * float4(0.25, 0.25, 0.25, 1);
	float att = 1.0 - clamp(length(lightPos - vert.worldPos) / lightRad, 0, 1);
	float4 lightdir = normalize(lightPos - vert.worldPos);
	float lightRatio = clamp(dot(lightdir, normalize(vert.norm)), 0, 1);
	final *= saturate(lightRatio * lightColor);
	final.a = 1;
	return saturate((final * att) + ambient);
	*/

	// SPOTLIGHT CODE
	/*
	*/
	float4 final = tex.Sample(sampl, vert.texOut);
	float4 ambient = final * float4(0.25, 0.25, 0.25, 1);
	float4 lightdir = normalize(lightPos - vert.worldPos);
	float lightRatio = clamp(dot(lightdir, normalize(vert.norm)), 0, 1);
	float surfaceRatio = clamp(dot(-lightdir, lightDirection), 0, 1);
	
	float att = clamp(length(lightPos - vert.worldPos) / lightRad, 0, 1);
	att *= att;
	att = 1.0 - att;

	float innerCone = coneRatio + 0.005f;
	float edgeAtt = 1.0 - clamp((innerCone - surfaceRatio) / (innerCone - coneRatio), 0, 1);
	/*
	float spotFactor;
	if (surfaceRatio > coneRatio)
	{
		spotFactor = 1;
	}
	else
	{
		spotFactor = 0;
	}
	*/
	final *= saturate(lightRatio * lightColor);
	//return saturate((final * spotFactor) + ambient);
	return saturate((final * edgeAtt * att) + ambient);
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