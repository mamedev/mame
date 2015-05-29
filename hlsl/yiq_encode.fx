// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// YIQ Encode Effect
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
	float2 Unused : TEXCOORD1;
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

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.TexCoord = Input.TexCoord;
	
	return Output;
}

//-----------------------------------------------------------------------------
// YIQ Encode Pixel Shader
//-----------------------------------------------------------------------------

uniform float AValue = 0.0f;
uniform float BValue = 0.0f;
uniform float CCValue = 3.04183f;
uniform float PValue = 1.0f;
uniform float ScanTime = 52.6f;

uniform float2 SourceDims;
uniform float2 SourceRect;

uniform float4 YDot = float4(0.299f, 0.587f, 0.114f, 0.0f);
uniform float4 IDot = float4(0.595716f, -0.274453f, -0.321263f, 0.0f);
uniform float4 QDot = float4(0.211456f, -0.522591f, 0.311135f, 0.0f);
uniform float4 OffsetX = float4(0.00f, 0.25f, 0.50f, 0.75f);

uniform float PI = 3.1415926535f;
uniform float PI2 = 6.2831853072f;

uniform float MaxC = 2.1183f;
uniform float MinC = -1.1183f;
uniform float CRange = 3.2366f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 InvDims = 1.0f / SourceDims;
	float4 CoordX = float4(Input.TexCoord.x + OffsetX * InvDims.x);
	float4 CoordY = Input.TexCoord.y;

	float2 TexelOffset = InvDims * 0.5f;
	float4 Texel0 = tex2D(DiffuseSampler, float2(CoordX.x, CoordY.x) + TexelOffset);
	float4 Texel1 = tex2D(DiffuseSampler, float2(CoordX.y, CoordY.y) + TexelOffset);
	float4 Texel2 = tex2D(DiffuseSampler, float2(CoordX.z, CoordY.z) + TexelOffset);
	float4 Texel3 = tex2D(DiffuseSampler, float2(CoordX.w, CoordY.w) + TexelOffset);
	
	float4 Y = float4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	float4 I = float4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	float4 Q = float4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

	float4 W = PI2 * CCValue * ScanTime;
	float4 VPosition = (CoordY * SourceRect.y) * (SourceDims.x / SourceRect.x);
	float4 T = CoordX / SourceRect.x + VPosition + BValue;
	
	float4 C = Y + I * cos(T * W) + Q * sin(T * W);
	C = (C - MinC) / CRange;
	
	return C;
}

//-----------------------------------------------------------------------------
// YIQ Encode Technique
//-----------------------------------------------------------------------------

technique EncodeTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
