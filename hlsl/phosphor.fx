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
	VS_OUTPUT Output = (VS_OUTPUT)0.0;

	Output.Position = float4(Input.Position.xyz, 1.0);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0 - Output.Position.y; // flip y
	Output.Position.xy -= 0.5; // center
	Output.Position.xy *= 2.0; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5 / TargetDims; // half texel offset correction (DX9)

	Output.PrevCoord = Output.TexCoord;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Phosphor Pixel Shader
//-----------------------------------------------------------------------------

uniform float DeltaTime = 0.0;
uniform float3 Phosphor = float3(0.0, 0.0, 0.0);

static const float F = 30.0;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 CurrY = tex2D(DiffuseSampler, Input.TexCoord);
	float3 PrevY = tex2D(PreviousSampler, Input.PrevCoord).rgb;

	PrevY[0] *= Phosphor[0] == 0.0 ? 0.0 : pow(Phosphor[0], F * DeltaTime);
	PrevY[1] *= Phosphor[1] == 0.0 ? 0.0 : pow(Phosphor[1], F * DeltaTime);
	PrevY[2] *= Phosphor[2] == 0.0 ? 0.0 : pow(Phosphor[2], F * DeltaTime);
	float a = max(PrevY[0], CurrY[0]);
	float b = max(PrevY[1], CurrY[1]);
	float c = max(PrevY[2], CurrY[2]);
	return Passthrough ? CurrY : float4(a, b, c, CurrY.a);
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
