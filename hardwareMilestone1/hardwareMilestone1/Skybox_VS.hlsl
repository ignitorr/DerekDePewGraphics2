#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float2 tex : TEXCOORD0;
	float3 norm : NORMAL;
};

struct OUTPUT_VERTEX
{
	float3 texOut : TEXCOORD0;
	float4 coordinate : SV_POSITION;
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
	float3 cameraPos;
};

OUTPUT_VERTEX main(INPUT_VERTEX input)
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	
	output.coordinate.xyz = input.coordinate.xyz;
	output.coordinate.w = 1;
	
	float3 texPos = output.coordinate.xyz;
	texPos.y *= -1;
	output.texOut = texPos;

	output.coordinate = mul(output.coordinate, world);
	output.coordinate = mul(output.coordinate, view);
	output.coordinate = mul(output.coordinate, proj);
	//output.coordinate.z = 1;
	

	return output;
}