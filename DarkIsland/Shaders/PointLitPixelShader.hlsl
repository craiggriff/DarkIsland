////////////////////////////////////////////////////////////////////////////////
// Filename: particle.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
Texture2DArray shaderTexture[4] : register(t0);
SamplerState SampleType : register(s0);

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
	uint tex_num : TEXNUM0;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	// think this is only used for rain
	float4 tex_sam;// = shaderTexture[0].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	float4 outcol;
	//discard;

	if (input.tex_num == 0)
	{
		tex_sam = shaderTexture[0].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 1)
	{
		tex_sam = shaderTexture[1].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 2)
	{
		tex_sam = shaderTexture[2].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}
	if (input.tex_num == 3)
	{
		tex_sam = shaderTexture[3].Sample(SampleType, float3(input.tex, 1.0f));// *input.color;
	}

	if (tex_sam.w < 0.02f)// || (outcol.r < 0.1f && outcol.g < 0.1f && outcol.b < 0.1f))
	{
		discard;
	}

	outcol = tex_sam * input.color;

	//outcol.w = tex_sam.w;
	outcol.w = tex_sam.w;// (outcol.x + outcol.y + outcol.z)*0.33;

	//outcol = float4(1.0f, 1.0f, 1.0f, 1.0f);
	//input.color.w = 0.1f;
	//tex_sam.a *= 0.05f;// tex_sam.r;
	//tex_sam.x
	return outcol;// float4(1.0f, 1.0f, 1.0f, 0.1f); //outcol;// outcol;// float4(1.0f, 1.0f, 1.0f, 0.1f);//  tex_sam;// float4(input.position.z, input.position.z, input.position.z, 1.0f);  //outcol;// float4(1.0f, 1.0f, 1.0f, 1.0f);// outcol;
}