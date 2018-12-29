// This is the deferred pixel shader.

////////////////////////////////////////////////////////////////////////////////
// Filename: deferred.ps
////////////////////////////////////////////////////////////////////////////////
#include "../DefShader.h"

//////////////
// TEXTURES //
//////////////
Texture2D shaderTexture : register(t0);

Texture2D normalTexture : register(t3);

///////////////////
// SAMPLE STATES //
///////////////////
SamplerState SampleTypeWrap : register(s0);

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

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float cam_dist : TEXCOORD3;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

// We have a new structure for the pixel shader output.This will output color data to the first render texture target, and normals to the second render texture target.

struct PixelOutputType
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 position : SV_Target2;
	float4 specular : SV_Target3;
};

// The pixel shader now uses a different output type that we defined above instead of just outputting a single floating point color value.It will now write to the two render targets defined in the struct.We send the color texture pixel to the first render target, and we send the normals to the second render target.The two textures that are output from this shader will then be used as the input data for all of our new re - written shaders.

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PixelOutputType main(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;
float4 normalMap;
float3 n = input.normal;

// Sample the color from the texture and store it for output to the render target.
output.color = shaderTexture.Sample(SampleTypeWrap, input.tex);

if (output.color.a < 0.3f) discard;

if (normal_height > 0.0f)//normalLvl == 1.0f)
{
	//discard;
	if (input.cam_dist < 25.0f)
	{
		normalMap = normalTexture.Sample(SampleTypeWrap, input.tex);

		normalMap = ((normalMap * (2.0f*normal_height)) - normal_height);

		n = (normalMap.x * input.tangent) + (normalMap.y * input.binormal) + (normalMap.z * input.normal);

		n = normalize(n);

		if (input.cam_dist > 15.0f)
		{
			n = lerp(n, input.normal, (input.cam_dist - 15.0f)*0.1f);
		}
	}
	else
	{
		n = input.normal;
	}
}

// Store the normal for output to the render target.
output.normal = float4(n, 0.0f);

output.position = float4(input.positionW, 1.0f);

output.specular = float4(0.0f,0.0f,0.0f, MaterialSpecular.w);

//output.normal.y = 0.0f;
return output;
}