// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// NTSC Effect
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
// NTSC Pixel Shader
//-----------------------------------------------------------------------------

uniform float AValue = 0.5f;
uniform float BValue = 0.5f;
uniform float CCValue = 3.5795454f;
uniform float OValue = 0.0f;
uniform float PValue = 1.0f;
uniform float ScanTime = 52.6f;

uniform float NotchHalfWidth = 1.0f;
uniform float YFreqResponse = 6.0f;
uniform float IFreqResponse = 1.2f;
uniform float QFreqResponse = 0.6f;

uniform float SignalOffset = 0.0f;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float PI = 3.1415927f;
static const float PI2 = PI * 2.0f;

static const float4 YDot = float4(0.299f, 0.587f, 0.114f, 0.0f);
static const float4 IDot = float4(0.595716f, -0.274453f, -0.321263f, 0.0f);
static const float4 QDot = float4(0.211456f, -0.522591f, 0.311135f, 0.0f);

static const float3 RDot = float3(1.0f, 0.956f, 0.621f);
static const float3 GDot = float3(1.0f, -0.272f, -0.647f);
static const float3 BDot = float3(1.0f, -1.106f, 1.703f);

static const float4 OffsetX = float4(0.0f, 0.25f, 0.50f, 0.75f);
static const float4 NotchOffset = float4(0.0f, 1.0f, 2.0f, 3.0f);

static const int SampleCount = 64;
static const int HalfSampleCount = SampleCount / 2;

float4 GetCompositeYIQ(float2 TexCoord)
{
	float2 SourceTexelDims = 1.0f / SourceDims;
	float2 SourceRes = SourceDims * SourceRect;

	float2 PValueSourceTexel = float2(PValue, 0.0f) * SourceTexelDims;

	float2 C0 = TexCoord + PValueSourceTexel * OffsetX.x;
	float2 C1 = TexCoord + PValueSourceTexel * OffsetX.y;
	float2 C2 = TexCoord + PValueSourceTexel * OffsetX.z;
	float2 C3 = TexCoord + PValueSourceTexel * OffsetX.w;
	float4 Cx = float4(C0.x, C1.x, C2.x, C3.x);
	float4 Cy = float4(C0.y, C1.y, C2.y, C3.y);
	float4 Texel0 = tex2D(DiffuseSampler, C0);
	float4 Texel1 = tex2D(DiffuseSampler, C1);
	float4 Texel2 = tex2D(DiffuseSampler, C2);
	float4 Texel3 = tex2D(DiffuseSampler, C3);

	float4 HPosition = Cx / SourceRect.x;
	float4 VPosition = Cy / SourceRect.y;

	float4 Y = float4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	float4 I = float4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	float4 Q = float4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

	float W = PI2 * CCValue * ScanTime;
	float WoPI = W / PI;

	float HOffset = (BValue + SignalOffset) / WoPI;
	float VScale = (AValue * SourceRes.y) / WoPI;

	float4 T = HPosition + HOffset + VPosition * VScale;
	float4 TW = T * W;

	float4 CompositeYIQ = Y + I * cos(TW) + Q * sin(TW);

	return CompositeYIQ;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);

	float2 SourceTexelDims = 1.0f / SourceDims;
	float2 SourceRes = SourceDims * SourceRect;

	float TimePerSample = ScanTime / (SourceRes.x * 4.0f);

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
	float PI2Length = PI2 / SampleCount;

	float W = PI2 * CCValue * ScanTime;
	float WoPI = W / PI;

	float HOffset = (BValue + SignalOffset) / WoPI;
	float VScale = (AValue * SourceRes.y) / WoPI;

	float4 YAccum = 0.0f;
	float4 IAccum = 0.0f;
	float4 QAccum = 0.0f;

	float4 Cy = Input.TexCoord.y;
	float4 VPosition = Cy / SourceRect.y;

	for (float i = 0; i < SampleCount; i += 4.0f)
	{
		float n = i - HalfSampleCount;

		float4 n4 = n + NotchOffset;

		float4 Cx = Input.TexCoord.x + SourceTexelDims.x * (n4 * 0.25f);
		float4 HPosition = Cx / SourceRect.x;

		float4 C = GetCompositeYIQ(float2(Cx.r, Cy.r));

		float4 T = HPosition + HOffset + VPosition * VScale;
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

	float3 YIQ = float3(
		(YAccum.r + YAccum.g + YAccum.b + YAccum.a),
		(IAccum.r + IAccum.g + IAccum.b + IAccum.a) * 2.0f,
		(QAccum.r + QAccum.g + QAccum.b + QAccum.a) * 2.0f);

	float3 RGB = float3(
		dot(YIQ, RDot),
		dot(YIQ, GDot),
		dot(YIQ, BDot));

	return float4(RGB, BaseTexel.a);
}

//-----------------------------------------------------------------------------
// NTSC Technique
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
