// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Deconvergence Effect
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
	//float2 RedCoord : TEXCOORD0;
	//float2 GrnCoord : TEXCOORD1;
	//float2 BluCoord : TEXCOORD2;
	float3 CoordX : TEXCOORD0;
	float3 CoordY : TEXCOORD1;
	float2 TexCoord : TEXCOORD2;
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
	float4 Color : COLOR0;
	//float2 RedCoord : TEXCOORD0;
	//float2 GrnCoord : TEXCOORD1;
	//float2 BluCoord : TEXCOORD2;
	float3 CoordX : TEXCOORD0;
	float3 CoordY : TEXCOORD1;
	float2 TexCoord : TEXCOORD2;
};

//-----------------------------------------------------------------------------
// Deconvergence Vertex Shader
//-----------------------------------------------------------------------------

uniform float3 ConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 ConvergeY = float3(0.0f, 0.0f, 0.0f);

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 SourceRect;

uniform float3 RadialConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 RadialConvergeY = float3(0.0f, 0.0f, 0.0f);

uniform float Prescale;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float2 invDims = 1.0f / SourceDims;
	float2 Ratios = SourceRect;
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	float2 TexCoord = Input.TexCoord;

	float2 RadialRed = float2(RadialConvergeX.x, RadialConvergeY.x);
	float2 RadialGrn = float2(RadialConvergeX.y, RadialConvergeY.y);
	float2 RadialBlu = float2(RadialConvergeX.z, RadialConvergeY.z);
	float2 ConvergeRed = float2(ConvergeX.x, ConvergeY.x);
	float2 ConvergeGrn = float2(ConvergeX.y, ConvergeY.y);
	float2 ConvergeBlu = float2(ConvergeX.z, ConvergeY.z);
	float2 ScaledRatio = ((TexCoord * SourceRect) - 0.5f);

	Output.CoordX = ((((TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + RadialConvergeX / SourceDims.x) + 0.5f) * Ratios.x + ConvergeX * invDims.x;
	Output.CoordY = ((((TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + RadialConvergeY / SourceDims.y) + 0.5f) * Ratios.y + ConvergeY * invDims.y;
	Output.TexCoord = TexCoord;

	return Output;
}

//-----------------------------------------------------------------------------
// Deconvergence Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float Alpha = tex2D(DiffuseSampler, Input.TexCoord).a;
	float RedTexel = tex2D(DiffuseSampler, float2(Input.CoordX.x, Input.CoordY.x)).r;
	float GrnTexel = tex2D(DiffuseSampler, float2(Input.CoordX.y, Input.CoordY.y)).g;
	float BluTexel = tex2D(DiffuseSampler, float2(Input.CoordX.z, Input.CoordY.z)).b;

	return float4(RedTexel, GrnTexel, BluTexel, Alpha);
}

//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

technique DeconvergeTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
