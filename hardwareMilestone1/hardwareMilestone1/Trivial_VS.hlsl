#pragma pack_matrix( row_major )

struct INPUT_VERTEX
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct OUTPUT_VERTEX
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

// TODO: PART 3 STEP 2a
cbuffer MATRIX_DATA : register( b0 )
{
	matrix world;
	matrix view;
	matrix proj;
};

OUTPUT_VERTEX main( INPUT_VERTEX input )
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	output.pos = (input.pos, 1);
	
	output.pos = mul(output.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);
	output.color = input.color;
	//sendToRasterizer.color = fromVertexBuffer.color;

	/*
	sendToRasterizer.projectedCoordinate.xy = fromVertexBuffer.coordinate.xy;
		
	// TODO : PART 4 STEP 4
	sendToRasterizer.projectedCoordinate.xy += constantOffset;
	
	// TODO : PART 3 STEP 7
	sendToRasterizer.colorOut = constantColor;
	// END PART 3
	*/

	return output;
}