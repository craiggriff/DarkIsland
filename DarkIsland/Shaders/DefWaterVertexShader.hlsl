////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////
#include "../DefShader.h"

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register (b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix mvp;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	//float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float3 normal : NORMAL;
	float3 scr_normal : NORMAL2;
	//float2 tex : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	output.positionW = mul(input.position, worldMatrix).xyz;
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.scr_normal = mul(input.normal, worldMatrix);
	output.scr_normal = mul(output.scr_normal, viewMatrix);
	output.scr_normal = mul(output.scr_normal, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	//output.tex = input.tex;

	output.normal = input.normal;

	return output;
}