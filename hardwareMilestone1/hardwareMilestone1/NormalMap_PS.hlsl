Texture2D tex[2] : register(t0);
SamplerState sampl : register(s0);

#define D_LIGHTS 1
#define P_LIGHTS 1
#define S_LIGHTS 1

//////////////////////
// LIGHTING STRUCTS //
//////////////////////
struct directional_light
{
	float4 lightDirection;
	float4 lightColor;
};

struct point_light
{
	float4 lightPos;
	float4 lightColor;
	float lightRad;
};

struct spot_light
{
	float4 lightDirection;
	float4 lightPos;
	float4 lightColor;
	float lightRad;
	float outerConeRatio;
	float innerConeRatio;
};
//////////////////////////
// END LIGHTING STRUCTS //
//////////////////////////

cbuffer THIS_IS_VRAM : register(b0)
{
	directional_light dLights[D_LIGHTS];
	point_light pLights[P_LIGHTS];
	spot_light sLights[S_LIGHTS];
};

/*
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
*/

struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPos : WORLDPOS;
	float3 norm : NORMAL;
	float3 tangent : TANGENT;
};

float4 main(OUTPUT_VERTEX vert) : SV_TARGET
{

	float4 lightFinals[D_LIGHTS + P_LIGHTS + S_LIGHTS];

	//float4 tex0 = tex[0].Sample(sampl, vert.texOut);
	//float4 tex1 = tex[1].Sample(sampl, vert.texOut);
	//float4 final = tex0 * tex1 * 2.0f;
	//float4 final = (tex0 * 0.5f) + (tex1 * 0.5f);

	// NEW NORMAL MAPPING CODE //
	float4 final = tex[0].Sample(sampl, vert.texOut);

	float4 normalMap = tex[1].Sample(sampl, vert.texOut);
	normalMap = 2.0f * normalMap - 1.0f;

	vert.tangent = normalize(vert.tangent - dot(vert.tangent, vert.norm) * vert.norm);

	float3 biTangent = cross(vert.norm, vert.tangent);

	float3x3 textureSpace = float3x3(vert.tangent, biTangent, vert.norm);

	vert.norm = normalize(mul(normalMap, textureSpace));

	// END NORMAL MAPPING CODE //

	float4 ambient = final * float4(0.15, 0.15, 0.15, 1);
	
	uint currentLight = 0;

	
	// DIRECTIONAL LIGHTS
	for (uint i = 0; i < D_LIGHTS; i++)
	{
		//final += saturate(dot(normalize(-dLights[i].lightDirection.xyz), normalize(vert.norm)) * dLights[i].lightColor);
		//final.a = 1;
		lightFinals[currentLight] = saturate(dot(normalize(-dLights[i].lightDirection.xyz), normalize(vert.norm)) * dLights[i].lightColor);

		currentLight += 1;
	}

	// POINT LIGHTS
	for (uint i = 0; i < P_LIGHTS; i++)
	{
		float att = 1.0 - clamp(length(pLights[i].lightPos - vert.worldPos) / pLights[i].lightRad, 0, 1);
		float4 lightdir = normalize(pLights[i].lightPos - vert.worldPos);
		float lightRatio = clamp(dot(lightdir, normalize(vert.norm)), 0, 1);

		lightFinals[currentLight] = saturate(lightRatio * pLights[i].lightColor);
		lightFinals[currentLight] *= att;

		currentLight += 1;
	}

	// SPOT LIGHTS
	for (uint i = 0; i < S_LIGHTS; i++)
	{
		float4 lightdir = normalize(sLights[i].lightPos - vert.worldPos);
		float lightRatio = clamp(dot(lightdir, normalize(vert.norm)), 0, 1);
		float surfaceRatio = clamp(dot(-lightdir, sLights[i].lightDirection), 0, 1);

		float att = clamp(length(sLights[i].lightPos - vert.worldPos) / sLights[i].lightRad, 0, 1);
		att *= att;
		att = 1.0 - att;

		float edgeAtt = 1.0 - clamp((sLights[i].innerConeRatio - surfaceRatio) / (sLights[i].innerConeRatio - sLights[i].outerConeRatio), 0, 1);

		lightFinals[currentLight] = saturate(lightRatio * sLights[i].lightColor);
		lightFinals[currentLight] *= att * edgeAtt;

		currentLight += 1;
	}




	float4 finalcolor;
	finalcolor.x = 1;
	finalcolor.y = 1;
	finalcolor.z = 1;
	finalcolor.w = 1;
	for (uint i = 0; i < (D_LIGHTS + P_LIGHTS + S_LIGHTS); i++)
	{
		finalcolor += saturate(lightFinals[i]);
	}
	return (final * finalcolor) - final + ambient;
	//return saturate(final + ambient);
}
