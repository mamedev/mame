// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// YIQ Encode Effect
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
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// YIQ Encode Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 SourceRect;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5f / SourceDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// YIQ Encode Pixel Shader
//-----------------------------------------------------------------------------

uniform float AValue = 0.5f;
uniform float BValue = 0.5f;
uniform float CCValue = 3.5975454f;
uniform float OValue = 0.0f;
uniform float PValue = 1.0f;

uniform float ScanTime = 52.6f;
uniform float FrameOffset = 0.0f;

uniform float MaxC = 2.1183f;
uniform float MinC = -1.1183f;
uniform float CRange = 3.2366f;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float4 YDot = float4(0.299f, 0.587f, 0.114f, 0.0f);
static const float4 IDot = float4(0.595716f, -0.274453f, -0.321263f, 0.0f);
static const float4 QDot = float4(0.211456f, -0.522591f, 0.311135f, 0.0f);
static const float4 OffsetX = float4(0.0f, 0.25f, 0.50f, 0.75f);

static const float PI = 3.1415927f;
static const float PI2 = 6.2831855f;

float4 ps_main(PS_INPUT Input) : COLOR
{	
	float2 InvDims = 1.0f / SourceDims;

	float2 InvPValue = float2(PValue, 0.0f) * InvDims;

	float2 C0 = Input.TexCoord + InvPValue * OffsetX.x;
	float2 C1 = Input.TexCoord + InvPValue * OffsetX.y;
	float2 C2 = Input.TexCoord + InvPValue * OffsetX.z;
	float2 C3 = Input.TexCoord + InvPValue * OffsetX.w;
	float4 Cx = float4(C0.x, C1.x, C2.x, C3.x);
	float4 Cy = float4(C0.y, C1.y, C2.y, C3.y);
	float4 Texel0 = tex2D(DiffuseSampler, C0);
	float4 Texel1 = tex2D(DiffuseSampler, C1);
	float4 Texel2 = tex2D(DiffuseSampler, C2);
	float4 Texel3 = tex2D(DiffuseSampler, C3);

	float4 Y = float4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	float4 I = float4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	float4 Q = float4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));
	
	float BValueFrameOffset = BValue * FrameOffset;

	float4 HPosition = Cx / SourceRect.x;
	float4 VPosition = Cy * (SourceDims.y * SourceRect.y) * 2.0f;

	float4 W = PI2 * CCValue * ScanTime;
	float4 T = HPosition + AValue * VPosition + BValueFrameOffset;
	float4 TW = T * W + OValue;

	float4 Encoded = Y + I * cos(TW) + Q * sin(TW);

	return (Encoded - MinC) / CRange;;
}

//-----------------------------------------------------------------------------
// YIQ Encode Technique
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
