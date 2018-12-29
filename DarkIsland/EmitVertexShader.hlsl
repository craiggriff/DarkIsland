// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix mvp;
};

// Per-vertex data used as input to the vertex shader.
struct VertexInputType
{
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float2 tex : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexInputType input)
{
	PixelShaderInput output;
	float4 pos = input.position;
	pos.w = 1.0f;
	output.positionW = mul(pos, worldMatrix).xyz;

	// Transform the vertex position into projected space.
	pos = mul(pos, worldMatrix);
	pos = mul(pos, viewMatrix);
	pos = mul(pos, projectionMatrix);

	//pos.z -= 0.1f;
	output.position = pos;

	output.tex = input.tex;

	// Pass the color through without modification.
	//output.color = input.color;

	return output;
}