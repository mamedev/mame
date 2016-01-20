// license:BSD-3-Clause
// copyright-holders:ImJezze
//-----------------------------------------------------------------------------
// Distortion Effect
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
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
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
// Distortion Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims; // size of the window or fullscreen
uniform float2 TargetDims; // size of the target surface
uniform float2 QuadDims; // size of the screen quad

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.Color = Input.Color;

	Output.TexCoord = Input.Position.xy / ScreenDims;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float CurvatureAmount = 0.0f;
uniform float RoundCornerAmount = 0.0f;
uniform float SmoothBorderAmount = 0.0f;
uniform float VignettingAmount = 0.0f;
uniform float ReflectionAmount = 0.0f;

uniform bool OrientationSwapXY = false; // false landscape, true portrait for default screen orientation
uniform bool RotationSwapXY = false; // swapped default screen orientation due to screen rotation
uniform int RotationType = 0; // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°

float2 GetRatioCorrection()
{
	float ScreenRatio = ScreenDims.x / ScreenDims.y;
	float QuadRatio = QuadDims.x / QuadDims.y;
	float ScreenQuadRatio = QuadRatio / ScreenRatio;

	return ScreenQuadRatio > 1.0f
		? float2(1.0, 1.0f / ScreenQuadRatio)
		: float2(ScreenQuadRatio, 1.0);
}

float GetNoiseFactor(float3 n, float random)
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

	// normalized screen quad ratio
	float2 QuadRatio = float2 (1.0f, QuadDims.y / QuadDims.x);

	// upper right quadrant
	float2 spotOffset =
		RotationType == 1 // 90°
			? float2(-0.25f, -0.25f)
			: RotationType == 2 // 180°
				? float2(0.25f, -0.25f)
				: RotationType == 3 // 270°
					? float2(0.25f, 0.25f)
					: float2(-0.25f, 0.25f);

	float2 SpotCoord = coord;
	SpotCoord += spotOffset * RatioCorrection;
	SpotCoord *= QuadRatio;
	SpotCoord /= RatioCorrection;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75f;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = amount * normalizedSigmoid(Spot, 0.75);

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0f;

	return saturate(SigmoidSpot);
}

float GetRoundCornerFactor(float2 coord, float radiusAmount, float smoothAmount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(QuadDims.x, QuadDims.y) * 0.5;
	float radius = range * max(radiusAmount, 0.0025f);
	float smooth = 1.0 / (range * max(smoothAmount, 0.0025f));

	// compute box
	float box = roundBox(ScreenDims * (coord * 2.0f), ScreenDims * RatioCorrection, radius);

	// apply smooth
	box *= smooth;
	box += 1.0f - pow(smooth * 0.5f, 0.5f);

	float border = smoothstep(1.0f, 0.0f, box);

	return saturate(border);
}

// www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
float2 GetDistortedCoords(float2 centerCoord, float amount)
{
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

float2 GetCoords(float2 coord, float distortionAmount)
{
	float2 RatioCorrection = GetRatioCorrection();

	// center coordinates
	coord -= 0.5f;

	// apply ratio difference between screen and quad
	coord /= RatioCorrection;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount);

	// revert ratio difference between screen and quad
	coord *= RatioCorrection;

	// un-center coordinates
	coord += 0.5f;

	return coord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 TexCoord = Input.TexCoord;
	float2 BaseCoord = TexCoord;

	// Screen Curvature
	BaseCoord = GetCoords(BaseCoord, CurvatureAmount * 0.25f); // reduced amount

	float2 BaseCoordCentered = BaseCoord;
	BaseCoordCentered -= 0.5f;

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	// Vignetting Simulation
	float2 VignetteCoord = BaseCoordCentered;

	float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
	BaseColor.rgb *= VignetteFactor;

	// Light Reflection Simulation
	float3 LightColor = float3(1.0f, 0.90f, 0.80f); // color temperature 5.000 Kelvin

	float2 SpotCoord = BaseCoordCentered;
	float2 NoiseCoord = BaseCoordCentered;

	float SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount);
	float NoiseFactor = GetNoiseFactor(SpotAddend, random(NoiseCoord));
	BaseColor.rgb += SpotAddend * NoiseFactor * LightColor;

	// Round Corners Simulation
	float2 RoundCornerCoord = BaseCoordCentered;

	float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerAmount, SmoothBorderAmount);
	BaseColor.rgb *= roundCornerFactor;

	return BaseColor;
}

//-----------------------------------------------------------------------------
// Distortion Effect
//-----------------------------------------------------------------------------

technique DistortionTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader = compile ps_3_0 ps_main();
	}
}