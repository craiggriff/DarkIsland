////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#include "../DefShader.h"

Texture2D shaderTexture : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D normalTexture : register(t3);
//Texture2D depthTexture : register(t4);

SamplerState SampleType : register(s0);

TextureCube environmentMapCube : register(t5);

//TextureCubeArray pointshadowMapCubeArray[MAX_POINT_SHADOWS] : register(t60);

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
	uint shadow_index;
	float spare;
	int lightmap;
	matrix dirView; // directional light view and projection
	matrix dirProj;
};

StructuredBuffer<PointLight> pointLights : register(t50);
StructuredBuffer<PointLight> spotLights : register(t51);

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

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

	float spare1a;
	float spare2a;
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

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

float4 mix(float4 col1, float4 col2)
{
	float4 add = (col1 + col2)*0.04f;
	float4 m = max(col1, col2);

	return m;// add + m;
}

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

float VectorToDepth(float3 Vec)
{
	//return 0.5f;
	float3 AbsVec = abs(Vec);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	// Replace f and n with the far and near plane values you used when
	//   you drew your cube map.
	const float f = POINT_LIGHT_RADIUS;
	const float n = 0.1f;

	float NormZComp = (f + n) / (f - n) - (2 * f*n) / (f - n) / LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;

	float lightIntensity;
	float lightIntensityDirectional;

	float4 outcolor;
	float3 reflection;
	float4 specular;
	float4 specular_point;

	float3 lightDir = -lightDirection;

	float3 n = input.normal;
	float3 v = input.viewDirection;
	float3 l = float3(0.0f, 0.0f, 0.0f);
	float3 h = float3(0.0f, 0.0f, 0.0f);

	float atten = 0.0f;
	float nDotL = 0.0f;
	float nDotH = 0.0f;
	float power = 0.0f;
	float4 normalMap;

	float2 projectTexCoord;
	float4 projectionColor;
	float lightDepthValue;
	float depthValue;
	float4 lightmapValue;

	float bias = 0.001f;

	float point_distance;
	float point_atten;
	float depthValue2;
	float3 point_light_dir;

	textureColor = shaderTexture.Sample(SampleType, input.tex);

	if (textureColor.a < 0.3f) discard;

	outcolor = input.color;
	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Calculate the amount of light on this pixel.
	lightIntensityDirectional = saturate(dot(n, lightDir));

	if (false)//(normal_height>0.0f)//normalLvl == 1.0f)
	{
		//discard;
		if (input.cam_dist < 25.0f)
		{
			normalMap = normalTexture.Sample(SampleType, input.tex);

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

		if (lightIntensityDirectional > 0.0f)
		{
			// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
			outcolor += ((diffuseColor * MaterialDiffuse)* lightIntensityDirectional);

			// Saturate the ambient and diffuse color.
			outcolor = saturate(outcolor);
		}
	}

	n = input.normal;

	if (MaterialSpecular.w > 0.0f)
	{
		if (lightIntensityDirectional > 0.0f)
		{
			reflection = normalize(2 * lightIntensityDirectional * n - lightDir);

			specular = ((MaterialSpecular*specularColor)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower*0.2f);
		}
	}

	if (true)
	for (int i = 0; i < numPointLights; i++)
	{
		if (manhattandist(pointLights[i].pos.x, pointLights[i].pos.y, pointLights[i].pos.z,
			input.positionW.x, input.positionW.y, input.positionW.z, pointLights[i].radius) < 0.0f)
			continue;

		l = (pointLights[i].pos - pointLights[i].pos);

		point_distance = length(l);

		if (point_distance > pointLights[i].radius)
			continue;

		point_atten = (1.0f - (point_distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*point_distance + 0.01*point_distance*point_distance));

		if (point_atten < 0.1f)
			continue;

		l = normalize(l);

		lightIntensity = saturate(dot(n, l));
		bool bAddLight = true;
		const int indx = pointLights[i].shadow_index;

		depthValue2 = 1.0f;
		/*
		if (indx > -1)
		{
			point_light_dir = normalize(l);

			point_light_dir.x = -point_light_dir.x;

			[unroll(MAX_POINT_SHADOWS)]for (int j = 0; j < MAX_POINT_SHADOWS - 1; j++)
			{
				if (indx == j)
				{
					depthValue2 = pointshadowMapCubeArray[j].Sample(samPointSam, float4(-point_light_dir, 1.0f)).r; break;

					//break;
				}
			}

			//if(bFound==true)
			if (VectorToDepth(input.positionW - pointLights[i].pos) - bias > depthValue2)
				continue;

			//depthValue2 = pointshadowMapCubeArray[2].Sample(samPointSam, float4(-point_light_dir, 1.0f)).r; break;
		}
		*/
		float4 ambient = (pointLights[i].diffuse*0.3f);
		outcolor = mix(outcolor, (max((pointLights[i].diffuse * lightIntensity), ambient)) * LIGHT_POINT_MULTIPLIER * point_atten);
		//outcolor = float4(1.0f,1.0f,1.0f,1.0f);

		if (lightIntensity <= 0.0f || pointLights[i].specular_power <= 0.0f)
			continue;

		//reflection = normalize(2 * lightIntensity * n - l);

		//specular += ((float4(speculars.xyz, 1.0f) *pointLights[i].specular)*2.0f)*pow(saturate(dot(reflection, viewDir)), pointLights[i].specular_power);
	}

	//float s = input.cam_dist;
	//s = saturate(100.0f - s);

	//color = lerp(color, float4(0.5f,0.5f,0.5f,0.0f), s/10.0f);

	// Multiply the texture pixel and the input color to get the textured result.
	outcolor = (outcolor*textureColor)*2.0f;  // 2.0f is gamma
											  // textureColor.w;// *vertexCol.w;// textureColor.w; // retain alpha
											  //outcolor.w = input.color.w;

	if (true)//specular.w >0.0f)
	{
		outcolor = saturate(outcolor + specular);
	}

	if (false) // sky reflection , cool
	{
		float3 vec_tp = normalize(reflect(-normalize(input.viewDirection), normalize(-n)));

		vec_tp.x = -vec_tp.x;
		//outcolor = lerp(float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f), outcolor, 0.5f);

		outcolor = outcolor + float4(environmentMapCube.Sample(SampleType, vec_tp).rgb,1.0f) * (specular.w*0.3f);
		//float3 vec_tp = input.positionW - input.pointPosition;// float3(input.positionW.x - input.pointPosition.x, input.positionW.y - input.pointPosition.y, input.positionW.z - input.pointPosition.z);

		//float Dist = length(vec_tp);

		//vec_tp = normalize(vec_tp);

		//depthValue = pointshadowMapCube.Sample(SampleType, vec_tp).r;

		//outcolor.r = depthValue;
	}
	outcolor.a = textureColor.a;// *input.color.a;
	// Add the specular component last to the output color.
	return outcolor;// float4(1.0f, 0.0f, 1.0f, 1.0f);
}