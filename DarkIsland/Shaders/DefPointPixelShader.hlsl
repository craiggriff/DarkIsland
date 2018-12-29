////////////////////////////////////////////////////////////////////////////////
// Filename: particle.ps
////////////////////////////////////////////////////////////////////////////////
#include "../DefShader.h"

/////////////
// GLOBALS //
/////////////
Texture2DArray shaderTexture[4] : register(t0);

Texture2D normalTexture : register(t3);

SamplerState SampleType : register(s0);

cbuffer MaterialBuffer : register(b4)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
	float normal_height;

	float spare1;
	float spare2;
};

struct PixelOutputType
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 position : SV_Target2;
	float4 specular : SV_Target3;
};

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2A;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
	float3 cam_dir : TEXCOORD3A;
	uint tex_num : TEXNUM0;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PixelOutputType main(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;

	float4 tex_sam;

	if (input.tex_num == 0)
	{
		tex_sam = shaderTexture[0].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 1)
	{
		tex_sam = shaderTexture[1].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 2)
	{
		tex_sam = shaderTexture[2].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 3)
	{
		tex_sam = shaderTexture[3].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}

	if (tex_sam.w < 0.1f)
	{
		discard;
	}

	output.color = tex_sam * input.color;
	output.normal = float4(input.cam_dir,1.2f);

	output.position = float4(input.positionW, input.position.z / input.position.w);

	output.specular = float4(MaterialSpecular.x*MaterialSpecular.w, MaterialSpecular.y*MaterialSpecular.w, MaterialSpecular.z*MaterialSpecular.w, MaterialSpecular.w);

	return output;
}