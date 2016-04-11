$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Autos
uniform vec4 u_swap_xy;
uniform vec4 u_source_dims; // size of the guest machine
uniform vec4 u_quad_dims;
uniform vec4 u_screen_scale; // TODO: Hook up ScreenScale code-side
uniform vec4 u_screen_offset; // TODO: Hook up ScreenOffset code-side

// User-supplied
uniform vec4 u_scanline_alpha;
uniform vec4 u_scanline_scale;
uniform vec4 u_scanline_bright_scale;
uniform vec4 u_scanline_bright_offset;
uniform vec4 u_scanline_jitter;
uniform vec4 u_scanline_height;
uniform vec4 u_scanline_variation;
uniform vec4 u_shadow_alpha;
uniform vec4 u_shadow_count;
uniform vec4 u_shadow_uv;
uniform vec4 u_shadow_uv_offset;
uniform vec4 u_humbar_hertz_rate; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
uniform vec4 u_humbar_alpha;
uniform vec4 u_power;
uniform vec4 u_floor;

// Parametric
uniform vec4 u_time;
uniform vec4 u_jitter_amount;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_shadow, 1);

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

vec2 GetAdjustedCoords(vec2 coord, vec2 center_offset)
{
	// center coordinates
	coord -= center_offset;

	// apply screen scale
	//coord /= u_screen_scale.xy;

	// un-center coordinates
	coord += center_offset;

	// apply screen offset
	coord += (center_offset * 2.0) * u_screen_offset.xy;

	return coord;
}

// vector screen has the same quad texture coordinates for every screen orientation, raster screen differs
vec2 GetShadowCoord(vec2 QuadCoord, vec2 SourceCoord)
{
	vec2 QuadTexel = vec2(1.0, 1.0) / u_quad_dims.xy;

	vec2 canvasCoord = QuadCoord + u_shadow_uv_offset.xy / u_quad_dims.xy;

	vec2 shadowUV = u_shadow_uv.xy;
	vec2 shadowCount = u_shadow_count.xy;

	// swap x/y vector and raster in screen mode (not source mode)
	canvasCoord = u_swap_xy.x > 0.0
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y vector and raster in screen mode (not source mode)
	shadowCount = u_swap_xy.x > 0.0
		? shadowCount.yx
		: shadowCount.xy;

	vec2 shadowTile = QuadTexel * shadowCount;

	vec2 shadowFrac = fract(canvasCoord / shadowTile);

	// swap x/y raster in screen mode (not vector and not source mode)
	shadowFrac = u_swap_xy.x > 0.0
		? shadowFrac.yx
		: shadowFrac.xy;

	vec2 shadowCoord = (shadowFrac * shadowUV);

	return shadowCoord;
}

void main()
{
	vec2 BaseCoord = GetAdjustedCoords(v_texcoord0, vec2(0.5, 0.5));

	// Color
	vec4 BaseColor = texture2D(s_tex, BaseCoord);

	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0)
	{
		BaseColor.rgb = vec3(0.0, 0.0, 0.0);
	}

	// Mask Simulation
	if (u_shadow_alpha.x > 0.0)
	{
		vec2 ShadowCoord = GetShadowCoord(v_texcoord0.xy, v_texcoord0.xy);
		
		vec4 ShadowColor = texture2D(s_shadow, ShadowCoord);
		vec3 ShadowMaskColor = mix(vec3(1.0, 1.0, 1.0), ShadowColor.rgb, u_shadow_alpha.xxx);

		// apply shadow mask color
		BaseColor.rgb *= ShadowMaskColor;
	}

	// Color Compression
	// increasing the floor of the signal without affecting the ceiling
	BaseColor.rgb = u_floor.rgb + (vec3(1.0, 1.0, 1.0) - u_floor.rgb) * BaseColor.rgb;
	
	// Color Power
	BaseColor.r = pow(BaseColor.r, u_power.r);
	BaseColor.g = pow(BaseColor.g, u_power.g);
	BaseColor.b = pow(BaseColor.b, u_power.b);

	// Scanline Simulation
	if (u_scanline_alpha.x > 0.0f)
	{
		float BrightnessOffset = (u_scanline_bright_offset.x * u_scanline_alpha.x);
		float BrightnessScale = (u_scanline_bright_scale.x * u_scanline_alpha.x) + (1.0 - u_scanline_alpha.x);

		float ColorBrightness = 0.299 * BaseColor.r + 0.587 * BaseColor.g + 0.114 * BaseColor.b;

		float ScanCoord = v_texcoord0.y * u_source_dims.y * u_scanline_scale.x * 3.1415927;
		float ScanCoordJitter = u_scanline_jitter.x * u_jitter_amount.x * 1.618034;
		float ScanSine = sin(ScanCoord + ScanCoordJitter);
		float ScanlineWide = u_scanline_height.x + u_scanline_variation.x * max(1.0, u_scanline_height.x) * (1.0 - ColorBrightness);
		float ScanSineScaled = pow(ScanSine * ScanSine, ScanlineWide);
		float ScanBrightness = ScanSineScaled * BrightnessScale + BrightnessOffset * BrightnessScale;

		BaseColor.rgb *= mix(vec3(1.0, 1.0, 1.0), vec3(ScanBrightness, ScanBrightness, ScanBrightness), u_scanline_alpha.xxx);
	}

	// Hum Bar Simulation
	if (u_humbar_alpha.x > 0.0f)
	{
		float HumTimeStep = fract(u_time.x * 0.001);
		float HumBrightness = 1.0 - fract(BaseCoord.y + HumTimeStep) * u_humbar_alpha.x;
		BaseColor.rgb *= HumBrightness;
	}

	gl_FragColor = vec4(BaseColor.rgb * v_color0.rgb, BaseColor.a);
}
