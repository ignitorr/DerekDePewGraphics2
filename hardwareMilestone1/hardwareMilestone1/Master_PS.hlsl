Texture2D tex[2] : register(t0);
Texture2D normalTex : register(t2);
Texture2D specularTex : register(t3);
SamplerState sampl : register(s0);

#define D_LIGHTS 1
#define P_LIGHTS 1
#define S_LIGHTS 1

#define SPEC_POWER 128

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


struct OUTPUT_VERTEX
{
	float2 texOut : TEXCOORD0;
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPos : WORLDPOS;
	float3 norm : NORMAL;
	float3 tangent : TANGENT;
	bool normalMap : NMAP;
	bool specMap : SPEC;
	bool multiTex : MULTI;
	float3 camPos : CAMPOS;
	float emissive : EMM;
};

float4 main(OUTPUT_VERTEX vert) : SV_TARGET
{
	float4 lightFinal = float4(0, 0, 0, 1);
	float4 specFinal = float4(0, 0, 0, 1);

	float4 specIntensity = (specularTex.Sample(sampl, vert.texOut)) * 1.5f;

	float4 final;
	if (vert.multiTex)
	{
		float4 tex0 = tex[0].Sample(sampl, vert.texOut);
		float4 tex1 = tex[1].Sample(sampl, vert.texOut);
		final = tex0 * tex1 * 2.0f;
	}
	else
	{
		final = tex[0].Sample(sampl, vert.texOut);
	}
	// NEW NORMAL MAPPING CODE //
	if (vert.normalMap)
	{

	float4 normalMap = normalTex.Sample(sampl, vert.texOut);
	normalMap = 2.0f * normalMap - 1.0f;

	vert.tangent = normalize(vert.tangent - dot(vert.tangent, vert.norm) * vert.norm);

	float3 biTangent = cross(vert.norm, vert.tangent);

	float3x3 textureSpace = float3x3(vert.tangent, biTangent, vert.norm);

	vert.norm = normalize(mul(normalMap, textureSpace));
	}
	// END NORMAL MAPPING CODE //

	float4 ambient = final * float4(0.15, 0.15, 0.15, 1);
	float4 emmFinal = final * float4(vert.emissive, vert.emissive, vert.emissive, 1);

	// DIRECTIONAL LIGHTS
	for (uint i = 0; i < D_LIGHTS; i++)
	{
		float ratio = dot(normalize(-dLights[i].lightDirection.xyz), normalize(vert.norm)) * dLights[i].lightColor;
		float4 lightEffect = saturate(ratio);

		lightFinal += saturate(lightEffect);

		// SPECULAR
		if (vert.specMap)
		{
			float3 toCam = vert.camPos;
			float3 toLight = dLights[i].lightDirection.xyz;
			float3 reflectionVec = normalize(reflect(-toCam, vert.norm));
			float spec = dot(reflectionVec, toLight);
			spec = pow(spec, SPEC_POWER);

			float4 specEffect = (dLights[i].lightColor * spec * specIntensity);
			specEffect *= ratio;

			specFinal += saturate(specEffect);
		}
	}

	// POINT LIGHTS
	for (uint i = 0; i < P_LIGHTS; i++)
	{
		float att = 1.0 - clamp(length(pLights[i].lightPos - vert.worldPos) / pLights[i].lightRad, 0, 1);
		float4 lightdir = normalize(pLights[i].lightPos - vert.worldPos);
		float lightRatio = clamp(dot(lightdir, normalize(vert.norm)), 0, 1);


		float4 lightEffect = saturate(lightRatio * pLights[i].lightColor);
		lightEffect *= att;

		lightFinal += saturate(lightEffect);

		// SPECULAR
		if (vert.specMap)
		{
			float3 toCam = vert.camPos;
			float3 toLight = normalize(pLights[i].lightPos - vert.worldPos);
			float3 reflectionVec = normalize(reflect(-toCam, vert.norm));
			float spec = dot(reflectionVec, toLight);
			spec = pow(spec, SPEC_POWER);

			float4 specEffect = (pLights[i].lightColor * spec * specIntensity);
			specEffect *= lightRatio * att;

			specFinal += saturate(specEffect);
		}
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

		float4 lightEffect = saturate(lightRatio * sLights[i].lightColor);
		lightEffect *= att * edgeAtt;

		lightFinal += saturate(lightEffect);

		// SPECULAR
		if (vert.specMap)
		{
			float3 toCam = vert.camPos;
			float3 toLight = normalize(sLights[i].lightPos - vert.worldPos);
			float3 reflectionVec = normalize(reflect(-toCam, vert.norm));
			float spec = dot(reflectionVec, toLight);
			spec = pow(spec, SPEC_POWER);

			float4 specEffect = (sLights[i].lightColor * spec * specIntensity);
			specEffect *= att * edgeAtt;

			specFinal += saturate(specEffect);
		}
	}

	return (final * lightFinal) + ambient + specFinal + emmFinal;
}
