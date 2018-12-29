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
	uint shadow_index;
	float spare;
	int lightmap;
	matrix dirView; // directional light view and projection
	matrix dirProj;
};

StructuredBuffer<PointLight> pointLights : register(t50);
StructuredBuffer<PointLight> spotLights : register(t51);

Texture2DArray lightmapArray[8] : register(t40);

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};
//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 pos : POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	float2 padding : PADDING;
	float3 position : POSITIONI;
	float4 color : COLORI;
	float2 paddingb : PADDINGI;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};
float manhattandist(float x, float y, float z, float x1, float y1, float z1, float distance)
{
	float dx = abs(x - x1);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - z1);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - y1);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	float3 l;
	float point_distance, point_atten;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.pos.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.pos, worldMatrix);

	output.position.x += input.position.x;
	output.position.y += input.position.y;
	output.position.z += input.position.z;

	output.color = input.color;// +(diffuseColor;

	output.color.r = diffuseColor.r * 0.6f;
	output.color.g = diffuseColor.g * 0.6f;
	output.color.b = diffuseColor.b * 0.6f;

	output.color.r += ambientColor.r;// *0.7f;
	output.color.g += ambientColor.g;// *0.7f;
	output.color.b += ambientColor.b;// *0.7f;
	output.color.a = 0.0f;
	output.color = saturate(output.color);

#ifdef LIGHT_SHADING_ENABLE
	float4 viewPosition;
	float2 projectTexCoord;
	float4 lightmapValue;
	float lightIntensity = 0.3f;
	float spot_atten;
	float spot_distance;

	for (int i = 0; i < /*numSpotLights*/MAX_SPOT_LIGHTS; i++)
	{
		if (numSpotLights <= i)
			continue;

		//continue;
		if (manhattandist(spotLights[i].pos.x, spotLights[i].pos.y, spotLights[i].pos.z,
			output.position.x, output.position.y, output.position.z, spotLights[i].radius) < 0.0f)
			continue;

		l = (spotLights[i].pos - output.position.xyz);
		spot_distance = length(l);
		if (dot(-spotLights[i].dir, normalize(l)) > 0.0f)
		{
			spot_atten = (1.0f - (spot_distance / spotLights[i].radius)) * (1.0 / (1.0 + 0.1*spot_distance + 0.01*spot_distance*spot_distance));

			viewPosition = mul(float4(output.position.xyz, 1.0f), spotLights[i].dirView);
			viewPosition = mul(viewPosition, spotLights[i].dirProj);

			projectTexCoord.x = viewPosition.x / viewPosition.w / 2.0f + 0.5f;
			projectTexCoord.y = -viewPosition.y / viewPosition.w / 2.0f + 0.5f;

			if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
			{
				uint2 pos_xy = { projectTexCoord.x, projectTexCoord.y };
				//lightmapValue = 1.0f;// lightmapArray[0][pos_xy];

				lightmapValue = lightmapArray[0].SampleLevel(samPointSam, float3(projectTexCoord, 1.0f), 0)*2.0f;

				output.color = max(output.color, spotLights[i].diffuse * lightmapValue * lightIntensity * spot_atten);
			}
		}
		/*
		//outputColor = float4(1.0f, 1.0f, 0.0f, 0.0f);
		l = (spotLights[i].pos - output.position.xyz);
		point_distance = length(l);

		point_atten = (1.0f - (point_distance / spotLights[i].radius)) * (1.0 / (1.0 + 0.1*point_distance + 0.05*point_distance*point_distance));

		float4 col = spotLights[i].diffuse*point_atten;//  float4((((pointLights[i].diffuse*0.75f) + pointLights[i].diffuse*0.5f)*0.5f)*point_atten, 0.1f);

		output.color = max(output.color, col);
		*/
	}

	for (int i = 0; i < /*numPointLights*/MAX_POINT_LIGHTS; i++)
	{
		if (numPointLights <= i)
			continue;

		//continue;
		if (manhattandist(pointLights[i].pos.x, pointLights[i].pos.y, pointLights[i].pos.z,
			output.position.x, output.position.y, output.position.z, pointLights[i].radius) < 0.0f)
			continue;

		//outputColor = float4(1.0f, 1.0f, 0.0f, 0.0f);
		l = (pointLights[i].pos - output.position.xyz);
		point_distance = length(l);

		point_atten = (1.0f - (point_distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*point_distance + 0.05*point_distance*point_distance));

		float4 col = pointLights[i].diffuse*point_atten;//  float4((((pointLights[i].diffuse*0.75f) + pointLights[i].diffuse*0.5f)*0.5f)*point_atten, 0.1f);

		output.color = max(output.color, col);
	}
#endif

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//output.color.w = 1.0f;// input.color.w;

	output.color.w = 0.0f;// min(((output.color.x + output.color.y + output.color.z)*((input.color.w*0.2f) + 0.1f)), input.color.w);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	//input.color.w = 1.0f;
	//output.color = input.color;// *ambientColor*(diffuseColor*0.5);
	//output.color.w = 1.0f;

	// Store the particle color for the pixel shader.

	return output;
}