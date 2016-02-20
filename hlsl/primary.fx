// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Primary Effect
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

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
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
};

//-----------------------------------------------------------------------------
// Primary Vertex Shader
//-----------------------------------------------------------------------------

static const float Epsilon = 1.0e-7f;

uniform float2 ScreenDims;
uniform float2 TargetDims;

uniform bool PostPass;
uniform bool VectorScreen;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	float2 targetDims = TargetDims + Epsilon; // bug: with exact target dimensions the font disappears

	if (PostPass)
	{
		Output.TexCoord = Input.Position.xy / ScreenDims;
		Output.TexCoord += 0.5f / targetDims; // half texel offset correction (DX9)
	}
	else
	{
		Output.TexCoord = Input.TexCoord;
		// Output.TexCoord += 0.5f / targetDims; // half texel offset correction (DX9)
	}

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Primary Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);
	BaseTexel *= PostPass && VectorScreen
		? Input.Color + float4(1.0f, 1.0f, 1.0f, 0.0f)
		: Input.Color;

	return BaseTexel;
}

//-----------------------------------------------------------------------------
// Primary Technique
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
