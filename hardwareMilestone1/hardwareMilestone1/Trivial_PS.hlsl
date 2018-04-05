/*
float4 main( float4 colorFromRasterizer : COLOR ) : SV_TARGET
{
	return colorFromRasterizer;
}


*/

struct INPUT_PIXEL
{
	float4 pos : SV_POSITION;
	//float4 color : COLOR;
};

float4 main( INPUT_PIXEL input) : SV_TARGET
{
	return float4(0, 1.0f, 1.0f, 1.0f);
}