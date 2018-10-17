$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Shadowmask Effect
//-----------------------------------------------------------------------------

#include "common.sh"

#define MONOCHROME 1.0
#define DICHROME 2.0
#define TRICHROME 3.0

// Autos
uniform vec4 u_swap_xy;
uniform vec4 u_source_dims; // size of the guest machine
uniform vec4 u_target_dims;
uniform vec4 u_target_scale;
uniform vec4 u_screen_scale;
uniform vec4 u_screen_offset;
// uniform vec4 u_back_color; // TODO

// User-supplied
uniform vec4 u_shadow_tile_mode;
uniform vec4 u_shadow_alpha;
uniform vec4 u_shadow_count;
uniform vec4 u_shadow_uv;
uniform vec4 u_shadow_uv_offset;
uniform vec4 u_humbar_hertz_rate; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
uniform vec4 u_humbar_alpha;
uniform vec4 u_power;
uniform vec4 u_floor;
uniform vec4 u_chroma_mode;
uniform vec4 u_conversion_gain;

// Parametric
uniform vec4 u_time; // milliseconds

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_shadow, 1);

//-----------------------------------------------------------------------------
// Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

vec2 GetAdjustedCoords(vec2 coord)
{
	// center coordinates
	coord -= 0.5;

	// apply screen scale
	coord *= u_screen_scale.xy;

	// un-center coordinates
	coord += 0.5;

	// apply screen offset
	coord += u_screen_offset.xy;

	return coord;
}

vec2 GetShadowCoord(vec2 TargetCoord, vec2 SourceCoord)
{
	// base-target dimensions (remove oversampling)
	vec2 BaseTargetDims = u_target_dims.xy / u_target_scale.xy;
	BaseTargetDims = u_swap_xy.x > 0.0
		? BaseTargetDims.yx
		: BaseTargetDims.xy;

	vec2 canvasCoord = u_shadow_tile_mode.x == 0.0
		? TargetCoord + u_shadow_uv_offset.xy / BaseTargetDims
		: SourceCoord + u_shadow_uv_offset.xy / u_source_dims.xy;
	vec2 canvasTexelDims = u_shadow_tile_mode.x == 0.0
		? vec2(1.0, 1.0) / BaseTargetDims
		: vec2(1.0, 1.0) / u_source_dims.xy;

	vec2 shadowUV = u_shadow_uv.xy;
	vec2 shadowCount = u_shadow_count.xy;

	// swap x/y in screen mode (not source mode)
	canvasCoord = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y in screen mode (not source mode)
	shadowCount = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? shadowCount.yx
		: shadowCount.xy;

	vec2 shadowTile = canvasTexelDims * shadowCount;

	vec2 shadowFrac = fract(canvasCoord / shadowTile);

	// swap x/y in screen mode (not source mode)
	shadowFrac = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? shadowFrac.yx
		: shadowFrac.xy;

	vec2 shadowCoord = (shadowFrac * shadowUV);

	return shadowCoord;
}

void main()
{
	vec2 BaseCoord = GetAdjustedCoords(v_texcoord0);

	// Color
	vec4 BaseColor = texture2D(s_tex, BaseCoord);

	// Clamp
	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0 || BaseCoord.x > 1.0 || BaseCoord.y > 1.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	else
	{
		// Hum Bar Simulation
		if (u_humbar_alpha.x > 0.0f)
		{
			float HumTimeStep = fract(u_time.x * u_humbar_hertz_rate.x);
			float HumBrightness = 1.0 - fract(BaseCoord.y + HumTimeStep) * u_humbar_alpha.x;
			BaseColor.rgb *= HumBrightness;
		}

		// Mask Simulation
		if (u_shadow_alpha.x > 0.0)
		{
			vec2 ShadowCoord = GetShadowCoord(v_texcoord0.xy, BaseCoord.xy);

			vec4 ShadowColor = texture2D(s_shadow, ShadowCoord);
			vec3 ShadowMaskColor = mix(vec3(1.0, 1.0, 1.0), ShadowColor.rgb, u_shadow_alpha.xxx);

			// apply shadow mask color
			BaseColor.rgb *= ShadowMaskColor;

			// // TODO
			// vec3 ShadowMaskClear = (1.0f - ShadowColor.a) * u_shadow_alpha.xxx;

			// // clear shadow mask by background color
			// BaseColor.rgb = mix(BaseColor.rgb, u_back_color.rgb, ShadowMaskClear);
		}

		// Color Compression
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = u_floor.rgb + (vec3(1.0, 1.0, 1.0) - u_floor.rgb) * BaseColor.rgb;

		// Color Power
		BaseColor.r = pow(BaseColor.r, u_power.r);
		BaseColor.g = pow(BaseColor.g, u_power.g);
		BaseColor.b = pow(BaseColor.b, u_power.b);

		BaseColor.rgb *= v_color0.rgb;
		if (u_chroma_mode.x == MONOCHROME) {
			BaseColor.r = dot(u_conversion_gain.rgb, BaseColor.rgb);
			BaseColor.gb = vec2(BaseColor.r, BaseColor.r);
		} else if (u_chroma_mode.x == DICHROME) {
			BaseColor.r = dot(u_conversion_gain.rg, BaseColor.rg);
			BaseColor.g = BaseColor.r;
		}

		gl_FragColor = BaseColor;
	}
}
