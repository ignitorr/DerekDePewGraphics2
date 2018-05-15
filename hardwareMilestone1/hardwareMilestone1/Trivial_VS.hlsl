#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	//float4 color : COLOR;
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
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{


	OUTPUT_VERTEX newVert = (OUTPUT_VERTEX)0;

	newVert.projectedCoordinate.w = 1;

	uint row = fromVertexBuffer.instanceID / 10;
	uint offset = fromVertexBuffer.instanceID % 10;

	newVert.projectedCoordinate.xyz = fromVertexBuffer.coordinate.xyz;
	newVert.projectedCoordinate.x -= 2.0f * row;
	newVert.projectedCoordinate.z += 2.0f * offset;
	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, world);
	newVert.worldPos = newVert.projectedCoordinate;
	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, view);
	newVert.projectedCoordinate = mul(newVert.projectedCoordinate, proj);
	//newVert.colorOut = fromVertexBuffer.color;
	newVert.texOut = fromVertexBuffer.tex;
	newVert.norm = mul(float4(fromVertexBuffer.norm, 0), world).xyz;

	return newVert;
}