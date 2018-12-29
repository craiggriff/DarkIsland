////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#include "../DefShader.h"

Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float3 normal : NORMAL;
	float3 scr_normal : NORMAL2;
	//float2 tex : TEXCOORD0;
};

struct PixelOutputType
{
	float4 position : SV_Target0;
	float4 normal : SV_Target1;
	float4 scr_normal : SV_Target2;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PixelOutputType main(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;

	float4 textureColor;

	//textureColor = shaderTexture.Sample(SampleType, input.tex);

	// Store the normal for output to the render target.
	output.normal = float4(input.normal, 0.0f);
	output.scr_normal = float4(input.scr_normal, 0.0f);

	output.position = float4(input.positionW, input.position.z / input.position.w);

	//if (false)//(textureColor.a < 0.2f)
	//	discard;

	return output;
}