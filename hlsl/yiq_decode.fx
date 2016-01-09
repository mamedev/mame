// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// YIQ Decode Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Composite;

sampler CompositeSampler = sampler_state
{
	Texture   = <Composite>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

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
	float4 TexCoord : TEXCOORD0;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// YIQ Decode Vertex Shader
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

	Output.TexCoord.xy = Input.TexCoord;
	Output.TexCoord.xy += 0.5f / SourceDims; // half texel offset correction (DX9)

	return Output;
}

//-----------------------------------------------------------------------------
// YIQ Decode Pixel Shader
//-----------------------------------------------------------------------------

uniform float AValue = 0.5f;
uniform float BValue = 0.5f;
uniform float CCValue = 3.5975454f;
uniform float OValue = 0.0f;
uniform float PValue = 1.0f; // unused

uniform float ScanTime = 52.6f;
uniform float FrameOffset = 0.0f;

uniform float NotchHalfWidth = 1.0f;
uniform float YFreqResponse = 6.0f;
uniform float IFreqResponse = 1.2f;
uniform float QFreqResponse = 0.6f;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float4 NotchOffset = float4(0.0f, 1.0f, 2.0f, 3.0f);

static const float PI = 3.1415927f;
static const float PI2 = 6.2831855f;

static const float MaxC = 2.1183f;
static const float MinC = -1.1183f;
static const float CRange = 3.2366f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord.xy);

	float2 InvDims = 1.0f / SourceDims;

	// YIQ convolution: N coefficients each
	float4 YAccum = 0.0f;
	float4 IAccum = 0.0f;
	float4 QAccum = 0.0f;

	float BValueFrameOffset = BValue * FrameOffset;

	float FrameWidthx4 = SourceDims.x * SourceRect.x * 4.0f;
	float TimePerSample = ScanTime / FrameWidthx4;

	float Fc_y1 = (CCValue - NotchHalfWidth) * TimePerSample;
	float Fc_y2 = (CCValue + NotchHalfWidth) * TimePerSample;
	float Fc_y3 = YFreqResponse * TimePerSample;
	float Fc_i = IFreqResponse * TimePerSample;
	float Fc_q = QFreqResponse * TimePerSample;
	float Fc_i_2 = Fc_i * 2.0f;
	float Fc_q_2 = Fc_q * 2.0f;
	float Fc_y1_2 = Fc_y1 * 2.0f;
	float Fc_y2_2 = Fc_y2 * 2.0f;
	float Fc_y3_2 = Fc_y3 * 2.0f;
	float Fc_i_pi2 = Fc_i * PI2;
	float Fc_q_pi2 = Fc_q * PI2;
	float Fc_y1_pi2 = Fc_y1 * PI2;
	float Fc_y2_pi2 = Fc_y2 * PI2;
	float Fc_y3_pi2 = Fc_y3 * PI2;
	float PI2Length = PI2 / 82.0f;

	float W = PI2 * CCValue * ScanTime;

	float4 Cy = Input.TexCoord.y;
	float4 VPosition = Cy * (SourceDims.y * SourceRect.y) * 2.0f;

	for(float n = -41.0f; n < 42.0f; n += 4.0f)
	{
		float4 n4 = n + NotchOffset;

		float4 Cx = Input.TexCoord.x + InvDims.x * n4 * 0.25f;
		float4 HPosition = (Cx / SourceRect.x);

		float4 C = tex2D(CompositeSampler, float2(Cx.r, Cy.r)) * CRange + MinC;

		float4 T = HPosition + AValue * VPosition + BValueFrameOffset;
		float4 WT = W * T + OValue;

		float4 SincKernel = 0.54f + 0.46f * cos(PI2Length * n4);

		float4 SincYIn1 = Fc_y1_pi2 * n4;
		float4 SincYIn2 = Fc_y2_pi2 * n4;
		float4 SincYIn3 = Fc_y3_pi2 * n4;
		float4 SincIIn = Fc_i_pi2 * n4;
		float4 SincQIn = Fc_q_pi2 * n4;

		float4 SincY1 = SincYIn1 != 0.0f ? sin(SincYIn1) / SincYIn1 : 1.0f;
		float4 SincY2 = SincYIn2 != 0.0f ? sin(SincYIn2) / SincYIn2 : 1.0f;
		float4 SincY3 = SincYIn3 != 0.0f ? sin(SincYIn3) / SincYIn3 : 1.0f;

		float4 IdealY = (Fc_y1_2 * SincY1 - Fc_y2_2 * SincY2) + Fc_y3_2 * SincY3;
		float4 IdealI = Fc_i_2 * (SincIIn != 0.0f ? sin(SincIIn) / SincIIn : 1.0f);
		float4 IdealQ = Fc_q_2 * (SincQIn != 0.0f ? sin(SincQIn) / SincQIn : 1.0f);

		float4 FilterY = SincKernel * IdealY;
		float4 FilterI = SincKernel * IdealI;
		float4 FilterQ = SincKernel * IdealQ;

		YAccum = YAccum + C * FilterY;
		IAccum = IAccum + C * cos(WT) * FilterI;
		QAccum = QAccum + C * sin(WT) * FilterQ;
	}

	float Y = YAccum.r + YAccum.g + YAccum.b + YAccum.a;
	float I = (IAccum.r + IAccum.g + IAccum.b + IAccum.a) * 2.0f;
	float Q = (QAccum.r + QAccum.g + QAccum.b + QAccum.a) * 2.0f;

	float3 YIQ = float3(Y, I, Q);

	float3 Decode = float3(
		dot(YIQ, float3(1.0f, 0.956f, 0.621f)),
		dot(YIQ, float3(1.0f, -0.272f, -0.647f)),
		dot(YIQ, float3(1.0f, -1.106f, 1.703f)));	
	return float4(Decode, BaseTexel.a);
}

//-----------------------------------------------------------------------------
// YIQ Decode Technique
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
