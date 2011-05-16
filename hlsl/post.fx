//-----------------------------------------------------------------------------
// Post-Processing Effect
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

texture Shadow;

sampler ShadowSampler = sampler_state
{
	Texture   = <Shadow>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
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
	float3 CoordX : TEXCOORD2;
	float3 CoordY : TEXCOORD3;
	float2 ShadowCoord : TEXCOORD4;
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
	float3 CoordX : TEXCOORD2;
	float3 CoordY : TEXCOORD3;
	float2 ShadowCoord : TEXCOORD4;
};

//-----------------------------------------------------------------------------
// Post-Processing Vertex Shader
//-----------------------------------------------------------------------------

uniform float RedConvergeX;
uniform float RedConvergeY;
uniform float GrnConvergeX;
uniform float GrnConvergeY;
uniform float BluConvergeX;
uniform float BluConvergeY;

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

uniform float WidthRatio;
uniform float HeightRatio;

uniform float RedRadialConvergeX;
uniform float RedRadialConvergeY;
uniform float GrnRadialConvergeX;
uniform float GrnRadialConvergeY;
uniform float BluRadialConvergeX;
uniform float BluRadialConvergeY;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	float2 invDims = float2(1.0f / RawWidth, 1.0f / RawHeight);
	float2 Ratios = float2(1.0f / WidthRatio, 1.0f / HeightRatio);
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
	
	Output.CoordX.x = ((((Output.TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + RedRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + RedConvergeX * invDims.x;
	Output.CoordX.y = ((((Output.TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + GrnRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + GrnConvergeX * invDims.x;
	Output.CoordX.z = ((((Output.TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + BluRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + GrnConvergeX * invDims.x;
	
	Output.CoordY.x = ((((Output.TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + RedRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + RedConvergeY * invDims.y;
	Output.CoordY.y = ((((Output.TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + GrnRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + BluConvergeY * invDims.y;
	Output.CoordY.z = ((((Output.TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + BluRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + BluConvergeY * invDims.y;
	
	Output.ShadowCoord = Output.TexCoord * float2(RawWidth, RawHeight);
	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float PI = 3.14159265f;

uniform float PincushionAmountX = 0.1f;
uniform float PincushionAmountY = 0.1f;

uniform float ScanlineAmount = 1.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;

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

uniform float EdgeDetectScale = 1.0f;
uniform float EdgeToBaseRatio = 0.0f;

uniform float SubsampleLength = 3.0f;

uniform float CurrFrame = 0.0f;
uniform float CrawlWidth = 3.0f;
uniform float CrawlHeight = 3.0f;
uniform float CrawlRate = 3.0f;

uniform float UseShadow = 0.0f;
uniform float ShadowBrightness = 1.0f;
uniform float ShadowPixelSizeX = 3.0f;
uniform float ShadowPixelSizeY = 3.0f;
uniform float ShadowU = 0.375f;
uniform float ShadowV = 0.375f;
uniform float ShadowWidth = 8.0f;
uniform float ShadowHeight = 8.0f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 Ratios = float2(WidthRatio, HeightRatio);

	// -- Screen Pincushion Calculation --
	float2 UnitCoord = Input.TexCoord * Ratios * 2.0f - 1.0f;

	float PincushionR2 = pow(length(UnitCoord),2.0f) / pow(length(Ratios), 2.0f);
	float2 PincushionCurve = UnitCoord * PincushionAmountX * PincushionR2;
	float2 BaseCoord = Input.TexCoord + PincushionCurve;

	// RGB Pincushion Calculation
	float3 PincushionCurveX = UnitCoord.x * PincushionAmountX * PincushionR2;
	float3 PincushionCurveY = UnitCoord.y * PincushionAmountX * PincushionR2;

	float4 BaseTexel = tex2D(DiffuseSampler, BaseCoord);

	// -- Alpha Clipping (1px border in drawd3d does not work for some reason) --
	clip((BaseCoord.x < 2.0f / RawWidth) ? -1 : 1);
	clip((BaseCoord.y < 1.0f / RawHeight) ? -1 : 1);
	clip((BaseCoord.x > (2.0f / WidthRatio - 2.0f / RawWidth)) ? -1 : 1);
	clip((BaseCoord.y > 1.0f / HeightRatio) ? -1 : 1);

	// -- Chroma Subsampling & RGB Deconvergence --
	float3 CoordX = Input.CoordX + PincushionCurveX;
	float3 CoordY = Input.CoordY + PincushionCurveY;
	float RedTexel = tex2D(DiffuseSampler, float2(CoordX.x, CoordY.x)).r;
	float GrnTexel = tex2D(DiffuseSampler, float2(CoordX.y, CoordY.y)).g;
	float BluTexel = tex2D(DiffuseSampler, float2(CoordX.z, CoordY.z)).b;

	float ChromaSubsampleWidth = RawWidth / SubsampleLength;
	float LumaSubsampleWidth = RawWidth / SubsampleLength; // For now
	float3 YFrac = frac((CoordY * RawHeight) / CrawlHeight);
	float3 CrawlPercent = frac(CurrFrame / CrawlRate + (YFrac - frac(YFrac * CrawlWidth) / CrawlWidth)) * CrawlWidth;
	float3 ChromaCoordX = Input.CoordX + CrawlPercent / RawWidth;
	float3 ChannelFrac = frac((ChromaCoordX + PincushionCurveX) * ChromaSubsampleWidth);
	ChromaCoordX = ((ChromaCoordX + PincushionCurveX) * ChromaSubsampleWidth - ChannelFrac) / ChromaSubsampleWidth + ChannelFrac / (ChromaSubsampleWidth * 2.0f);
	float3 ChromaCoordY = Input.CoordY + PincushionCurveY;

	float3 BaseTexelRight = tex2D(DiffuseSampler, BaseCoord + float2(1.0f / TargetWidth, 0.0f)).rgb;
	float3 BaseTexelUpper = tex2D(DiffuseSampler, BaseCoord + float2(0.0f, 1.0f / TargetHeight)).rgb;

	float3 UpperDifference = abs(BaseTexel.rgb - BaseTexelUpper);
	float3 RightDifference = abs(BaseTexel.rgb - BaseTexelRight);
	float3 EdgeDifference = (UpperDifference + RightDifference) * EdgeDetectScale;

	float RedChromaTexel = tex2D(DiffuseSampler, float2(ChromaCoordX.x, ChromaCoordY.x)).r;
	float GrnChromaTexel = tex2D(DiffuseSampler, float2(ChromaCoordX.y, ChromaCoordY.y)).g;
	float BluChromaTexel = tex2D(DiffuseSampler, float2(ChromaCoordX.z, ChromaCoordY.z)).b;

	float3 ChromaTexel = float3(RedChromaTexel, GrnChromaTexel, BluChromaTexel);
	float3 LumaTexel = float3(RedTexel, GrnTexel, BluTexel);

	// -- RGB Tint & Shift --
	float ShiftedRed = dot(LumaTexel, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrn = dot(LumaTexel, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBlu = dot(LumaTexel, float3(BluFromRed, BluFromGrn, BluFromBlu));
	float ShiftedRedChroma = dot(ChromaTexel, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrnChroma = dot(ChromaTexel, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBluChroma = dot(ChromaTexel, float3(BluFromRed, BluFromGrn, BluFromBlu));
	float EdgeRed = dot(EdgeDifference, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float EdgeGrn = dot(EdgeDifference, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float EdgeBlu = dot(EdgeDifference, float3(BluFromRed, BluFromGrn, BluFromBlu));
	
	// -- RGB Offset & Scale --
	float3 OutTexelLuma = float3(ShiftedRed, ShiftedGrn, ShiftedBlu) * float3(RedScale, GrnScale, BluScale) + float3(RedOffset, GrnOffset, BluOffset);
	float3 OutTexelChroma = float3(ShiftedRedChroma, ShiftedGrnChroma, ShiftedBluChroma) * float3(RedScale, GrnScale, BluScale) + float3(RedOffset, GrnOffset, BluOffset);
	
	// -- Saturation --
	float OutLumaLuma = dot(OutTexelLuma, float3(0.3f, 0.59f, 0.11f));
	float3 OutChromaLuma = OutTexelLuma - float3(OutLumaLuma, OutLumaLuma, OutLumaLuma);
	float3 SaturatedLuma = OutLumaLuma + OutChromaLuma * Saturation;
	
	float OutLumaChroma = dot(OutTexelChroma, float3(0.3f, 0.59f, 0.11f));
	float3 OutChromaChroma = OutTexelChroma  - float3(OutLumaChroma, OutLumaChroma, OutLumaChroma);
	float3 SaturatedChroma = OutLumaChroma + OutChromaChroma * Saturation;
	
	// -- YIQ Convolution --
	float Y = dot(SaturatedLuma, float3(0.299f, 0.587f, 0.114f));
	float I = dot(SaturatedChroma, float3(0.595716f, -0.274453f, -0.321263f));
	float Q = dot(SaturatedChroma, float3(0.211456f, -0.522591f, 0.311135f));
	Y = dot(float3(Y, I, Q), float3(YfromY, YfromI, YfromQ));
	I = dot(float3(Y, I, Q), float3(IfromY, IfromI, IfromQ));
	Q = dot(float3(Y, I, Q), float3(QfromY, QfromI, QfromQ));
	float3 OutYIQ = float3(Y, I, Q) * float3(YScale, IScale, QScale) + float3(YOffset, IOffset, QOffset);
	float3 OutRGB = float3(dot(OutYIQ, float3(1.0f, 0.9563f, 0.6210f)), dot(OutYIQ, float3(1.0f, -0.2721f, -0.6474f)), dot(OutYIQ, float3(1.0f, -1.1070f, 1.7046f)));

	OutRGB = lerp(OutRGB, float3(EdgeRed, EdgeGrn, EdgeBlu), EdgeToBaseRatio);

	float3 Power = float3(RedPower, GrnPower, BluPower);
	OutRGB = pow(OutRGB, Power);

	// -- Color Compression (increasing the floor of the signal without affecting the ceiling) --
	float3 Floor = float3(RedFloor, GrnFloor, BluFloor);
	OutRGB = Floor + (1.0f - Floor) * OutRGB;

	// -- Scanline Simulation --
	float3 ScanBrightness = lerp(1.0f, abs(sin(((CoordY * RawHeight * ScanlineScale) * PI + ScanlineOffset * RawHeight))) * ScanlineBrightScale + ScanlineBrightOffset, ScanlineAmount);
	float3 Scanned = OutRGB * ScanBrightness;

	float2 ShadowCoord = BaseCoord * float2(RawWidth, RawHeight);
	float ShadowCoordX = frac(ShadowCoord.x / ShadowPixelSizeX) * ShadowU + 1.0f / ShadowWidth;
	float ShadowCoordY = frac(ShadowCoord.y / ShadowPixelSizeY) * ShadowV + 1.0f / ShadowHeight;
	float3 ShadowTexel = lerp(1.0f, tex2D(ShadowSampler, float2(ShadowCoordX, ShadowCoordY)), UseShadow);
	
	// -- Final Pixel --
	float4 Output = lerp(Input.Color, float4(Scanned * lerp(1.0f, ShadowTexel * 1.25f, ShadowBrightness), BaseTexel.a) * Input.Color, Input.ExtraInfo.x);
	
	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Effect
//-----------------------------------------------------------------------------

technique TestTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		//Sampler[0] = <DiffuseSampler>;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
