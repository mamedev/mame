// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline & Shadowmask Effect
//-----------------------------------------------------------------------------

texture DiffuseTexture;

sampler DiffuseSampler = sampler_state
{
	Texture = <DiffuseTexture>;
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
	Texture = <ShadowTexture>;
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

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 ScreenCoord : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 ScreenCoord : TEXCOORD1;
};

bool xor(bool a, bool b)
{
	return (a || b) && !(a && b);
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims; // size of the window or fullscreen
uniform float2 ScreenRatio = float2(1.0f, 3.0f / 4.0f);

uniform float2 SourceDims; // size of the texture in power-of-two size
uniform float2 SourceRect; // size of the uv rectangle

uniform float2 ShadowDims = float2(32.0f, 32.0f); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0f, 0.0f);

uniform float2 Prescale = float2(8.0f, 8.0f);

uniform bool OrientationSwapXY = false; // false landscape, true portrait for default screen orientation
uniform bool RotationSwapXY = false; // swapped default screen orientation due to screen rotation
uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures 

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float4 Position = Input.Position;
	Position.xy *= (ScreenDims + 1.0f) / ScreenDims;
	Position.xy -= 0.5f / ScreenDims;

	float2 shadowUVOffset = ShadowUVOffset;
	shadowUVOffset = xor(OrientationSwapXY, RotationSwapXY)
		? shadowUVOffset.yx
		: shadowUVOffset.xy;

	// todo: calculate offset
	float2 ScreenCoordPrescaleOffset = 0.0f;
	ScreenCoordPrescaleOffset += shadowUVOffset;

	Output.ScreenCoord = Position.xy;
	Output.ScreenCoord += ScreenCoordPrescaleOffset;

	Output.Position = float4(Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float ScanlineAlpha = 1.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineHeight = 1.0f;

uniform float CurvatureAmount = 0.0f;
uniform float RoundCornerAmount = 0.0f;
uniform float VignettingAmount = 0.0f;
uniform float ReflectionAmount = 0.0f;

uniform float ShadowAlpha = 0.0f;
uniform float2 ShadowCount = float2(6.0f, 6.0f);
uniform float2 ShadowUV = float2(0.25f, 0.25f);

uniform float3 Power = float3(1.0f, 1.0f, 1.0f);
uniform float3 Floor = float3(0.0f, 0.0f, 0.0f);

static const float Epsilon = 1.0e-7f;
static const float PI = 3.1415927f;
static const float E = 2.7182817f;
static const float Gelfond = 23.140692f; // e^pi (Gelfond constant)
static const float GelfondSchneider = 2.6651442f; // 2^sqrt(2) (Gelfond–Schneider constant)

float nextPowerOfTwo(float n)
{
	return pow(2, floor(log2(n) / log2(2)) + 1);
}

// www.stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader/
float random(float2 seed)
{
	// irrationals for pseudo randomness
	float2 i = float2(Gelfond, GelfondSchneider);

	return frac(cos(dot(seed, i)) * 123456.0f);
}

// www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
float normalizedSigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - abs(n) * 2.0f * k + 1);
}

float GetNoiseFactor(float n, float random)
{
	// smaller n become more noisy
	return 1.0f + random * max(0.0f, 0.25f * pow(E, -4 * n));
}

float GetVignetteFactor(float2 coord, float amount)
{
	float2 VignetteCoord = coord;

	float VignetteBlur = amount * 2.0f;

	// 0.5 full screen fitting circle
	float VignetteRadius = 1.0f - amount * 0.5f;
	float Vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteBlur, length(VignetteCoord));

	// reduce strength to 50%
	Vignette = lerp(1.0, 1.0 * Vignette, 0.5f);

	return saturate(Vignette);
}

float GetSpotAddend(float2 coord, float amount)
{
	float2 SpotCoord = coord;
	SpotCoord += OrientationSwapXY 
		? float2(-0.25f, -0.25f) * ScreenRatio  // upper right quadrant
		: float2(-0.25f,  0.25f) * ScreenRatio; // upper right quadrant

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75f;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = normalizedSigmoid(Spot, 0.75) * amount;

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0f;

	return saturate(SigmoidSpot);
}

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float RoundBox(float2 p, float2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0f)) - r;
}

float GetRoundCornerFactor(float2 coord, float amount)
{
	float2 HalfSourceDims = SourceDims * 0.5f;
	float2 SourceTexelDims = 1.0f / SourceDims;

	// hint: roundness correction (base on the default ratio of 4:3)
	float2 RoundCoord = coord;
	RoundCoord -= SourceTexelDims;
	RoundCoord *= SourceTexelDims + 1.0f;
	RoundCoord *= SourceDims / ScreenRatio;

	float radius = amount * 50.0f;

	// compute box
	float box = RoundBox(RoundCoord.xy, HalfSourceDims / ScreenRatio, radius);

	float solidBorder = smoothstep(1.0f, 0.5f, box);

	// apply blur
	float blur = 1.0f / max(2.0f, amount * 100.0f); // blur amount
	box *= blur * 2.0f; // blur stength
	box += 1.0f - pow(blur * 0.5f, 0.5f); // blur offset

	float blurBorder = smoothstep(1.0f, 0.5f, box);

	float border = solidBorder * (blurBorder + 0.5f);

	return saturate(border);
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenTexelDims = 1.0f / ScreenDims;
	float2 SourceTexelDims = 1.0f / SourceDims;

	float2 UsedArea = 1.0f / SourceRect;
	float2 HalfRect = SourceRect * 0.5f;

	// Screen Curvature
	float2 CurvatureUnitCoord = 
		  Input.TexCoord 
		* UsedArea * 2.0f -
		  1.0f;
	float2 CurvatureCurve =
		  CurvatureUnitCoord
		* pow(length(CurvatureUnitCoord), 2.0f)
		/ pow(length(UsedArea), 2.0f)
		* CurvatureAmount * 0.25f; // reduced curvature
	float2 CurvatureZoom = 
		  1.0f - 
		  UsedArea * 2.0f
		/ pow(length(UsedArea), 2.0f)
		* CurvatureAmount * 0.25f; // reduced curvature

	float2 ScreenCoord = Input.ScreenCoord / ScreenDims;
	ScreenCoord -= HalfRect;
	ScreenCoord *= CurvatureZoom; // zoom
	ScreenCoord += HalfRect;
	ScreenCoord += CurvatureCurve; // distortion

	float2 BaseCoord = Input.TexCoord;
	BaseCoord -= HalfRect;
	BaseCoord *= CurvatureZoom; // zoom
	BaseCoord += HalfRect;
	BaseCoord += CurvatureCurve; // distortion

	float2 BaseCoordCentered = Input.TexCoord;
	BaseCoordCentered -= HalfRect;
	BaseCoordCentered *= CurvatureZoom; // zoom
	BaseCoordCentered += CurvatureCurve; // distortion

	// float2 BaseAreaRatioCoord = BaseCoord;
	// BaseAreaRatioCoord *= UsedArea * ScreenRatio;

	float2 BaseAreaRatioCoordCentered = BaseCoordCentered;
	BaseAreaRatioCoordCentered *= UsedArea * ScreenRatio;

	// float2 BaseAreaCoord = BaseCoord;
	// BaseAreaCoord *= UsedArea;

	float2 BaseAreaCoordCentered = BaseCoordCentered;
	BaseAreaCoordCentered *= UsedArea;

	// // Alpha Clipping (round corners applies smoother clipping when screen is curved)
	// clip((BaseCoord < SourceTexelDims) ? -1 : 1);
	// clip((BaseCoord > SourceRect + SourceTexelDims) ? -1 : 1);

	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	// Vignetting Simulation (may affect bloom)
	float2 VignetteCoord = BaseAreaRatioCoordCentered;

	float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
	BaseColor.rgb *= VignetteFactor;

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		float2 shadowDims = ShadowDims;
		shadowDims = xor(OrientationSwapXY, RotationSwapXY)
			? shadowDims.yx
			: shadowDims.xy;

		float2 shadowUV = ShadowUV;
		// shadowUV = xor(OrientationSwapXY, RotationSwapXY)
			// ? shadowUV.yx
			// : shadowUV.xy;

		float2 screenCoord = ScreenCoord;
		screenCoord = xor(OrientationSwapXY, RotationSwapXY)
			? screenCoord.yx
			: screenCoord.xy;

		float2 shadowCount = ShadowCount;
		shadowCount = xor(OrientationSwapXY, RotationSwapXY)
			? shadowCount.yx
			: shadowCount.xy;

		float2 shadowTile = (ScreenTexelDims * shadowCount);
		shadowTile = xor(OrientationSwapXY, RotationSwapXY)
			? shadowTile.yx
			: shadowTile.xy;

		float2 ShadowFrac = frac(screenCoord / shadowTile);
		float2 ShadowCoord = (ShadowFrac * shadowUV);
		ShadowCoord += 0.5f / shadowDims; // half texel offset
		// ShadowCoord = xor(OrientationSwapXY, RotationSwapXY)
			// ? ShadowCoord.yx
			// : ShadowCoord.xy;

		float3 ShadowColor = tex2D(ShadowSampler, ShadowCoord).rgb;
		ShadowColor = lerp(1.0f, ShadowColor, ShadowAlpha);

		BaseColor.rgb *= ShadowColor;
	}

	// Color Compression (may not affect bloom)
	if (!PrepareBloom)
	{
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = Floor + (1.0f - Floor) * BaseColor.rgb;
	}

	// Color Power (may affect bloom)
	BaseColor.r = pow(BaseColor.r, Power.r);
	BaseColor.g = pow(BaseColor.g, Power.g);
	BaseColor.b = pow(BaseColor.b, Power.b);

	// Scanline Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		// todo: there is an offset which can be noticed at lower prescale in high-resolution
		float2 ScanlinePrescaleOffset = 0.0f;

		float InnerSine = BaseCoordCentered.y * ScanlineScale * SourceDims.y;
		float ScanJitter = ScanlineOffset * SourceDims.y;
		float ScanBrightMod = sin(InnerSine * PI + ScanJitter + ScanlinePrescaleOffset);
		float3 ScanColor = lerp(1.0f, (pow(ScanBrightMod * ScanBrightMod, ScanlineHeight) * ScanlineBrightScale + 1.0f + ScanlineBrightOffset) * 0.5f, ScanlineAlpha);

		BaseColor.rgb *= ScanColor;
	}

	// Output
	float4 Output = BaseColor * Input.Color;
	Output.a = 1.0f;

	// Light Reflection Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		float3 LightColor = float3(1.0f, 0.90f, 0.80f);

		float2 SpotCoord = BaseAreaRatioCoordCentered;
		float2 NoiseCoord = BaseAreaRatioCoordCentered;

		float SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount);
		float NoiseFactor = GetNoiseFactor(SpotAddend, random(NoiseCoord));
		Output.rgb += SpotAddend * NoiseFactor * LightColor;
	}

	// Round Corners Simulation (may affect bloom)
	float2 RoundCornerCoord = BaseAreaCoordCentered;

	float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerAmount);
	Output.rgb *= roundCornerFactor;

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
		PixelShader = compile ps_3_0 ps_main();
	}
}