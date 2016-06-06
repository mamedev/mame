$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Distortion Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Autos
uniform vec4 u_swap_xy;
uniform vec4 u_screen_dims;
uniform vec4 u_target_dims;
uniform vec4 u_quad_dims;

// User-supplied
uniform vec4 u_distortion;
uniform vec4 u_cubic_distortion;
uniform vec4 u_distort_corner;
uniform vec4 u_round_corner;
uniform vec4 u_smooth_border;
uniform vec4 u_vignetting;
uniform vec4 u_reflection;

// Samplers
SAMPLER2D(s_tex, 0);

// Functions

// www.stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader/
float rand(vec2 seed)
{
	// irrationals for pseudo randomness
	vec2 i = vec2(23.140692, 2.6651442); // e^pi (Gelfond constant), 2^sqrt(2) (Gelfond-Schneider constant)

	return fract(cos(dot(seed, i)) * 123456.0);
}

// www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
float normalizedSigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - abs(n) * 2.0 * k + 1.0);
}

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(vec2 p, vec2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0)) - r;
}

float GetNoiseFactor(float n, float random)
{
	// smaller n become more noisy
	return 1.0 + random * max(0.0, 0.25 * pow(2.7182817, -8.0 * n));
}

float GetVignetteFactor(vec2 coord, float amount)
{
	vec2 VignetteCoord = coord;

	float VignetteLength = length(VignetteCoord);
	float VignetteBlur = (amount * 0.75) + 0.25;

	// 0.5 full screen fitting circle
	float VignetteRadius = 1.0 - (amount * 0.25);
	float Vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteBlur, VignetteLength);

	return saturate(Vignette);
}

float GetSpotAddend(vec2 coord, float amount)
{
	vec2 SpotCoord = coord;

	// upper right quadrant
	vec2 spotOffset = vec2(-0.25, 0.25);

	// normalized screen canvas ratio
	vec2 CanvasRatio = ((u_swap_xy.x > 0.0) ? vec2(1.0, u_quad_dims.x / u_quad_dims.y) : vec2(1.0, u_quad_dims.y / u_quad_dims.x));

	SpotCoord += spotOffset;
	SpotCoord *= CanvasRatio;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = amount * normalizedSigmoid(Spot, 0.75);

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0;

	return saturate(SigmoidSpot);
}

float GetRoundCornerFactor(vec2 coord, vec2 bounds, float radiusAmount, float smoothAmount)
{
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(bounds.x, bounds.y);
	float amountMinimum = range > 0.0f ? 1.0f / range : 0.0f;
	float radius = range * max(radiusAmount, amountMinimum);
	float smooth_val = 1.0f / (range * max(smoothAmount, amountMinimum * 3.0f));

	// compute box
	float box = roundBox(bounds * (coord * 2.0f), bounds, radius);

	// apply smooth
	box *= smooth_val;
	box += 1.0 - pow(smooth_val * 0.5, 0.5);

	float border = smoothstep(1.0, 0.0, box);

	return saturate(border);
}

// www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
vec2 GetDistortedCoords(vec2 centerCoord, float amount, float amountCube)
{
	// lens distortion coefficient
	float k = amount;

	// cubic distortion value
	float kcube = amountCube;

	// compute cubic distortion factor
	float r2 = centerCoord.x * centerCoord.x + centerCoord.y * centerCoord.y;
	float f = kcube == 0.0 ? 1.0 + r2 * k : 1.0 + r2 * (k + kcube * sqrt(r2));

   	// fit screen bounds
	f /= 1.0 + amount * 0.25 + amountCube * 0.125;

	// apply cubic distortion factor
   	centerCoord *= f;

	return centerCoord;
}

vec2 GetCoords(vec2 coord, float distortionAmount, float cubicDistortionAmount)
{
	// center coordinates
	coord -= 0.5;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount, cubicDistortionAmount);

	// un-center coordinates
	coord += 0.5;

	return coord;
}

// Shader

void main()
{
	float distortionAmount = u_distortion.x;
	float cubicDistortionAmount = u_cubic_distortion.x > 0.0
		? u_cubic_distortion.x * 1.1  // cubic distortion need to be a little higher to compensate the quartic distortion
		: u_cubic_distortion.x * 1.2; // negativ values even more

	vec2 TexelDims = vec2(1.0 / u_target_dims.x, 1.0 / u_target_dims.y);

	// Screen Curvature
	vec2 BaseCoord = GetCoords(v_texcoord0, distortionAmount, cubicDistortionAmount);

	// Corner Curvature
	vec2 CornerCoord = GetCoords(v_texcoord0, u_distort_corner.x, 0.0);

	vec2 BaseCoordCentered = BaseCoord;
	BaseCoordCentered -= 0.5;
	vec2 CornerCoordCentered = CornerCoord;
	CornerCoordCentered -= 0.5;

	// Color
	vec4 BaseColor = texture2D(s_tex, BaseCoord);

	// Clamp
	if (BaseCoord.x > 1.0 + TexelDims.x || BaseCoord.y > 1.0 + TexelDims.y || BaseCoord.x < 0.0 - TexelDims.x || BaseCoord.y < 0.0 - TexelDims.y)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	else
	{
		// Vignetting Simulation
		vec2 VignetteCoord = CornerCoordCentered;

		float VignetteFactor = GetVignetteFactor(VignetteCoord, u_vignetting.x);
		BaseColor.rgb *= VignetteFactor;

		// Light Reflection Simulation
		vec4 LightColor = vec4(1.0, 0.90, 0.80, 1.0); // color temperature 5.000 Kelvin

		vec2 SpotCoord = CornerCoordCentered;
		vec2 NoiseCoord = CornerCoordCentered;

		float SpotAddend = GetSpotAddend(SpotCoord, u_reflection.x);
		float NoiseFactor = GetNoiseFactor(SpotAddend, rand(NoiseCoord));
		BaseColor += SpotAddend * NoiseFactor * LightColor;

		// Round Corners Simulation
		vec2 RoundCornerCoord = CornerCoordCentered;
		vec2 RoundCornerBounds = (u_swap_xy.x > 0.0)
			? u_quad_dims.yx
			: u_quad_dims.xy;

		float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerBounds, u_round_corner.x * 0.5f, u_smooth_border.x * 0.5f);
		BaseColor.rgb *= roundCornerFactor;

		gl_FragColor = BaseColor;
	}
}
