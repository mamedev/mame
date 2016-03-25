$input v_color0, v_texcoord0, v_texcoord1

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
//-----------------------------------------------------------------------------

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

// Autos
uniform vec4 u_swap_xy;
uniform vec4 u_screen_dims; // size of the output window
uniform vec4 u_source_dims; // size of the guest machine
uniform vec4 u_screen_scale; // TODO: Hook up ScreenScale code-side
uniform vec4 u_screen_offset; // TODO: Hook up ScreenOffset code-side

// User-supplied
uniform vec4 u_prepare_bloom; // disables some effects for rendering bloom textures
uniform vec4 u_prepare_vector;
uniform vec4 u_scanline_alpha;
uniform vec4 u_scanline_scale;
uniform vec4 u_scanline_bright_scale;
uniform vec4 u_scanline_bright_offset;
uniform vec4 u_scanline_jitter;
uniform vec4 u_scanline_height;
uniform vec4 u_scanline_variation;
uniform vec4 u_back_color; // TODO: Unused in current implementation, mostly
uniform vec4 u_shadow_tile_mode; // 0 based on screen dimension, 1 based on source dimension
uniform vec4 u_shadow_alpha;
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

void main()
{
	vec2 ScreenCoord = v_texcoord0.xy;
	vec2 BaseCoord = GetAdjustedCoords(v_texcoord0, vec2(0.5, 0.5));

	// Color
	vec4 BaseColor = texture2D(s_tex, BaseCoord);
	BaseColor.a = 1.0;

	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0)
	{
		BaseColor.rgb = vec3(0.0, 0.0, 0.0);
	}

	// Mask Simulation (may not affect bloom)
	if (u_prepare_bloom.x == 0.0 && u_shadow_alpha.x > 0.0)
	{
		vec2 screenCoord = ScreenCoord;
		screenCoord = u_swap_xy.x > 0.0 ? screenCoord.yx : screenCoord.xy;

		vec2 shadowTile = (u_shadow_tile_mode.x == 0.0 ? u_screen_dims.xy : u_source_dims.xy);
		shadowTile = u_swap_xy.x > 0.0 ? shadowTile.yx : shadowTile.xy;

		vec2 ShadowFrac = fract(screenCoord * shadowTile);
		vec2 ShadowCoord = (ShadowFrac * u_shadow_uv.xy);

		vec4 ShadowColor = texture2D(s_shadow, ShadowCoord);
		vec3 ShadowMaskColor = mix(vec3(1.0, 1.0, 1.0), ShadowColor.rgb, u_shadow_alpha.xxx);
		float ShadowMaskClear = (1.0 - ShadowColor.a) * u_shadow_alpha.x;

		// apply shadow mask color
		BaseColor.rgb *= ShadowMaskColor;
		// clear shadow mask by background color
		BaseColor.rgb = mix(BaseColor.rgb, u_back_color.rgb, ShadowMaskClear);
	}

	// Color Compression (may not affect bloom)
	if (u_prepare_bloom.x == 0.0)
	{
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = u_floor.rgb + (vec3(1.0, 1.0, 1.0) - u_floor.rgb) * BaseColor.rgb;
	}

	// Color Power (may affect bloom)
	BaseColor.r = pow(BaseColor.r, u_power.r);
	BaseColor.g = pow(BaseColor.g, u_power.g);
	BaseColor.b = pow(BaseColor.b, u_power.b);

	// Scanline Simulation (may not affect bloom)
	//if (u_prepare_bloom.x == 0.0)
	//{
		// Scanline Simulation (may not affect vector screen)
		//if (u_prepare_vector.x == 0.0 && u_scanline_alpha.x > 0.0f)
		//{
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
		//}

		// Hum Bar Simulation (may not affect vector screen)
		//if (u_prepare_vector.x == 0.0 && u_humbar_alpha.x > 0.0f)
		//{
			//float HumTimeStep = fract(u_time.x * 0.001);
			//float HumBrightness = 1.0 - fract(BaseCoord.y + HumTimeStep) * u_humbar_alpha.x;
			//BaseColor.rgb *= HumBrightness;
		//}
	//}

	//vec4 Output = u_prepare_vector.x > 0.0 ? BaseColor * (v_color0 + vec4(1.0, 1.0, 1.0, 0.0)) : BaseColor * v_color0;
	//Output.a = 1.0;

	gl_FragColor = vec4(BaseColor.rgb, 1.0);//Output;
}
