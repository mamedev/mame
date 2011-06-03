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
	float2 Coord0 : TEXCOORD0;
	float2 Coord1 : TEXCOORD1;
	float2 Coord2 : TEXCOORD2;
	float2 Coord3 : TEXCOORD3;
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
	float2 Coord0 : TEXCOORD0;
	float2 Coord1 : TEXCOORD1;
	float2 Coord2 : TEXCOORD2;
	float2 Coord3 : TEXCOORD3;
};

//-----------------------------------------------------------------------------
// YIQ Encode Vertex Shader
//-----------------------------------------------------------------------------

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

uniform float WidthRatio;
uniform float HeightRatio;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.x /= TargetWidth;
	Output.Position.y /= TargetHeight;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.x -= 0.5f;
	Output.Position.y -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.Coord0 = Input.TexCoord;
	Output.Coord1 = Input.TexCoord;
	Output.Coord2 = Input.TexCoord;
	Output.Coord3 = Input.TexCoord;
	
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

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 Scaler = float2(RawWidth, RawHeight);
	float2 InvRatios = float2(1.0f / WidthRatio, 1.0f / HeightRatio);

	float2 Coord0 = Input.Coord0 + float2(PValue * 0.00f, 0.0f) / Scaler;
	float2 Coord1 = Input.Coord1 + float2(PValue * 0.25f, 0.0f) / Scaler;
	float2 Coord2 = Input.Coord2 + float2(PValue * 0.50f, 0.0f) / Scaler;
	float2 Coord3 = Input.Coord3 + float2(PValue * 0.75f, 0.0f) / Scaler;

	float2 TexelOffset = 0.5f / Scaler;
	float3 Texel0 = tex2D(DiffuseSampler, Coord0 + TexelOffset).rgb;
	float3 Texel1 = tex2D(DiffuseSampler, Coord1 + TexelOffset).rgb;
	float3 Texel2 = tex2D(DiffuseSampler, Coord2 + TexelOffset).rgb;
	float3 Texel3 = tex2D(DiffuseSampler, Coord3 + TexelOffset).rgb;
	
	float PI = 3.1415926535897932384626433832795;
	float W = PI * 2.0f * CCValue * ScanTime;
	float T0 = Coord0.x * WidthRatio + AValue * Coord0.y * 2.0f * (RawHeight / HeightRatio) + BValue;
	float T1 = Coord1.x * WidthRatio + AValue * Coord1.y * 2.0f * (RawHeight / HeightRatio) + BValue;
	float T2 = Coord2.x * WidthRatio + AValue * Coord2.y * 2.0f * (RawHeight / HeightRatio) + BValue;
	float T3 = Coord3.x * WidthRatio + AValue * Coord3.y * 2.0f * (RawHeight / HeightRatio) + BValue;

	float Y0 = dot(Texel0, float3(0.299f, 0.587f, 0.114f));
	float I0 = dot(Texel0, float3(0.595716f, -0.274453f, -0.321263f));
	float Q0 = dot(Texel0, float3(0.211456f, -0.522591f, 0.311135f));

	float Y1 = dot(Texel1, float3(0.299f, 0.587f, 0.114f));
	float I1 = dot(Texel1, float3(0.595716f, -0.274453f, -0.321263f));
	float Q1 = dot(Texel1, float3(0.211456f, -0.522591f, 0.311135f));

	float Y2 = dot(Texel2, float3(0.299f, 0.587f, 0.114f));
	float I2 = dot(Texel2, float3(0.595716f, -0.274453f, -0.321263f));
	float Q2 = dot(Texel2, float3(0.211456f, -0.522591f, 0.311135f));

	float Y3 = dot(Texel3, float3(0.299f, 0.587f, 0.114f));
	float I3 = dot(Texel3, float3(0.595716f, -0.274453f, -0.321263f));
	float Q3 = dot(Texel3, float3(0.211456f, -0.522591f, 0.311135f));

	float MaxC = 2.1183f;
	float MinC = -1.1183f;
	float CRange = MaxC - MinC;
	
	float C0 = Y0 + I0 * cos(T0 * W) + Q0 * sin(T0 * W);
	float C1 = Y1 + I1 * cos(T1 * W) + Q1 * sin(T1 * W);
	float C2 = Y2 + I2 * cos(T2 * W) + Q2 * sin(T2 * W);
	float C3 = Y3 + I3 * cos(T3 * W) + Q3 * sin(T3 * W);
	C0 = (C0 - MinC) / CRange;
	C1 = (C1 - MinC) / CRange;
	C2 = (C2 - MinC) / CRange;
	C3 = (C3 - MinC) / CRange;
	
	return float4(C0, C1, C2, C3);
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
