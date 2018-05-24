#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float2 tex : TEXCOORD0;
	float3 norm : NORMAL;
	float3 tangent : TANGENT;
	uint instanceID : SV_INSTANCEID;
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

cbuffer THIS_IS_VRAM : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
	float4 offset;
	float4 inm;
	float4 camPos;
	float emissive;
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{


	OUTPUT_VERTEX newVert = (OUTPUT_VERTEX)0;

	newVert.projectedCoordinate.w = 1;
	newVert.projectedCoordinate.xyz = fromVertexBuffer.coordinate.xyz;

	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, world);
	if (bool(inm.x))
	{
		newVert.projectedCoordinate.xyz += fromVertexBuffer.instanceID * offset.xyz;
	}
	newVert.worldPos = newVert.projectedCoordinate;
	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, view);
	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, proj);
	newVert.texOut = fromVertexBuffer.tex;
	newVert.norm = mul(float4(fromVertexBuffer.norm, 0), world).xyz;
	newVert.tangent = mul(fromVertexBuffer.tangent, world);

	newVert.normalMap = bool(inm.y);
	newVert.specMap = bool(inm.w);
	newVert.multiTex = bool(inm.z);

	newVert.camPos = newVert.worldPos.xyz - camPos.xyz;
	newVert.camPos = normalize(newVert.camPos);

	newVert.emissive = emissive;

	return newVert;
}