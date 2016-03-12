// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture = <Diffuse>;
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
// Defocus Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	float2 TexCoord = Input.TexCoord;
	TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.TexCoord = TexCoord;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Defocus Pixel Shader
//-----------------------------------------------------------------------------

uniform float2 Defocus = float2(0.0f, 0.0f);

static const float2 Coord1Offset = float2( 0.75f,  0.50f);
static const float2 Coord2Offset = float2( 0.25f,  1.00f);
static const float2 Coord3Offset = float2(-0.50f,  0.75f);
static const float2 Coord4Offset = float2(-1.00f,  0.25f);
static const float2 Coord5Offset = float2(-0.75f, -0.50f);
static const float2 Coord6Offset = float2(-0.25f, -1.00f);
static const float2 Coord7Offset = float2( 0.50f, -0.75f);
static const float2 Coord8Offset = float2( 1.00f, -0.25f);

float4 ps_main(PS_INPUT Input) : COLOR
{
	// imaginary texel dimensions independed from screen dimension, but ratio
	float2 TexelDims = (1.0f / 1024);

	float2 DefocusTexelDims = Defocus * TexelDims;

	float4 d = tex2D(DiffuseSampler, Input.TexCoord);
	float3 d1 = tex2D(DiffuseSampler, Input.TexCoord + Coord1Offset * DefocusTexelDims).rgb;
	float3 d2 = tex2D(DiffuseSampler, Input.TexCoord + Coord2Offset * DefocusTexelDims).rgb;
	float3 d3 = tex2D(DiffuseSampler, Input.TexCoord + Coord3Offset * DefocusTexelDims).rgb;
	float3 d4 = tex2D(DiffuseSampler, Input.TexCoord + Coord4Offset * DefocusTexelDims).rgb;
	float3 d5 = tex2D(DiffuseSampler, Input.TexCoord + Coord5Offset * DefocusTexelDims).rgb;
	float3 d6 = tex2D(DiffuseSampler, Input.TexCoord + Coord6Offset * DefocusTexelDims).rgb;
	float3 d7 = tex2D(DiffuseSampler, Input.TexCoord + Coord7Offset * DefocusTexelDims).rgb;
	float3 d8 = tex2D(DiffuseSampler, Input.TexCoord + Coord8Offset * DefocusTexelDims).rgb;

	float3 blurred = (d.rgb + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8) / 9.0f;
	blurred = lerp(d.rgb, blurred, 1.0f);

	return float4(blurred, d.a);
}

//-----------------------------------------------------------------------------
// Defocus Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}
