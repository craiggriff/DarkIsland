////////////////////////////////////////////////////////////////////////////////
// Filename: particle.ps
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 tex_sam = shaderTexture.Sample(SampleType, input.tex);// *input.color;
	float4 outcol;
	//discard;

	if (tex_sam.w < 0.02f)// || (outcol.r < 0.1f && outcol.g < 0.1f && outcol.b < 0.1f))
	{
		discard;
	}

	outcol = tex_sam * input.color;
	//outcol.w = (outcol.x + outcol.y + outcol.z)*0.33;

	outcol.w = tex_sam.w;
	//tex_sam.a *= 0.05f;// tex_sam.r;
	//tex_sam.x
	return outcol;// float4(1.0f, 1.0f, 1.0f, 0.1f);//  tex_sam;// float4(input.position.z, input.position.z, input.position.z, 1.0f);  //outcol;// float4(1.0f, 1.0f, 1.0f, 1.0f);// outcol;
}