// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Westley M. Martinez
//-----------------------------------------------------------------------------
// Ghosting Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture   = <Diffuse>;
	SRGBTexture = TRUE;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

texture LastPass;

sampler PreviousSampler = sampler_state
{
	Texture   = <LastPass>;
	SRGBTexture = TRUE;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 PrevCoord : TEXCOORD1;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 PrevCoord : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Phosphor Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;

uniform bool Passthrough;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.PrevCoord = Output.TexCoord;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Phosphor Pixel Shader
//-----------------------------------------------------------------------------

uniform float DeltaTime = 0.0f;
uniform float3 LCDRise = { 0.0f, 0.0f, 0.0f };
uniform float3 LCDFall = { 0.0f, 0.0f, 0.0f };
static const float TAU_FACTOR = 0.159154943;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 CurrPix = tex2D(DiffuseSampler, Input.TexCoord);
	float3 PrevPix = tex2D(PreviousSampler, Input.PrevCoord).rgb;
	float3 tau = { 0.0f, 0.0f, 0.0f };
	float r = PrevPix.r;
	float g = PrevPix.g;
	float b = PrevPix.b;

	if (CurrPix.r > r) {
		tau.r = LCDRise.r * TAU_FACTOR;
		r = tau.r == 0 ? CurrPix.r : (CurrPix.r - r) *
		    (1 - exp(-DeltaTime / tau.r)) + r;
	}
	else {
		tau.r = LCDFall.r * TAU_FACTOR;
		r = tau.r == 0 ? CurrPix.r : (r - CurrPix.r) *
		    exp(-DeltaTime / tau.r) + CurrPix.r;
	}
	if (CurrPix.g > g) {
		tau.g = LCDRise.g * TAU_FACTOR;
		g = tau.g == 0 ? CurrPix.g : (CurrPix.g - g) *
		    (1 - exp(-DeltaTime / tau.g)) + g;
	}
	else {
		tau.g = LCDFall.g * TAU_FACTOR;
		g = tau.g == 0 ? CurrPix.g : (g - CurrPix.g) *
		    exp(-DeltaTime / tau.g) + CurrPix.g;
	}
	if (CurrPix.b > b) {
		tau.b = LCDRise.b * TAU_FACTOR;
		b = tau.b == 0 ? CurrPix.b : (CurrPix.b - b) *
		    (1 - exp(-DeltaTime / tau.b)) + b;
	}
	else {
		tau.b = LCDFall.b * TAU_FACTOR;
		b = tau.b == 0 ? CurrPix.b : (b - CurrPix.b) *
		    exp(-DeltaTime / tau.b) + CurrPix.b;
	}
	return Passthrough ?  CurrPix : float4(r, g, b, CurrPix.a);
}

//-----------------------------------------------------------------------------
// Phosphor Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		SRGBWriteEnable = TRUE;
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
