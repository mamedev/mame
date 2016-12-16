// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Phosphor Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture   = <Diffuse>;
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
uniform int Mode = 0;
uniform float3 Tau = { 0.0f, 0.0f, 0.0f };
uniform float3 Beta = { 0.0f, 0.0f, 0.0f };
uniform float3 Gamma = { 0.0f, 0.0f, 0.0f };

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 CurrPix = tex2D(DiffuseSampler, Input.TexCoord);
	float3 PrevPix = tex2D(PreviousSampler, Input.PrevCoord).rgb;
	float r = PrevPix.r;
	float g = PrevPix.g;
	float b = PrevPix.b;

	if (Mode == 0) {
		r = 0;
		g = 0;
		b = 0;
	}
	else if (Mode == 1) {
		r *= exp(-DeltaTime / Tau.r);
		g *= exp(-DeltaTime / Tau.g);
		b *= exp(-DeltaTime / Tau.b);
	}
	else {
		if (r != 0.0f)
			r = pow(Gamma.r * DeltaTime + pow(1 / r, 1 / Beta.r), -Beta.r);
		if (g != 0.0f)
			g = pow(Gamma.g * DeltaTime + pow(1 / g, 1 / Beta.g), -Beta.g);
		if (b != 0.0f)
			b = pow(Gamma.b * DeltaTime + pow(1 / b, 1 / Beta.b), -Beta.b);
	}
	float RedMax = max(CurrPix.r, r);
	float GreenMax = max(CurrPix.g, g);
	float BlueMax = max(CurrPix.b, b);

	return Passthrough ?
	       CurrPix : float4(RedMax, GreenMax, BlueMax, CurrPix.a);
}

//-----------------------------------------------------------------------------
// Phosphor Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
