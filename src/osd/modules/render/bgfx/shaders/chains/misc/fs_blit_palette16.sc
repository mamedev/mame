$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_pal, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_inv_tex_size0;
uniform vec4 u_inv_tex_size1;

void main()
{
	vec2 original_uv = v_texcoord0.xy * u_tex_size0.xy * vec2(2.0, 1.0);
	float mod_val = mod(original_uv.x, 4.0);
	vec2 rounded_uv = vec2(original_uv.x - mod_val, original_uv.y) * vec2(0.5, 1.0);

	float inv_width = u_inv_tex_size0.x * 0.5;

	vec2 palette_uv = vec2(0.0, 0.0);
	if (mod_val < 2.0)
	{
		palette_uv.x = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + vec2(inv_width * 0.5, 0.0)).r;
		palette_uv.y = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + vec2(inv_width * 1.5, 0.0)).r;
	}
	else
	{
		palette_uv.x = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + vec2(inv_width * 2.5, 0.0)).r;
		palette_uv.y = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + vec2(inv_width * 3.5, 0.0)).r;
	}

	gl_FragColor = vec4(texture2D(s_pal, palette_uv * vec2(256.0, 256.0) * u_inv_tex_size1.xy).rgb, 1.0) * v_color0;
}
