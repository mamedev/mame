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

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float Epsilon = 1.0e-7f;
static const float PI = 3.1415927f;
static const float E = 2.7182817f;
static const float Gelfond = 23.140692f; // e^pi (Gelfond constant)
static const float GelfondSchneider = 2.6651442f; // 2^sqrt(2) (Gelfond-Schneider constant)

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

bool xor(bool a, bool b)
{
	return (a || b) && !(a && b);
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

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(float2 p, float2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0f)) - r;
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims; // size of the window or fullscreen
uniform float2 SourceDims; // size of the texture in power-of-two size
uniform float2 SourceRect; // size of the uv rectangle
uniform float2 TargetDims; // size of the target surface
uniform float2 QuadDims; // size of the screen quad

uniform float2 ShadowDims = float2(32.0f, 32.0f); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0f, 0.0f);

uniform bool OrientationSwapXY = false; // false landscape, true portrait for default screen orientation
uniform bool RotationSwapXY = false; // swapped default screen orientation due to screen rotation
uniform int RotationType = 0; // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°

uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures 
uniform bool PrepareVector = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float2 shadowUVOffset = ShadowUVOffset;
	shadowUVOffset = xor(OrientationSwapXY, RotationSwapXY)
		? shadowUVOffset.yx
		: shadowUVOffset.xy;

	float2 ScreenCoordOffset = 0.0f;
	ScreenCoordOffset += shadowUVOffset;

	Output.ScreenCoord = Input.Position.xy;
	Output.ScreenCoord += ScreenCoordOffset;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = PrepareVector
		? Input.Position.xy / ScreenDims
		: Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenScale = float2(1.0f, 1.0f);
uniform float2 ScreenOffset = float2(0.0f, 0.0f);

uniform float ScanlineAlpha = 1.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineHeight = 1.0f;

uniform float CurvatureAmount = 1.0f;
uniform float RoundCornerAmount = 0.0f;
uniform float SmoothBorderAmount = 0.0f;
uniform float VignettingAmount = 0.0f;
uniform float ReflectionAmount = 0.0f;

uniform float ShadowAlpha = 0.0f;
uniform float2 ShadowCount = float2(6.0f, 6.0f);
uniform float2 ShadowUV = float2(0.25f, 0.25f);

uniform float3 Power = float3(1.0f, 1.0f, 1.0f);
uniform float3 Floor = float3(0.0f, 0.0f, 0.0f);

float2 GetRatioCorrection()
{
	if (PrepareVector)
	{
		float ScreenRatio = ScreenDims.x / ScreenDims.y;
		float QuadRatio = QuadDims.x / QuadDims.y;
		float ScreenQuadRatio = QuadRatio / ScreenRatio;

		return ScreenQuadRatio > 1.0f
			? float2(1.0, 1.0f / ScreenQuadRatio)
			: float2(ScreenQuadRatio, 1.0);
	}
	else
	{
		return SourceRect;
	}
}

float GetNoiseFactor(float n, float random)
{
	// smaller n become more noisy
	return 1.0f + random * max(0.0f, 0.25f * pow(E, -8 * n));
}

float GetVignetteFactor(float2 coord, float amount)
{
	float2 VignetteCoord = coord;

	float VignetteLength = length(VignetteCoord);
	float VignetteBlur = (amount * 0.75f) + 0.25;

	// 0.5 full screen fitting circle
	float VignetteRadius = 1.0f - (amount * 0.25f);
	float Vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteBlur, VignetteLength);

	return saturate(Vignette);
}

float GetSpotAddend(float2 coord, float amount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// normalized screen canvas ratio
	float2 CanvasRatio = PrepareVector
		? float2(1.0f, QuadDims.y / QuadDims.x)
		: float2(1.0f, xor(OrientationSwapXY, RotationSwapXY) 
			? QuadDims.x / QuadDims.y 
			: QuadDims.y / QuadDims.x);

	// upper right quadrant
	float2 spotOffset = PrepareVector
		? RotationType == 1 // 90°
			? float2(-0.25f, -0.25f)
			: RotationType == 2 // 180°
				? float2(0.25f, -0.25f)
				: RotationType == 3 // 270°
					? float2(0.25f, 0.25f)
					: float2(-0.25f, 0.25f)
		: OrientationSwapXY
			? float2(0.25f, 0.25f)
			: float2(-0.25f, 0.25f);

	float2 SpotCoord = coord;
	SpotCoord += spotOffset * RatioCorrection;
	SpotCoord *= CanvasRatio;
	SpotCoord /= RatioCorrection;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75f;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = normalizedSigmoid(Spot, 0.75) * amount;

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0f;

	return saturate(SigmoidSpot);
}

float GetRoundCornerFactor(float2 coord, float radiusAmount, float smoothAmount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float2 CanvasDims = PrepareVector
		? ScreenDims
		: xor(OrientationSwapXY, RotationSwapXY) 
			? QuadDims.yx / SourceRect 
			: QuadDims.xy / SourceRect;

	coord = PrepareVector
		? coord
		: coord - 1.0f / SourceDims; // alignment correction (raster graphics)

	float range = min(QuadDims.x, QuadDims.y) * 0.5;
	float radius = range * max(radiusAmount, 0.0025f);
	float smooth = 1.0 / (range * max(smoothAmount, 0.0025f));

	// compute box
	float box = roundBox(CanvasDims * (coord * 2.0f), CanvasDims * RatioCorrection, radius);

	// apply smooth
	box *= smooth;
	box += 1.0f - pow(smooth * 0.5f, 0.5f);

	float border = smoothstep(1.0f, 0.0f, box);

	return saturate(border);
}

// www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
float2 GetDistortedCoords(float2 centerCoord, float amount)
{
	amount *= 0.25f; // reduced amount

	// lens distortion coefficient
	float k = amount;

	// cubic distortion value
	float kcube = amount * 2.0f;

	// compute cubic distortion factor
	float r2 = centerCoord.x * centerCoord.x + centerCoord.y * centerCoord.y;
	float f = kcube == 0.0f
		? 1.0f + r2 * k
		: 1.0f + r2 * (k + kcube * sqrt(r2));

   	// fit screen bounds
	f /= 1.0f + amount * 0.5f;

	// apply cubic distortion factor
   	centerCoord *= f;

	return centerCoord;
}

float2 GetCoords(float2 coord, float2 centerOffset, float distortionAmount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// center coordinates
	coord -= centerOffset;

	// apply ratio difference between screen and quad
	coord /= RatioCorrection;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount);

	// revert ratio difference between screen and quad
	coord *= RatioCorrection;

	// un-center coordinates
	coord += centerOffset;

	return coord;
}

float2 GetAdjustedCoords(float2 coord, float2 centerOffset, float distortionAmount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// center coordinates
	coord -= centerOffset;

	// apply ratio difference between screen and quad
	coord /= RatioCorrection;

	// apply screen scale
	coord /= ScreenScale;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount);

	// revert ratio difference between screen and quad
	coord *= RatioCorrection;

	// un-center coordinates
	coord += centerOffset;

	// apply screen offset
	coord += (centerOffset * 2.0) * ScreenOffset;

	return coord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenTexelDims = 1.0f / ScreenDims;

	float2 HalfSourceRect = PrepareVector
		? float2(0.5f, 0.5f)
		: SourceRect * 0.5f;

	float2 ScreenCoord = Input.ScreenCoord / ScreenDims;
	ScreenCoord = GetCoords(ScreenCoord, float2(0.5f, 0.5f), CurvatureAmount);

	float2 DistortionCoord = Input.TexCoord;
	DistortionCoord = GetCoords(DistortionCoord, HalfSourceRect, CurvatureAmount);

	float2 BaseCoord = Input.TexCoord;
	BaseCoord = GetAdjustedCoords(BaseCoord, HalfSourceRect, CurvatureAmount);

	float2 DistortionCoordCentered = DistortionCoord;
	DistortionCoordCentered -= HalfSourceRect;

	float2 BaseCoordCentered = BaseCoord;
	BaseCoordCentered -= HalfSourceRect;

	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	if (BaseCoord.x < 0.0f || BaseCoord.y < 0.0f)
	{
		BaseColor.rgb = 0.0f;
	}

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
		// Scanline Simulation (disabled for vector)
		if (!PrepareVector)
		{
			float InnerSine = BaseCoord.y * ScanlineScale * SourceDims.y;
			float ScanJitter = ScanlineOffset * SourceDims.y;
			float ScanBrightMod = sin(InnerSine * PI + ScanJitter);
			float3 ScanColor = lerp(1.0f, (pow(ScanBrightMod * ScanBrightMod, ScanlineHeight) * ScanlineBrightScale + 1.0f + ScanlineBrightOffset) * 0.5f, ScanlineAlpha);

			BaseColor.rgb *= ScanColor;
		}
	}

	// Output
	float4 Output = PrepareVector
		? BaseColor * (Input.Color + float4(1.0f, 1.0f, 1.0f, 0.0f))
		: BaseColor * Input.Color;
	Output.a = 1.0f;

	// Vignetting Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		float2 VignetteCoord = DistortionCoordCentered;

		float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
		Output.rgb *= VignetteFactor;
	}

	// Light Reflection Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		float3 LightColor = float3(1.0f, 0.90f, 0.80f);

		float2 SpotCoord = DistortionCoordCentered;
		float2 NoiseCoord = DistortionCoordCentered;

		float SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount);
		float NoiseFactor = GetNoiseFactor(SpotAddend, random(NoiseCoord));
		Output.rgb += SpotAddend * NoiseFactor * LightColor;
	}

	// Round Corners Simulation (may affect bloom)
	float2 RoundCornerCoord = DistortionCoordCentered;

	float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerAmount, SmoothBorderAmount);
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