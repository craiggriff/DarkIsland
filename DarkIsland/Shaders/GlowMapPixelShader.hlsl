////////////////////////////////////////////////////////////////////////////////
// Filename: glowmap.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);
Texture2D glowMapTexture : register(t1);
Texture2D glowMapOriginalTexture : register(t2);

Texture2D waterPosTexture : register(t3);
Texture2D waterNormTexture : register(t4);

TextureCube environmentMapCube : register(t5);

Texture2D groundPosTexture : register(t6);

Texture2D waterNormScreenTexture : register(t7);

SamplerState SampleType;

#define REFRACTION_VACUUM 1.0000;
#define REFRACTION_AIR 1.0003;
#define REFRACTION_ICE 1.3100;
#define REFRACTION_WATER 1.3330;
#define REFRACTION_GASOLINE 1.3980;
#define REFRACTION_GLASS 1.5500;
#define REFRACTION_SAPPHIRE 1.7700;
#define REFRACTION_DIAMOND 2.4190;

float3 Refract(float3 incidentVec, float3 normal, float eta)
{
	float3 output;

	float N_dot_I = dot(normal, incidentVec);
	float k = 1.f - eta * eta * (1.f - N_dot_I * N_dot_I);
	if (k < 0.f)
		output = float3(0.f, 0.f, 0.f);
	else
		output = eta * incidentVec - (eta * N_dot_I + sqrt(k)) * normal;

	return output;
}

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

cbuffer ScreenSizeBuffer : register(b10)
{
	float width_size;
	float height_size;
	float2 padding;
};

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 mix(float4 col1, float4 col2)
{
	float4 add = col2 * 0.5f;
	float4 m = max(col1, col2);

	return add + m;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 waterPos;
	float3 waterNorm;
	float3 waterScrNorm;
	float4 waterColor;

	float4 groundPos;

	float4 mixColor;
	float4 outColor;

	float4 textureColor;

	float3 viewDirection;

	float4 glowMap;
	float4 glowMapOriginal;

	textureColor = shaderTexture.Sample(SampleType, input.tex);
	glowMap = glowMapTexture.Sample(SampleType, input.tex);
	glowMapOriginal = glowMapOriginalTexture.Sample(SampleType, input.tex);

	waterPos = waterPosTexture.Sample(SampleType, input.tex);

	waterNorm = waterNormTexture.Sample(SampleType, input.tex).xyz;
	waterScrNorm = waterNormScreenTexture.Sample(SampleType, input.tex).xyz;

	groundPos = groundPosTexture.Sample(SampleType, input.tex);

	viewDirection = eye_position - waterPos.xyz;

	if (waterNorm.y > 0.0)
	{
		float3 vec_tp = normalize(reflect(-normalize(viewDirection), normalize(waterNorm.xyz)));

		vec_tp.x = -vec_tp.x;
		waterColor = float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f);

		float3 view_dir = groundPos - waterPos;
		float view_depth = length(view_dir);
		float view_dist = length(waterPos);
		view_dir = normalize(view_dir);
		if (false)
		{
			float3 vdir = float3(0.0f, 0.0f, 1.0f);

			float3 rdir = Refract(vdir, waterScrNorm, 1.5f);

			float2 tex = input.tex;
			tex.x += (view_depth*1.15f)*rdir.x*10.1f;// *-texel_width*3000.0f;// 0.1f;// (view_depth*texel_width);
			tex.y += (view_depth*1.15f)*rdir.z*10.1f;// *-texel_height*3000.0f;

															//waterPos = waterPosTexture.Sample(SampleType, input.tex);

			textureColor = shaderTexture.Sample(SampleType, tex);// *view_depth;// float4(view_depth, view_depth, view_depth, view_depth);// shaderTexture.Sample(SampleType, tex) * view_depth;
		}

		if (false)//(view_depth > 0.0f)
		{
			float2 tex = input.tex;

			tex.x += ((view_depth + 1.0f)*2.5f)*waterScrNorm.x*15.3f*(1.0f / (view_dist*(view_dist*1.7f)));// *-texel_width*3000.0f;// 0.1f;// (view_depth*texel_width);
			tex.y += ((view_depth + 1.0f)*2.5f)*waterScrNorm.z*15.3f*(1.0f / (view_dist*(view_dist*1.7f)));// *-texel_height*3000.0f;

			//waterPos = waterPosTexture.Sample(SampleType, input.tex);

			textureColor = shaderTexture.Sample(SampleType, tex); // +(view_depth*0.1f*float4(1.0f, 1.0f, 1.0f, 1.0f));// *view_depth;// float4(view_depth, view_depth, view_depth, view_depth);// shaderTexture.Sample(SampleType, tex) * view_depth;
		}
	}
	else
		waterColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	outColor = textureColor + glowMapOriginal + glowMap + waterColor;// mix(glowMapOriginal, glowMap);// textureColor;// glowMapOriginal;// textureColor;// +glowMapOriginal;// glowMap;// textureColor + glowMapOriginal;// +glowMap;  //mix(textureColor, glowMap);

	return outColor;// float4(1.0f, 1.0f, 1.0f, 1.0f);// ;
}