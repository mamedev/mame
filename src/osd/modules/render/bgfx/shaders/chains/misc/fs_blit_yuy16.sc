$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

#define round(X) floor((X)+0.5)

vec4 u_tex_size0;
vec4 u_inv_tex_size0;

vec3 ycc_to_rgb(float y, float cb, float cr)
{
	float r = saturate(y + 1.40200 * (cr - 0.5));
	float g = saturate(y - 0.34414 * (cb - 0.5) - 0.71414 * (cr - 0.5));
	float b = saturate(y + 1.77200 * (cb - 0.5));
	return vec3(r, g, b);
}

void main()
{
	vec2 size_minus_one = u_tex_size0.xy - vec2(1.0, 1.0);
	vec2 original_uv = round(v_texcoord0.xy * size_minus_one);
	float mod_val = mod(original_uv.x, 2.0);
	vec2 rounded_uv = round(vec2(original_uv.x - mod_val, original_uv.y));
	vec2 next_uv = rounded_uv + vec2(1.0, 0.0);
	vec2 srcpix0 = texture2D(s_tex, rounded_uv / size_minus_one).rg;
	vec2 srcpix1 = texture2D(s_tex, next_uv / size_minus_one).rg;
	float cr = srcpix1.r;
	float cb = srcpix0.r;
	if (mod_val < 1.0)
		gl_FragColor = vec4(ycc_to_rgb(srcpix0.g, cb, cr), 1.0) * v_color0;
	else
		gl_FragColor = vec4(ycc_to_rgb(srcpix1.g, cb, cr), 1.0) * v_color0;
}
