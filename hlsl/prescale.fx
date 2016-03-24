// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,Themaister,ImJezze
//-----------------------------------------------------------------------------
// Pre-scale Effect
//
// Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.
//
// https://github.com/libretro/common-shaders/blob/master/retro/shaders/sharp-bilinear.cg
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
	float2 TexCoord : TEXCOORD0;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 Unused : TEXCOORD1;
};

struct PS_INPUT
{
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Pre-scale Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;
uniform float2 SourceDims;

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

	return Output;
}

//-----------------------------------------------------------------------------
// Pre-scale Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 Scale = TargetDims / SourceDims;

	float2 TexelDims = Input.TexCoord * SourceDims;
	float2 i = floor(TexelDims);
	float2 s = frac(TexelDims);

	// Figure out where in the texel to sample to get the correct pre-scaled bilinear.
	float2 CenterDistance = s - 0.5f;
	float2 RegionRange = 0.5f - 0.5f / Scale;
	float2 f = (CenterDistance - clamp(CenterDistance, -RegionRange, RegionRange)) * Scale + 0.5f;

	float2 TexCoord = (i + f) / SourceDims;

	return tex2D(DiffuseSampler, TexCoord);
}

//-----------------------------------------------------------------------------
// Pre-scale Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
