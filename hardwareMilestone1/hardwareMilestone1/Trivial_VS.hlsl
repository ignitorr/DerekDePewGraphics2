#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float4 color : COLOR;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
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
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate.w = 1;

	//sendToRasterizer.projectedCoordinate.xy = fromVertexBuffer.coordinate.xy;
	sendToRasterizer.projectedCoordinate.xyz = fromVertexBuffer.coordinate.xyz;
	// TODO : PART 4 STEP 4
	//sendToRasterizer.projectedCoordinate.xy += constantOffset;
	//sendToRasterizer.projectedCoordinate.x += constantOffset.x;
	//sendToRasterizer.projectedCoordinate.y += constantOffset.y;
	// TODO : PART 3 STEP 7
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, world);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, view);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, proj);
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	// END PART 3

	return sendToRasterizer;
}