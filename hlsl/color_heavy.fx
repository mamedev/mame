//-----------------------------------------------------------------------------
// Color-Convolution Effect
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
	float2 ExtraInfo : TEXCOORD1;
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
	float2 TexCoord : TEXCOORD0;
	float2 ExtraInfo : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Post-Processing Vertex Shader
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
	Output.TexCoord = Input.TexCoord;//(Input.TexCoord - float2(0.5f, 0.5f)) / 8.0f + float2(0.25f, 0.25f);
	Output.ExtraInfo = Input.ExtraInfo;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float RedFromRed = 1.0f;
uniform float RedFromGrn = 0.0f;
uniform float RedFromBlu = 0.0f;
uniform float GrnFromRed = 0.0f;
uniform float GrnFromGrn = 1.0f;
uniform float GrnFromBlu = 0.0f;
uniform float BluFromRed = 0.0f;
uniform float BluFromGrn = 0.0f;
uniform float BluFromBlu = 1.0f;

uniform float YfromY = 1.0f;
uniform float YfromI = 0.0f;
uniform float YfromQ = 0.0f;
uniform float IfromY = 0.0f;
uniform float IfromI = 1.0f;
uniform float IfromQ = 0.0f;
uniform float QfromY = 0.0f;
uniform float QfromI = 0.0f;
uniform float QfromQ = 1.0f;

uniform float RedOffset = 0.0f;
uniform float GrnOffset = 0.0f;
uniform float BluOffset = 0.0f;

uniform float RedScale = 1.0f;
uniform float GrnScale = 1.0f;
uniform float BluScale = 1.0f;

uniform float RedFloor = 0.0f;
uniform float GrnFloor = 0.0f;
uniform float BluFloor = 0.0f;

uniform float Saturation = 1.0f;

uniform float YScale = 1.0f;
uniform float IScale = 1.0f;
uniform float QScale = 1.0f;
uniform float YOffset = 0.0f;
uniform float IOffset = 0.0f;
uniform float QOffset = 0.0f;

uniform float RedPower = 2.2f;
uniform float GrnPower = 2.2f;
uniform float BluPower = 2.2f;

uniform float YSubsampleLength = 3.0f;
uniform float ISubsampleLength = 3.0f;
uniform float QSubsampleLength = 3.0f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	// -- Bandwidth Subsampling --
	float YSubsampleWidth = (RawWidth * 2.0f) / YSubsampleLength;
	float ISubsampleWidth = (RawWidth * 2.0f) / ISubsampleLength;
	float QSubsampleWidth = (RawWidth * 2.0f) / QSubsampleLength;
	float3 SubsampleWidth = float3(YSubsampleWidth, ISubsampleWidth, QSubsampleWidth);
	float3 SubsampleFrac = frac(Input.TexCoord.x * SubsampleWidth);
	float3 SubsampleCoord = (Input.TexCoord.x * SubsampleWidth - SubsampleFrac) / SubsampleWidth + SubsampleFrac / (SubsampleWidth * 2.0f);

	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);
	
	float3 YTexel = tex2D(DiffuseSampler, float2(SubsampleCoord.x, Input.TexCoord.y)).rgb;
	float3 ITexel = tex2D(DiffuseSampler, float2(SubsampleCoord.y, Input.TexCoord.y)).rgb;
	float3 QTexel = tex2D(DiffuseSampler, float2(SubsampleCoord.z, Input.TexCoord.y)).rgb;
	
	float3 LastYTexel = tex2D(DiffuseSampler, float2(SubsampleCoord.x - SubsampleFrac.x / RawWidth, Input.TexCoord.y)).rgb;
	float3 LastITexel = tex2D(DiffuseSampler, float2(SubsampleCoord.y - SubsampleFrac.y / RawWidth, Input.TexCoord.y)).rgb;
	float3 LastQTexel = tex2D(DiffuseSampler, float2(SubsampleCoord.z - SubsampleFrac.z / RawWidth, Input.TexCoord.y)).rgb;
	
	YTexel = lerp(LastYTexel, YTexel, SubsampleFrac.x);
	ITexel = lerp(LastITexel, ITexel, SubsampleFrac.y);
	QTexel = lerp(LastQTexel, QTexel, SubsampleFrac.z);

	// -- RGB Tint & Shift --
	float ShiftedRedY = dot(YTexel, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrnY = dot(YTexel, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBluY = dot(YTexel, float3(BluFromRed, BluFromGrn, BluFromBlu));
	float ShiftedRedI = dot(ITexel, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrnI = dot(ITexel, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBluI = dot(ITexel, float3(BluFromRed, BluFromGrn, BluFromBlu));
	float ShiftedRedQ = dot(QTexel, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrnQ = dot(QTexel, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBluQ = dot(QTexel, float3(BluFromRed, BluFromGrn, BluFromBlu));
	
	// -- RGB Offset & Scale --
	float3 RGBScale = float3(RedScale, GrnScale, BluScale);
	float3 RGBShift = float3(RedOffset, GrnOffset, BluOffset);
	float3 OutTexelY = float3(ShiftedRedY, ShiftedGrnY, ShiftedBluY) * RGBScale + RGBShift;
	float3 OutTexelI = float3(ShiftedRedI, ShiftedGrnI, ShiftedBluI) * RGBScale + RGBShift;
	float3 OutTexelQ = float3(ShiftedRedQ, ShiftedGrnQ, ShiftedBluQ) * RGBScale + RGBShift;
	
	// -- Saturation --
	float3 Gray = float3(0.3f, 0.59f, 0.11f);
	float OutLumaY = dot(OutTexelY, Gray);
	float OutLumaI = dot(OutTexelI, Gray);
	float OutLumaQ = dot(OutTexelQ, Gray);
	float3 OutChromaY = OutTexelY - OutLumaY;
	float3 OutChromaI = OutTexelI - OutLumaI;
	float3 OutChromaQ = OutTexelQ - OutLumaQ;
	float3 SaturatedY = OutLumaY + OutChromaY * Saturation;
	float3 SaturatedI = OutLumaI + OutChromaI * Saturation;
	float3 SaturatedQ = OutLumaQ + OutChromaQ * Saturation;
	
	// -- YIQ Convolution --
	float Y = dot(SaturatedY, float3(0.299f, 0.587f, 0.114f));
	float I = dot(SaturatedI, float3(0.595716f, -0.274453f, -0.321263f));
	float Q = dot(SaturatedQ, float3(0.211456f, -0.522591f, 0.311135f));
	Y = dot(float3(Y, I, Q), float3(YfromY, YfromI, YfromQ));
	I = dot(float3(Y, I, Q), float3(IfromY, IfromI, IfromQ));
	Q = dot(float3(Y, I, Q), float3(QfromY, QfromI, QfromQ));
	float3 OutYIQ = float3(Y, I, Q) * float3(YScale, IScale, QScale) + float3(YOffset, IOffset, QOffset);
	float3 OutRGB = float3(dot(OutYIQ, float3(1.0f, 0.9563f, 0.6210f)), dot(OutYIQ, float3(1.0f, -0.2721f, -0.6474f)), dot(OutYIQ, float3(1.0f, -1.1070f, 1.7046f)));

	OutRGB.r = pow(OutRGB.r, RedPower);
	OutRGB.g = pow(OutRGB.g, GrnPower);
	OutRGB.b = pow(OutRGB.b, BluPower);

	// -- Color Compression (increasing the floor of the signal without affecting the ceiling) --
	OutRGB = float3(RedFloor + (1.0f - RedFloor) * OutRGB.r, GrnFloor + (1.0f - GrnFloor) * OutRGB.g, BluFloor + (1.0f - BluFloor) * OutRGB.b);

	// -- Final Pixel --
	float4 Output = lerp(Input.Color, float4(OutRGB, BaseTexel.a) * Input.Color, Input.ExtraInfo.x);
	
	return BaseTexel;
}

//-----------------------------------------------------------------------------
// Color-Convolution Technique
//-----------------------------------------------------------------------------

technique ColorTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
