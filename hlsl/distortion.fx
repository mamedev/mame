// license:BSD-3-Clause
// copyright-holders:ImJezze
//-----------------------------------------------------------------------------
// Distortion Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
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

uniform bool VectorScreen;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.Color = Input.Color;

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	return Output;
}

//-----------------------------------------------------------------------------
// Distortion Pixel Shader
//-----------------------------------------------------------------------------

uniform float CurvatureAmount = 0.0f;
uniform float RoundCornerAmount = 0.0f;
uniform float SmoothBorderAmount = 0.0f;
uniform float VignettingAmount = 0.0f;
uniform float ReflectionAmount = 0.0f;

uniform bool SwapXY = false;
uniform int RotationType = 0; // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°

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
	float2 SpotCoord = coord;

	// hack for vector screen
	if (VectorScreen)
	{
		// upper right quadrant
		float2 spotOffset =
			RotationType == 1 // 90°
				? float2(-0.25f, -0.25f)
				: RotationType == 2 // 180°
					? float2(0.25f, -0.25f)
					: RotationType == 3 // 270° else 0°
						? float2(0.25f, 0.25f)
						: float2(-0.25f, 0.25f);

		// normalized screen canvas ratio
		float2 CanvasRatio = SwapXY
			? float2(QuadDims.x / QuadDims.y, 1.0f)
			: float2(1.0f, QuadDims.y / QuadDims.x);

		SpotCoord += spotOffset;
		SpotCoord *= CanvasRatio;
	}
	else
	{
		// upper right quadrant
		float2 spotOffset = float2(-0.25f, 0.25f);

		// normalized screen canvas ratio
		float2 CanvasRatio = SwapXY 
			? float2(1.0f, QuadDims.x / QuadDims.y)
			: float2(1.0f, QuadDims.y / QuadDims.x);

		SpotCoord += spotOffset;
		SpotCoord *= CanvasRatio;
	}

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
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float2 quadDims = QuadDims;
	quadDims = !VectorScreen && SwapXY
		? quadDims.yx
		: quadDims.xy;

	float range = min(quadDims.x, quadDims.y) * 0.5;
	float radius = range * max(radiusAmount, 0.0025f);
	float smooth = 1.0 / (range * max(smoothAmount, 0.0025f));

	// compute box
	float box = roundBox(quadDims * (coord * 2.0f), quadDims, radius);

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
	// center coordinates
	coord -= 0.5f;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount);

	// un-center coordinates
	coord += 0.5f;

	return coord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	// Screen Curvature
	float2 TexCoord = GetCoords(Input.TexCoord, CurvatureAmount * 0.25f); // reduced amount

	float2 TexCoordCentered = TexCoord;
	TexCoordCentered -= 0.5f;

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, TexCoord);
	BaseColor.a = 1.0f;

	// Vignetting Simulation
	float2 VignetteCoord = TexCoordCentered;

	float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
	BaseColor.rgb *= VignetteFactor;

	// Light Reflection Simulation
	float3 LightColor = float3(1.0f, 0.90f, 0.80f); // color temperature 5.000 Kelvin

	float2 SpotCoord = TexCoordCentered;
	float2 NoiseCoord = TexCoordCentered;

	float SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount);
	float NoiseFactor = GetNoiseFactor(SpotAddend, random(NoiseCoord));
	BaseColor.rgb += SpotAddend * NoiseFactor * LightColor;

	// Round Corners Simulation
	float2 RoundCornerCoord = TexCoordCentered;

	float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerAmount, SmoothBorderAmount);
	BaseColor.rgb *= roundCornerFactor;

	return BaseColor;
}

//-----------------------------------------------------------------------------
// Distortion Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader = compile ps_3_0 ps_main();
	}
}