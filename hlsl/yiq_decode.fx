//-----------------------------------------------------------------------------
// YIQ Decode Effect
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture   = <Diffuse>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
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
	float4 Coord0 : TEXCOORD0;
	float4 Coord1 : TEXCOORD1;
	float4 Coord2 : TEXCOORD2;
	float4 Coord3 : TEXCOORD3;
	float4 Coord4 : TEXCOORD4;
	float4 Coord5 : TEXCOORD5;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 ExtraInfo : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float4 Coord0 : TEXCOORD0;
	float4 Coord1 : TEXCOORD1;
	float4 Coord2 : TEXCOORD2;
	float4 Coord3 : TEXCOORD3;
	float4 Coord4 : TEXCOORD4;
	float4 Coord5 : TEXCOORD5;
};

//-----------------------------------------------------------------------------
// YIQ Decode Vertex Shader
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
	//Output.Position.x /= WidthRatio;
	//Output.Position.y /= HeightRatio;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.x -= 0.5f;
	Output.Position.y -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.Coord0.xy = Input.TexCoord + float2(-0.625f / RawWidth, 0.0f);
	Output.Coord1.xy = Input.TexCoord + float2(-0.375f / RawWidth, 0.0f);
	Output.Coord2.xy = Input.TexCoord + float2(-0.125f / RawWidth, 0.0f);
	Output.Coord3.xy = Input.TexCoord + float2(0.125f / RawWidth, 0.0f);
	Output.Coord4.xy = Input.TexCoord + float2(0.375f / RawWidth, 0.0f);
	Output.Coord5.xy = Input.TexCoord + float2(0.625f / RawWidth, 0.0f);

	return Output;
}

//-----------------------------------------------------------------------------
// YIQ Decode Pixel Shader
//-----------------------------------------------------------------------------

uniform float YSubsampleLength = 3.0f;
uniform float ISubsampleLength = 3.0f;
uniform float QSubsampleLength = 3.0f;

uniform float WValue;
uniform float AValue;
uniform float BValue;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 RawDims = float2(RawWidth, RawHeight);
	float4 OrigC = tex2D(DiffuseSampler, Input.Coord0.xy);
	float2 OrigC2 = tex2D(DiffuseSampler, Input.Coord4.xy);
	float4 C = OrigC;
	float2 C2 = OrigC2;
	
	float MaxC = 2.1183f;
	float MinC = -1.1183f;
	float CRange = MaxC - MinC;

	C = C * CRange + MinC;
	C2 = C2 * CRange + MinC;
	
	float2 Coord0 = Input.Coord0.xy * RawDims;
	float2 Coord1 = Input.Coord1.xy * RawDims;
	float2 Coord2 = Input.Coord2.xy * RawDims;
	float2 Coord3 = Input.Coord3.xy * RawDims;
	float2 Coord4 = Input.Coord4.xy * RawDims;
	float2 Coord5 = Input.Coord5.xy * RawDims;
	
	float W = WValue;
	float T0 = Coord0.x + AValue * Coord0.y + BValue;
	float T1 = Coord1.x + AValue * Coord1.y + BValue;
	float T2 = Coord2.x + AValue * Coord2.y + BValue;
	float T3 = Coord3.x + AValue * Coord3.y + BValue;
	float T4 = Coord4.x + AValue * Coord4.y + BValue;
	float T5 = Coord5.x + AValue * Coord5.y + BValue;
	float4 Tc = float4(T0, T1, T2, T3);
	float2 Tc2 = float2(T4, T5);
	
	float Y = (C.r + C.g + C.b + C.a + C2.r + C2.g) / 6.0f;
	
	float4 I = C * sin(W * Tc);
	float4 Q = C * cos(W * Tc);
	float2 I2 = C2 * sin(W * Tc2);
	float2 Q2 = C2 * cos(W * Tc2);
	
	float Iavg = (I.r + I.g + I.b + I.a + I2.r + I2.g) / 3.0f;
	float Qavg = (Q.r + Q.g + Q.b + Q.a + Q2.r + Q2.g) / 3.0f;

	float3 YIQ = float3(Y, Iavg, Qavg);
	
	float3 OutRGB = float3(dot(YIQ, float3(1.0f, 0.9563f, 0.6210f)), dot(YIQ, float3(1.0f, -0.2721f, -0.6474f)), dot(YIQ, float3(1.0f, -1.1070f, 1.7046f)));	
	
	return float4(OutRGB, 1.0f);
}

//-----------------------------------------------------------------------------
// YIQ Decode Technique
//-----------------------------------------------------------------------------

technique DecodeTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
