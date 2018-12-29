Texture2D shaderTexture : register(t0);

Texture2D normalTexture : register(t3);

Texture2D emmitTexture : register(t4);

SamplerState SampleTypeWrap : register(s0);

cbuffer MaterialBuffer : register(b4)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
	float normal_height;

	float emmit_brightness;
	float spare2;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float2 tex : TEXCOORD0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 outputColor = emmitTexture.Sample(SampleTypeWrap, input.tex);

	return outputColor * emmit_brightness;// float4(1.0f, 1.0f, 1.0f, 1.0f);// outputColor;
}