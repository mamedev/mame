//-----------------------------------------------------------------------------
// Scanline & Shadowmask Effect
//-----------------------------------------------------------------------------

texture DiffuseTexture;

sampler DiffuseSampler = sampler_state
{
	Texture   = <DiffuseTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

texture ShadowTexture;

sampler ShadowSampler = sampler_state
{
	Texture   = <ShadowTexture>;
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
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 SourceRect;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.TexCoord = Input.TexCoord + 0.5f / SourceDims;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float PI = 3.14159265f;

uniform float PincushionAmount = 0.00f;
uniform float CurvatureAmount = 0.08f;

uniform float ScanlineAlpha = 1.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineHeight = 0.5f;

uniform float ShadowAlpha = 0.0f;
uniform float2 ShadowCount = float2(320.0f, 240.0f);
uniform float2 ShadowUV = float2(0.375f, 0.375f);
uniform float2 ShadowDims = float2(8.0f, 8.0f);

uniform float3 Power = float3(1.0f, 1.0f, 1.0f);
uniform float3 Floor = float3(0.0f, 0.0f, 0.0f);

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 UsedArea = 1.0f / SourceRect;
	float2 HalfRect = SourceRect * 0.5f;
	float2 R2 = 1.0f / pow(length(UsedArea), 2.0f);
	// -- Screen Pincushion Calculation --
	float2 PinUnitCoord = Input.TexCoord * UsedArea * 2.0f - 1.0f;
	float PincushionR2 = pow(length(PinUnitCoord), 2.0f) * R2;
	float2 PincushionCurve = PinUnitCoord * PincushionAmount * PincushionR2;
	float2 BaseCoord = Input.TexCoord;
	float2 ScanCoord = BaseCoord - 0.5f / ScreenDims;
	
	BaseCoord -= HalfRect;
	BaseCoord *= 1.0f - PincushionAmount * UsedArea * 0.2f; // Warning: Magic constant
	BaseCoord += HalfRect;
	BaseCoord += PincushionCurve;

	ScanCoord -= HalfRect;
	ScanCoord *= 1.0f - PincushionAmount * UsedArea * 0.2f; // Warning: Magic constant
	ScanCoord += HalfRect;
	ScanCoord += PincushionCurve;

	float2 CurveClipUnitCoord = Input.TexCoord * UsedArea * 2.0f - 1.0f;
	float CurvatureClipR2 = pow(length(CurveClipUnitCoord), 2.0f) * R2;
	float2 CurvatureClipCurve = CurveClipUnitCoord * CurvatureAmount * CurvatureClipR2;
	float2 ScreenClipCoord = Input.TexCoord;
	ScreenClipCoord -= HalfRect;
	ScreenClipCoord *= 1.0f - CurvatureAmount * UsedArea * 0.2f; // Warning: Magic constant
	ScreenClipCoord += HalfRect;
	ScreenClipCoord += CurvatureClipCurve;

	// RGB Pincushion Calculation
	float3 PincushionCurveX = PinUnitCoord.x * PincushionAmount * PincushionR2;
	float3 PincushionCurveY = PinUnitCoord.y * PincushionAmount * PincushionR2;

	float4 BaseTexel = tex2D(DiffuseSampler, BaseCoord);

	// -- Alpha Clipping (1px border in drawd3d does not work for some reason) --
	clip((BaseCoord < 1.0f / SourceDims) ? -1 : 1);
	clip((BaseCoord > (SourceRect + 1.0f / SourceDims)) ? -1 : 1);

	// -- Scanline Simulation --
	float InnerSine = ScanCoord.y * SourceDims.y * ScanlineScale;
	float ScanBrightMod = sin(InnerSine * PI + ScanlineOffset * SourceDims.y);
	float3 ScanBrightness = lerp(1.0f, (pow(ScanBrightMod * ScanBrightMod, ScanlineHeight) * ScanlineBrightScale + 1.0f) * 0.5f, ScanlineAlpha);
	float3 Scanned = BaseTexel.rgb * ScanBrightness;

	// -- Color Compression (increasing the floor of the signal without affecting the ceiling) --
	Scanned = Floor + (1.0f - Floor) * Scanned;

	// Shadow mask
	// Note: This is broken right now and needs fixed
	float2 ShadowFrac = frac(BaseCoord * ShadowCount);
	float2 ShadowCoord = ShadowFrac * ShadowUV + 0.5f / ShadowDims;
	float3 ShadowTexel = lerp(1.0f, tex2D(ShadowSampler, ShadowCoord).rgb, ShadowAlpha);
	
	// -- Final Pixel --
	float4 Output = float4(Scanned * ShadowTexel, BaseTexel.a) * Input.Color;
	
	Output.r = pow(Output.r, Power.r);
	Output.g = pow(Output.g, Power.g);
	Output.b = pow(Output.b, Power.b);
	Output.a = 1.0f;

	return Output;
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Effect
//-----------------------------------------------------------------------------

technique ScanMaskTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		//Sampler[0] = <DiffuseSampler>;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
