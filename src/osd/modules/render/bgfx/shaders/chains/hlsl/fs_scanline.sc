$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Autos
uniform vec4 u_swap_xy;
uniform vec4 u_source_dims; // size of the guest machine
uniform vec4 u_target_dims;
uniform vec4 u_quad_dims;
uniform vec4 u_screen_scale;
uniform vec4 u_screen_offset;

// User-supplied
uniform vec4 u_scanline_alpha;
uniform vec4 u_scanline_scale;
uniform vec4 u_scanline_bright_scale;
uniform vec4 u_scanline_bright_offset;
uniform vec4 u_scanline_jitter;
uniform vec4 u_scanline_height;
uniform vec4 u_scanline_variation;

// Parametric
uniform vec4 u_time; // milliseconds
uniform vec4 u_jitter_amount;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_shadow, 1);

//-----------------------------------------------------------------------------
// Scanline Pixel Shader
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
		// Scanline Simulation
		if (u_scanline_alpha.x > 0.0f)
		{
			float BrightnessOffset = (u_scanline_bright_offset.x * u_scanline_alpha.x);
			float BrightnessScale = (u_scanline_bright_scale.x * u_scanline_alpha.x) + (1.0 - u_scanline_alpha.x);

			float ColorBrightness = 0.299 * BaseColor.r + 0.587 * BaseColor.g + 0.114 * BaseColor.b;

			float ScanCoord = BaseCoord.y;
			ScanCoord += u_swap_xy.x > 0.0
				? u_quad_dims.x <= u_source_dims.x * 2.0
					? 0.5 / u_quad_dims.x // uncenter scanlines if the quad is less than twice the size of the source
					: 0.0
				: u_quad_dims.y <= u_source_dims.y * 2.0
					? 0.5 / u_quad_dims.y // uncenter scanlines if the quad is less than twice the size of the source
					: 0.0;

			ScanCoord *= u_source_dims.y * u_scanline_scale.x * 3.1415927; // PI

			float ScanCoordJitter = u_scanline_jitter.x * u_jitter_amount.x * 1.5707963; // half PI
			float ScanSine = sin(ScanCoord + ScanCoordJitter);
			float ScanlineWide = u_scanline_height.x + u_scanline_variation.x * max(1.0, u_scanline_height.x) * (1.0 - ColorBrightness);
			float ScanSineScaled = pow(ScanSine * ScanSine, ScanlineWide);
			float ScanBrightness = ScanSineScaled * BrightnessScale + BrightnessOffset * BrightnessScale;

			BaseColor.rgb *= mix(vec3(1.0, 1.0, 1.0), vec3(ScanBrightness, ScanBrightness, ScanBrightness), u_scanline_alpha.xxx);
		}

		gl_FragColor = vec4(BaseColor.rgb * v_color0.rgb, BaseColor.a);
	}
}
