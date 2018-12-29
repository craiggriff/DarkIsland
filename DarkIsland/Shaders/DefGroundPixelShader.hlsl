////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#include "../DefShader.h"

//Texture2D depthTexture : register(t4);

SamplerState SampleType : register(s0);

TextureCube environmentMapCube : register(t5);

Texture2DArray textureArray[8] : register(t20);
Texture2DArray normalArray[8] : register(t30);

TextureCubeArray pointshadowMapCubeArray[MAX_POINT_SHADOWS] : register(t60);

Texture2DArray lightmapArray[8] : register(t40);

struct PointLight
{
	float radius;
	float3 pos;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dir;
	float spot;
	float specular_power;
	int shadow_cube_index;
	float spare;
	int lightmap;
	matrix dirView; // directional light view and projection
	matrix dirProj;
};

StructuredBuffer<PointLight> lights : register(t50);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
	float4 fogColor;
	float4 skyColor;

	float3 eye_position;
	uint numPointLights;
	uint numSpotLights;
	float lightning;
	float3 lightning_dir;
	float3 pos;
	float screen_width;
	float screen_height;
	float texel_width;
	float texel_height;
};

cbuffer MaterialBuffer : register(b4)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
	float normal_height;

	float spare1mat;
	float spare2mat;
};

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 viewDirection : TEXCOORD1;
	float3 positionW : TEXCOORD2;
	float cam_dist : TEXCOORD3;
	float tex_blend : TEXBLEND1;
	float tex_blend2 : TEXBLEND2;
	float tex_blend3 : TEXBLEND3;
	float tex_blend4 : TEXBLEND4;
	float tex_blend5 : TEXBLEND5;
	float tex_blend6 : TEXBLEND6;
	float tex_blend7 : TEXBLEND7;
	float tex_blend8 : TEXBLEND8;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct PixelOutputType
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 position : SV_Target2;
	float4 specular : SV_Target3;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PixelOutputType main(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;

	float4 textureColor = float4(0.0f,0.0f,0.0f,0.0f);

	float4 normalMap = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 n = input.normal;

	textureColor += input.tex_blend*textureArray[0].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend2*textureArray[1].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend3*textureArray[2].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend4*textureArray[3].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend5*textureArray[4].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend6*textureArray[5].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend7*textureArray[6].Sample(SampleType, float3(input.tex, 1.0f));
	textureColor += input.tex_blend8*textureArray[7].Sample(SampleType, float3(input.tex, 1.0f));

if (textureColor.a < 0.3f) discard;

if (normal_height > 0.0f)//normalLvl == 1.0f)
{
	//discard;
	if (input.cam_dist < 150.0f)
	{
		normalMap += input.tex_blend*normalArray[0].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend2*normalArray[1].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend3*normalArray[2].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend4*normalArray[3].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend5*normalArray[4].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend6*normalArray[5].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend7*normalArray[6].Sample(SampleType, float3(input.tex, 1.0f));
		normalMap += input.tex_blend8*normalArray[7].Sample(SampleType, float3(input.tex, 1.0f));
		/*
		normalMap = normalArray[0].Sample(SampleType, float3(input.tex,1.0f));

		if (input.tex_blend > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[1].Sample(SampleType, float3(input.tex, 1.0f)), 1.0f);
		}

		if (input.tex_blend2 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[2].Sample(SampleType, float3(input.tex, 1.0f)), 1.0f);
		}

		if (input.tex_blend3 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[3].Sample(SampleType, float3(input.tex, 1.0f)), input.tex_blend3);
		}

		if (input.tex_blend4 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[4].Sample(SampleType, float3(input.tex, 1.0f)), input.tex_blend4);
		}

		if (input.tex_blend5 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[5].Sample(SampleType, float3(input.tex, 1.0f)), input.tex_blend4);
		}

		if (input.tex_blend4 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[4].Sample(SampleType, float3(input.tex, 1.0f)), input.tex_blend4);
		}

		if (input.tex_blend7 > 0.0f)
		{
			normalMap = lerp(normalMap, normalArray[7].Sample(SampleType, float3(input.tex, 1.0f)), input.tex_blend7);
		}
		*/
		normalMap = ((normalMap * (2.0f*normal_height)) - normal_height);

		n = (normalMap.x * input.tangent) + (normalMap.y * input.binormal) + (normalMap.z * input.normal);

		n = normalize(n);

		if (input.cam_dist > 100.0f)
		{
			n = lerp(n, input.normal, (input.cam_dist - 100.0f)*0.1f);
		}
	}
	else
	{
		n = input.normal;
	}
}

// Sample the color from the texture and store it for output to the render target.
output.color = textureColor;// *input.color;

// Store the normal for output to the render target.
output.normal = float4(n, 0.0f);

output.position = float4(input.positionW, input.position.z / input.position.w);

output.specular = float4(0.0f, 0.0f, 0.0f, MaterialSpecular.w);

return output;// float4(1.0f, 0.0f, 1.0f, 1.0f);
}