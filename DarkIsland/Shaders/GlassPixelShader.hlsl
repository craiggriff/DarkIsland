////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#include "../DefShader.h"

Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D positionTexture : register(t2);
Texture2D specularTexture : register(t3);

TextureCube environmentMapCube : register(t5);

SamplerState SampleType : register(s0);

//Texture2DArray spotshadowMapArray[MAX_SPOT_SHADOWS] : register(t100);

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
	int shadow_cube_index;
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

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
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

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;

float lightIntensity;
float lightIntensityDirectional;

float4 outputColor;
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

float point_atten;

float bias = 0.001f;

float4 viewPosition;

float specular_level;

float3 viewDir;

float spot_atten;

textureColor = colorTexture.Sample(SampleType, input.tex);

if (textureColor.a < 0.3f) discard;

outputColor = input.color;
// Initialize the specular color.
specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

viewDir = eye_position - input.positionW;
viewDir = normalize(viewDir);

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
}
if (false)
{
	if (lightIntensityDirectional > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		outputColor += ((diffuseColor * MaterialDiffuse)* lightIntensityDirectional);

		// Saturate the ambient and diffuse color.
		outputColor = saturate(outputColor);
	}
}

if (false)//(MaterialSpecular.w > 0.0f)
{
	if (lightIntensityDirectional > 0.0f)
	{
		reflection = normalize(2 * lightIntensityDirectional * n - lightDir);

		specular = ((MaterialSpecular*specularColor)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower*0.2f);
	}
}

//outputColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef LIGHT_SHADING_ENABLE

for (int i = 0; i < /*numSpotLights*/MAX_SPOT_LIGHTS; i++)
{
	if (numSpotLights <= i)
		continue;

	l = (spotLights[i].pos - input.positionW.xyz);

	float distance = length(l);

	if (distance < spotLights[i].radius)
	{
		l = normalize(l);

		if (dot(-spotLights[i].dir, l) > 0.0f)
		{
			//lightIntensity = saturate(dot(n, l));

			lightIntensity = 0.3f + (saturate(dot(n, l)));

			if (lightIntensity > 0.0f)
			{
				spot_atten = (1.0f - (distance / spotLights[i].radius)) * (1.0 / (1.0 + 0.1*distance + 0.01*distance*distance));

				viewPosition = mul(float4(input.positionW.xyz, 1.0f), spotLights[i].dirView);
				viewPosition = mul(viewPosition, spotLights[i].dirProj);

				projectTexCoord.x = viewPosition.x / viewPosition.w / 2.0f + 0.5f;
				projectTexCoord.y = -viewPosition.y / viewPosition.w / 2.0f + 0.5f;

				bool spot_lighted = false;

				if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
				{
					lightmapValue = lightmapArray[0].Sample(samPointSam, float3(projectTexCoord, 1.0f));
					outputColor = mix(outputColor, spotLights[i].diffuse * lightmapValue * lightIntensity * spot_atten * 0.3f);

					specular_level = 0.0f;
					if (MaterialSpecular.w > 0.0f)
					{
						float3 r = normalize(2 * dot(l, n) * n - l);
						float3 v = viewDir;

						float dotProduct = dot(r, v);
						// specular power is 20.0f but could be from light source
						specular_level = pow(dotProduct, (10.0f)) * MaterialSpecular.w * 1.0f;

						lightmapValue = lightmapValue * 1.2f;
						outputColor = mix(outputColor, (spotLights[i].diffuse * lightmapValue * specular_level) * spot_atten);// (max(point_atten, specular_level)));
					}
				}
			}
		}
	}
}
#endif

#ifdef LIGHT_SHADING_ENABLE
for (int i = 0; i < /*numPointLights*/MAX_POINT_LIGHTS; i++)
{
	if (numPointLights <= i)
		continue;

	l = (pointLights[i].pos - input.positionW.xyz);

	float distance = length(l);

	if (distance < pointLights[i].radius)
	{
		point_atten = (1.0f - (distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*distance + 0.01*distance*distance));

		l = normalize(l);
		//h = normalize(l + v);

		lightIntensity = 0.3f + (saturate(dot(n, l)));
		if (lightIntensity > 0.0f)
		{
			outputColor = mix(outputColor, (pointLights[i].diffuse * lightIntensity) * point_atten * 0.3f);// (max(point_atten, specular_level)));
			specular_level = 0.0f;
			if (MaterialSpecular.w > 0.0f)
			{
				float3 r = normalize(2 * dot(l, n) * n - l);
				float3 v = viewDir;

				float dotProduct = dot(r, v);
				// specular power is 20.0f but could be from light source
				specular_level = pow(dotProduct, (10.0f)) * MaterialSpecular.w * 1.2f;

				outputColor = mix(outputColor, (pointLights[i].diffuse * specular_level) * point_atten);// (max(point_atten, specular_level)));
			}
		}
	}
}
#endif

//outputColor = outputColor * ((input.position.z / input.position.w)*0.5f) + 0.5f;   //((outputColor*fade_atten)*textureColor)*1.0f;  // 2.0f is gamma
										  // textureColor.w;// *vertexCol.w;// textureColor.w; // retain alpha
										  //outputColor.w = input.color.w;

if (false) // sky reflection , cool
{
	float3 vec_tp = normalize(reflect(-normalize(input.viewDirection), normalize(-n)));

	vec_tp.x = -vec_tp.x;
	//outputColor = lerp(float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f), outputColor, 0.5f);

	outputColor = outputColor + float4(environmentMapCube.Sample(SampleType, vec_tp).rgb,1.0f) * (specular.w*0.3f);
	//float3 vec_tp = input.positionW - input.pointPosition;// float3(input.positionW.x - input.pointPosition.x, input.positionW.y - input.pointPosition.y, input.positionW.z - input.pointPosition.z);

	//float Dist = length(vec_tp);

	//vec_tp = normalize(vec_tp);

	//depthValue = pointshadowMapCube.Sample(SampleType, vec_tp).r;

	//outputColor.r = depthValue;
}
outputColor.a = 0.3f;// textureColor.a;// *input.color.a;
							// Add the specular component last to the output color.
return outputColor;// float4(1.0f, 0.0f, 1.0f, 1.0f);
}