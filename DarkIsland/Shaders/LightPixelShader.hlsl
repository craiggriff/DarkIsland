////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////
#include "../DefShader.h"

// As inputs it takes the two render textures that contain the color information and the normals from the deferred shader.

/////////////
// GLOBALS //
/////////////
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D positionTexture : register(t2);
Texture2D specularTexture : register(t3);

TextureCube environmentMapCube : register(t5);

Texture2D testTexture : register(t6);

SamplerState SampleType : register(s0);

SamplerState BilenearSampleType : register(s3);

//TextureCubeArray pointshadowMapCubeArray[MAX_POINT_SHADOWS] : register(t60);

//Texture2DArray spotshadowMapArray[MAX_SPOT_SHADOWS] : register(t100);

Texture2DArray lightmapArray[8] : register(t40);

// We require a point sampler since we will be sampling out exact per - pixel data values from the render textures.

///////////////////
// SAMPLE STATES //
///////////////////
//SamplerState SampleTypePoint : register(s0);

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
	//ComparisonFunc = LESS;
	//MaxAnisotropy = 4;
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
	int numPointLights;
	int numSpotLights;
	float lightning;
	float3 lightning_dir;
	float3 pos;
	float screen_width;
	float screen_height;
	float texel_width;
	float texel_height;
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

float4 mix(float4 col1, float4 col2)
{
	float4 add = (col1 + col2)*0.04f;
	float4 m = max(col1, col2);
	//float4 m = col1 * col2;

	return m;// add + m;
}

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 pad : PAD0;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 colors;
	float4 normals;
	float4 positions;
	float4 speculars;
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float specular_level = 0.0f;

	float3 lightDir;
	float lightIntensity;
	float4 outputColor;
	float spot_atten;
	float3 l,n;
	float3 reflection;
	float bias = 0.001f;

	float2 projectTexCoord;
	float4 projectionColor;
	float lightDepthValue;
	float depthValue;
	float depthValue2;
	float4 lightmapValue;

	float4 ambDiffColor;

	float3 viewDir;

	float LightDepth;
	float Dist;

	float3 point_light_dir;

	float point_distance;
	float point_atten;
	float cam_atten;

	float pixel_distance;
	float spot_distance;

	int i;
	int k;

	float average_col = colors.r + colors.g + colors.b;
	/*
	average_col *= 0.333333f;

	colors.r = average_col;
	colors.g = average_col;
	colors.b = average_col;
	*/

	//if (colors.w == 0)
	//	discard;
	// Sample the normals from the normal render texture using the point sampler at this texture coordinate location.

	colors = colorTexture.Sample(samPointSam, input.tex);

	normals = normalTexture.Sample(samPointSam, input.tex);

	positions = positionTexture.Sample(samPointSam, input.tex);

	speculars = specularTexture.Sample(samPointSam, input.tex);

	n = normals.xyz;

	//We can then perform our directional lighting equation using this sampled information.

	viewDir = eye_position - positions.xyz;

	pixel_distance = length(viewDir);

	viewDir = normalize(viewDir);

	outputColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 viewPosition;
#ifdef LIGHT_SHADING_ENABLE
	for (k = 0; k < MAX_SPOT_LIGHTS/*numSpotLights*/; k++)
	{
		if (numSpotLights <= k)
			continue;

		l = (spotLights[k].pos - positions.xyz);

		spot_distance = length(l);

		if (spot_distance < spotLights[k].radius)
		{
			l = normalize(l);

			if (dot(-spotLights[k].dir, l) > 0.0f)
			{
				if (normals.w > 0.0f) // ignore normals
				{
					lightIntensity = normals.w;
				}
				else
				{
					lightIntensity = saturate(dot(n, l));
				}

				spot_atten = (1.0f - (spot_distance / spotLights[k].radius)) * (1.0 / (1.0 + 0.1*spot_distance + 0.01*spot_distance*spot_distance));

				viewPosition = mul(float4(positions.x, positions.y, positions.z, 1.0f), spotLights[k].dirView);
				viewPosition = mul(viewPosition, spotLights[k].dirProj);

				projectTexCoord.x = viewPosition.x / viewPosition.w / 2.0f + 0.5f;
				projectTexCoord.y = -viewPosition.y / viewPosition.w / 2.0f + 0.5f;

				bool spot_lighted = true;

				if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
				{
					lightmapValue = lightmapArray[0].Sample(SampleType, float3(projectTexCoord, 1.0f));
					outputColor = mix(outputColor, spotLights[k].diffuse * lightmapValue * lightIntensity * spot_atten);

					specular_level = 0.0f;
					if (speculars.w > 0.0f)
					{
						float3 r = normalize(2 * dot(l, n) * n - l);
						float3 v = viewDir;

						float dotProduct = dot(r, v);
						// specular power is 20.0f but could be from light source
						specular_level = pow(dotProduct, (10.0f)) * speculars.w * 1.0f;

						lightmapValue = lightmapValue * 1.2f;
						outputColor = mix(outputColor, (spotLights[k].diffuse * lightmapValue * specular_level) * spot_atten);// (max(point_atten, specular_level)));
					}
				}
			}
		}
	}
#endif

#ifdef LIGHT_SHADING_ENABLE
	for (i = 0; i < MAX_POINT_LIGHTS /*numPointLights*/; i++)
	{
		if (numPointLights <= i)
			continue;

		l = (pointLights[i].pos - positions.xyz);

		point_distance = length(l);

		if (point_distance > pointLights[i].radius)
			continue;

		point_atten = (1.0f - (point_distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*point_distance + 0.01*point_distance*point_distance));

		if (point_atten < 0.01f)
			continue;

		l = normalize(l);

		if (normals.w > 0.0f) // ignore normals
		{
			lightIntensity = normals.w;//
		}
		else
		{
			lightIntensity = saturate(dot(n, l));
		}
		bool bAddLight = true;
		const int indx = pointLights[i].shadow_index;

		depthValue2 = 1.0f;

		if (lightIntensity > 0.0f)//|| speculars.w == 0.0f)// <= 0.0f || normals.w > 0.0f)
		{
			outputColor = mix(outputColor, (pointLights[i].diffuse * lightIntensity) * point_atten);// (max(point_atten, specular_level)));
			specular_level = 0.0f;
			if (speculars.w > 0.0f)
			{
				float3 r = normalize(2 * dot(l, n) * n - l);
				float3 v = viewDir;

				float dotProduct = dot(r, v);
				// specular power is 20.0f but could be from light source
				specular_level = pow(dotProduct, (10.0f)) * speculars.w * 1.2f;

				outputColor = mix(outputColor, (pointLights[i].diffuse * specular_level) * point_atten);// (max(point_atten, specular_level)));
			}
		}
	}
#endif

	lightDir = -lightDirection;

	lightDir = normalize(lightDir);

	// Calculate the amount of light on this pixel.
	if (normals.w > 0.0f) // ignore normals
	{
		lightIntensity = normals.w;
	}
	else
	{
		lightIntensity = saturate(dot(normals.xyz, lightDir));
	}
	if (lightIntensity > 0.0f)
	{
		outputColor = mix(diffuseColor* lightIntensity, outputColor);
		if (speculars.w > 0.0f)
		{
			float3 r = normalize(2 * dot(lightDir, normals.xyz) * normals.xyz - lightDir);
			float3 v = viewDir;

			float dotProduct = dot(r, v);
			// specular power is 20.0f but could be from light source
			specular_level = pow(dotProduct, (10.0f)) * speculars.w * 1.0f;

			outputColor = mix(outputColor, (diffuseColor * specular_level));// (max(point_atten, specular_level)));

			// Add some sky becuase specular stuff is reflective
			float3 vec_tp = normalize(reflect(-normalize(viewDir), normalize(n)));

			vec_tp.x = -vec_tp.x;

			outputColor += float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f)*0.5f;
		}
	}
	/*
	if (true)
	{
		float3 vec_tp = normalize(reflect(-normalize(viewDir), normalize(n)));

		vec_tp.x = -vec_tp.x;

		outputColor += float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f)*0.5f;
	}
	*/
	float4 res = (mix(outputColor, ambientColor) * (colors * 1.0f));// +((outputColor + ambDiffColor + specular)*0.06f));

	return res + (res * positions.w);// +(specular*10.0f);
}