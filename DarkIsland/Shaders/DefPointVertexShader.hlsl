////////////////////////////////////////////////////////////////////////////////
// Filename: particle.vs
////////////////////////////////////////////////////////////////////////////////

#include "../DefShader.h"

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix mvp;
};

cbuffer VLightBuffer : register(b5)
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

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	uint tex_num : TEXNUM;
	float2 padding : PADDING;
};

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
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	input.position.w = 1.0f;

	output.positionW = mul(input.position, worldMatrix).xyz;

	output.tex_num = input.tex_num;

	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	//input.color.w = 1.0f;
	output.color = input.color;// *ambientColor*(diffuseColor*0.5);
	//output.color.w = 1.0f;

	output.cam_dir = eye_position - output.positionW;

	// Store the particle color for the pixel shader.

	return output;
}