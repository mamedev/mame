$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
//-----------------------------------------------------------------------------

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

SAMPLER2D(DiffuseSampler, 0);

//-----------------------------------------------------------------------------
// Uniforms
//-----------------------------------------------------------------------------

uniform vec4 u_screen_dims;

uniform vec4 u_curvature;
uniform vec4 u_round_corner;
uniform vec4 u_smooth_border;
uniform vec4 u_vignetting;
uniform vec4 u_reflection;

uniform vec4 u_rotation_type; // TODO

#define CurvatureAmount u_curvature.x
#define RoundCornerAmount u_round_corner.x
#define SmoothBorderAmount u_smooth_border.x
#define VignettingAmount u_vignetting.x
#define ReflectionAmount u_reflection.x

#define RotationType u_rotation_type.x // TODO

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// Holy fuck the number of functions...

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

vec2 GetRatioCorrection()
{
	float ScreenQuadRatio = 1.0;

	return ScreenQuadRatio > 1.0f
		? vec2(1.0, 1.0f / ScreenQuadRatio)
		: vec2(ScreenQuadRatio, 1.0);
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
	vec2 RatioCorrection = GetRatioCorrection();

	// normalized screen quad ratio
	vec2 QuadRatio = vec2(1.0, u_screen_dims.y / u_screen_dims.x);

	// normalized screen quad ratio
	// upper right quadrant
	vec2 spotOffset = vec2(-0.25, 0.25); // 0 degrees
	if (RotationType == 1.0)
		spotOffset = vec2(-0.25, -0.25); // 90 degrees
	if (RotationType == 2.0)
		spotOffset = vec2(0.25, -0.25); // 180 degrees
	if (RotationType == 3.0)
		spotOffset = vec2(0.25, 0.25); // 270 degrees

	vec2 SpotCoord = (coord + spotOffset * RatioCorrection) / RatioCorrection;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = amount * normalizedSigmoid(Spot, 0.75);

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0;

	return saturate(SigmoidSpot);
}

float GetRoundCornerFactor(vec2 coord, float radiusAmount, float smoothAmount)
{
	vec2 RatioCorrection = GetRatioCorrection();

	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(u_screen_dims.x, u_screen_dims.y) * 0.5;
	float radius = range * max(radiusAmount, 0.0025);
	float smooth_val = 1.0 / (range * max(smoothAmount, 0.0025));

	// compute box
	float box = roundBox(u_screen_dims.xy * (coord * 2.0f), u_screen_dims.xy * RatioCorrection, radius);

	// apply smooth
	box *= smooth_val;
	box += 1.0 - pow(smooth_val * 0.5, 0.5);

	float border = smoothstep(1.0, 0.0, box);

	return saturate(border);
}

// www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
vec2 GetDistortedCoords(vec2 centerCoord, float amount)
{
	// lens distortion coefficient
	float k = amount;

	// cubic distortion value
	float kcube = amount * 2.0;

	// compute cubic distortion factor
	float r2 = centerCoord.x * centerCoord.x + centerCoord.y * centerCoord.y;
	float f = kcube == 0.0 ? 1.0 + r2 * k : 1.0 + r2 * (k + kcube * sqrt(r2));

   	// fit screen bounds
	f /= 1.0 + amount * 0.5;

	// apply cubic distortion factor
   	centerCoord *= f;

	return centerCoord;
}

vec2 GetCoords(vec2 coord, float distortionAmount)
{
	vec2 RatioCorrection = GetRatioCorrection();

	// center coordinates
	coord -= 0.5;

	// apply ratio difference between screen and quad
	coord /= RatioCorrection;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount);

	// revert ratio difference between screen and quad
	coord *= RatioCorrection;

	// un-center coordinates
	coord += 0.5;

	return coord;
}

//-----------------------------------------------------------------------------
// Distortion Pixel Shader
//-----------------------------------------------------------------------------

void main()
{
	// Screen Curvature
	vec2 BaseCoord = GetCoords(v_texcoord0, CurvatureAmount * 0.25); // reduced amount

	vec2 BaseCoordCentered = BaseCoord;
	BaseCoordCentered -= 0.5;

	// Color
	vec4 BaseColor = texture2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0;

	// Clamp
	if (BaseCoord.x > 1.0 || BaseCoord.y > 1.0 || BaseCoord.x < 0.0 || BaseCoord.y < 0.0)
		BaseColor.rgb = vec3(0.0, 0.0, 0.0);
	
	// Vignetting Simulation
	vec2 VignetteCoord = BaseCoordCentered;

	float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
	BaseColor.rgb *= VignetteFactor;

	// Light Reflection Simulation
	vec3 LightColor = vec3(1.0, 0.90, 0.80); // color temperature 5.000 Kelvin

	vec2 SpotCoord = BaseCoordCentered;
	vec2 NoiseCoord = BaseCoordCentered;

	float SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount);
	float NoiseFactor = GetNoiseFactor(SpotAddend, rand(NoiseCoord));
	BaseColor.rgb += SpotAddend * NoiseFactor * LightColor;

	// Round Corners Simulation
	vec2 RoundCornerCoord = BaseCoordCentered;

	float roundCornerFactor = GetRoundCornerFactor(RoundCornerCoord, RoundCornerAmount, SmoothBorderAmount);
	BaseColor.rgb *= roundCornerFactor;

	gl_FragColor = BaseColor;
}
