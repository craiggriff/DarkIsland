//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

Texture2D diffuseTexture : register(t0);
SamplerState linearSampler : register(s0);



cbuffer ConstantBufferNeverChanges : register(b10)
{
    float4 lightPosition[4];
    float4 lightColor;
}

cbuffer ConstantBufferChangeOnResize : register(b11)
{
    matrix projection;
};

cbuffer ConstantBufferChangesEveryFrame : register(b12)
{
    matrix view;
};

cbuffer ConstantBufferChangesEveryPrim : register (b13)
{
    matrix world;
    float4 meshColor;
    float4 diffuseColor;
    float4 specularColor;
    float  specularExponent;
};

/*
struct VertextShaderInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float2 textureUV : TEXCOORD0;
};
*/
struct VertextShaderInput
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 textureUV : TEXCOORD0;
};


struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 textureUV : TEXCOORD0;
    float3 vertexToEye : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 vertexToLight0 : TEXCOORD3;
    float3 vertexToLight1 : TEXCOORD4;
    float3 vertexToLight2 : TEXCOORD5;
    float3 vertexToLight3 : TEXCOORD6;
};

struct PixelShaderFlatInput
{
    float4 position : SV_POSITION;
    float2 textureUV : TEXCOORD0;
    float4 diffuseColor : TEXCOORD1;
};
