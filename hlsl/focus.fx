// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Effect File Variables
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
// Functions
//-----------------------------------------------------------------------------

bool xor(bool a, bool b)
{
	return (a || b) && !(a && b);
}

//-----------------------------------------------------------------------------
// Simple Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;
uniform float2 SourceRect;
uniform float2 QuadDims;

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
// Simple Pixel Shader
//-----------------------------------------------------------------------------

float2 Coord1Offset = float2( 0.75f,  0.50f);
float2 Coord2Offset = float2( 0.25f,  1.00f);
float2 Coord3Offset = float2(-0.50f,  0.75f);
float2 Coord4Offset = float2(-1.00f,  0.25f);
float2 Coord5Offset = float2(-0.75f, -0.50f);
float2 Coord6Offset = float2(-0.25f, -1.00f);
float2 Coord7Offset = float2( 0.50f, -0.75f);
float2 Coord8Offset = float2( 1.00f, -0.25f);

uniform float2 Defocus = float2(0.0f, 0.0f);

uniform bool OrientationSwapXY = false; // false landscape, true portrait for default screen orientation
uniform bool RotationSwapXY = false; // swapped default screen orientation due to screen rotation

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 QuadRatio = 
		float2(1.0f, xor(OrientationSwapXY, RotationSwapXY) 
			? QuadDims.y / QuadDims.x 
			: QuadDims.x / QuadDims.y);

	// imaginary texel dimensions independed from quad dimensions, but dependend on quad ratio
	float2 DefocusTexelDims = (1.0f / 1024.0) * SourceRect * QuadRatio * Defocus;

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
// Simple Effect
//-----------------------------------------------------------------------------

technique TestTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = <DiffuseSampler>;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}
