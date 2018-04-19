#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	//float4 color : COLOR;
	float2 tex : TEXCOORD0;
	float3 norm : NORMAL;

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
	//float4 constantColor;
	//float2 constantOffset;
	//float2 padding;
	matrix world;
	matrix view;
	matrix proj;
	float4 lightDirection;
	float4 lightColor;
	float4 lightPos;
	float lightRad;
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate.w = 1;

	sendToRasterizer.projectedCoordinate.xyz = fromVertexBuffer.coordinate.xyz;
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, world);
	sendToRasterizer.worldPos = sendToRasterizer.projectedCoordinate;
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, view);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, proj);
	//sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.texOut = fromVertexBuffer.tex;
	sendToRasterizer.norm = mul(float4(fromVertexBuffer.norm, 0), world).xyz;

	return sendToRasterizer;
}