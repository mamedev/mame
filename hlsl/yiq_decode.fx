//-----------------------------------------------------------------------------
// YIQ Decode Effect
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
	float4 Coord6 : TEXCOORD6;
	float4 Coord7 : TEXCOORD7;
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
	float4 Coord0 : TEXCOORD0;
	float4 Coord1 : TEXCOORD1;
	float4 Coord2 : TEXCOORD2;
	float4 Coord3 : TEXCOORD3;
	float4 Coord4 : TEXCOORD4;
	float4 Coord5 : TEXCOORD5;
	float4 Coord6 : TEXCOORD6;
	float4 Coord7 : TEXCOORD7;
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

uniform float FscValue;

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
	Output.Coord0.xy = Input.TexCoord + float2(0.00f / RawWidth, 0.0f);
	Output.Coord1.xy = Input.TexCoord + float2(0.25f / RawWidth, 0.0f);
	Output.Coord2.xy = Input.TexCoord + float2(0.50f / RawWidth, 0.0f);
	Output.Coord3.xy = Input.TexCoord + float2(0.75f / RawWidth, 0.0f);
	Output.Coord4.xy = Input.TexCoord + float2(1.00f / RawWidth, 0.0f);
	Output.Coord5.xy = Input.TexCoord + float2(1.25f / RawWidth, 0.0f);
	Output.Coord6.xy = Input.TexCoord + float2(1.50f / RawWidth, 0.0f);
	Output.Coord7.xy = Input.TexCoord + float2(1.75f / RawWidth, 0.0f);
	Output.Coord0.zw = Input.TexCoord + float2(2.00f / RawWidth, 0.0f);
	Output.Coord1.zw = Input.TexCoord + float2(2.25f / RawWidth, 0.0f);
	Output.Coord2.zw = Input.TexCoord + float2(2.50f / RawWidth, 0.0f);
	Output.Coord3.zw = Input.TexCoord + float2(2.75f / RawWidth, 0.0f);

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
	float4 BaseTexel = tex2D(DiffuseSampler, Input.Coord0.xy + float2(0.5f, 0.5f) / RawDims);
	float4 OrigC = tex2D(CompositeSampler, Input.Coord0.xy + float2(0.5f, 0.5f) / RawDims);
	float4 OrigC2 = tex2D(CompositeSampler, Input.Coord4.xy + float2(0.5f, 0.5f) / RawDims);
	float4 OrigC3 = tex2D(CompositeSampler, Input.Coord0.zw + float2(0.5f, 0.5f) / RawDims);
	float4 C = OrigC;
	float4 C2 = OrigC2;
	float4 C3 = OrigC3;
	
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
	float2 Coord6 = Input.Coord6.xy * RawDims;
	float2 Coord7 = Input.Coord7.xy * RawDims;
	float2 Coord8 = Input.Coord0.zw * RawDims;
	float2 Coord9 = Input.Coord1.zw * RawDims;
	float2 CoordA = Input.Coord2.zw * RawDims;
	float2 CoordB = Input.Coord3.zw * RawDims;
	
	float W = WValue * 2.0f;
	float T0 = Coord0.x + AValue * Coord0.y + BValue;
	float T1 = Coord1.x + AValue * Coord1.y + BValue;
	float T2 = Coord2.x + AValue * Coord2.y + BValue;
	float T3 = Coord3.x + AValue * Coord3.y + BValue;
	float T4 = Coord4.x + AValue * Coord4.y + BValue;
	float T5 = Coord5.x + AValue * Coord5.y + BValue;
	float T6 = Coord6.x + AValue * Coord6.y + BValue;
	float T7 = Coord7.x + AValue * Coord7.y + BValue;
	float T8 = Coord8.x + AValue * Coord8.y + BValue;
	float T9 = Coord9.x + AValue * Coord9.y + BValue;
	float TA = CoordA.x + AValue * CoordA.y + BValue;
	float TB = CoordB.x + AValue * CoordB.y + BValue;
	float4 Tc = float4(T0, T1, T2, T3);
	float4 Tc2 = float4(T4, T5, T6, T7);
	float4 Tc3 = float4(T8, T9, TA, TB);
	
	float Yvals[8];
	Yvals[0] = C.r; Yvals[1] = C.g; Yvals[2] = C.b; Yvals[3] = C.a; Yvals[4] = C2.r; Yvals[5] = C2.g; Yvals[6] = C2.b; Yvals[7] = C2.a;
	float Ytotal = 0.0f;
	for(uint idx = 0; idx < FscValue * 4.0f; idx++ )
	{
		Ytotal = Ytotal + Yvals[idx];
	}
	float Yavg = Ytotal / (FscValue * 4.0f);

	float4 I = (C - Yavg) * sin(W * Tc);
	float4 Q = (C - Yavg) * cos(W * Tc);
	float4 I2 = (C2 - Yavg) * sin(W * Tc2);
	float4 Q2 = (C2 - Yavg) * cos(W * Tc2);
	float4 I3 = (C3 - Yavg) * sin(W * Tc3);
	float4 Q3 = (C3 - Yavg) * cos(W * Tc3);
	
	float Itotal = 0.0f;
	float Qtotal = 0.0f;
	
	float Ivals[8];
	float Qvals[8];
	Ivals[0] = I.r; Ivals[1] = I.g; Ivals[2] = I.b; Ivals[3] = I.a; Ivals[4] = I2.r; Ivals[5] = I2.g; Ivals[6] = I2.b; Ivals[7] = I2.a;
	Qvals[0] = Q.r; Qvals[1] = Q.g; Qvals[2] = Q.b; Qvals[3] = Q.a; Qvals[4] = Q2.r; Qvals[5] = Q2.g; Qvals[6] = Q2.b; Qvals[7] = Q2.a;
	for(uint idx = 0; idx < FscValue * 4.0f; idx++ )
	{
		Itotal = Itotal + Ivals[idx];
		Qtotal = Qtotal + Qvals[idx];
	}
	float Iavg = Itotal / (FscValue * 3.0f);
	float Qavg = Qtotal / (FscValue * 3.0f);

	float3 YIQ = float3(Yavg, Iavg * 1.5f, Qavg);
	
	float3 OutRGB = float3(dot(YIQ, float3(1.0f, 0.956f, 0.621f)), dot(YIQ, float3(1.0f, -0.272f, -0.647f)), dot(YIQ, float3(1.0f, -1.106f, 1.703f)));	
	
	return float4(OutRGB, BaseTexel.a);
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
