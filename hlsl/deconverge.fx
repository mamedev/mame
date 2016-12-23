// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Deconvergence Effect
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
	float3 TexCoordX : TEXCOORD0;
	float3 TexCoordY : TEXCOORD1;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float3 TexCoordX : TEXCOORD0;
	float3 TexCoordY : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Deconvergence Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;

uniform float3 ConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 ConvergeY = float3(0.0f, 0.0f, 0.0f);
uniform float3 RadialConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 RadialConvergeY = float3(0.0f, 0.0f, 0.0f);

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

	// imaginary texel dimensions independed from screen dimension, but ratio
	float2 TexelDims = (1.0f / 1024);

	Output.TexCoordX = TexCoord.xxx;
	Output.TexCoordY = TexCoord.yyy;

	// center coordinates
	Output.TexCoordX -= 0.5f;
	Output.TexCoordY -= 0.5f;

	// radial converge offset to "translate" the most outer pixel as thay would be translated by the linar converge with the same amount
	float2 radialConvergeOffset = 2.0f;

	// radial converge
	Output.TexCoordX *= 1.0f + RadialConvergeX * TexelDims.xxx * radialConvergeOffset.xxx;
	Output.TexCoordY *= 1.0f + RadialConvergeY * TexelDims.yyy * radialConvergeOffset.yyy;

	// un-center coordinates
	Output.TexCoordX += 0.5f;
	Output.TexCoordY += 0.5f;

	// linear converge
	Output.TexCoordX += ConvergeX * TexelDims.xxx;
	Output.TexCoordY += ConvergeY * TexelDims.yyy;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Deconvergence Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float r = tex2D(DiffuseSampler, float2(Input.TexCoordX.x, Input.TexCoordY.x)).r;
	float g = tex2D(DiffuseSampler, float2(Input.TexCoordX.y, Input.TexCoordY.y)).g;
	float b = tex2D(DiffuseSampler, float2(Input.TexCoordX.z, Input.TexCoordY.z)).b;

	return float4(r, g, b, 1.0f);
}

//-----------------------------------------------------------------------------
// Deconvergence Technique
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
