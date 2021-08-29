$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

uniform vec4 u_tex_size0;
uniform vec4 u_inv_tex_size0;

vec3 ycc_to_rgb(float y, float cb, float cr)
{
	float r = saturate(y + 1.40200 * (cr - 0.5));
	float g = saturate(y - 0.34414 * (cb - 0.5) - 0.71414 * (cr - 0.5));
	float b = saturate(y + 1.77200 * (cb - 0.5));
	return vec3(r, g, b);
}

void main()
{
	vec2 half_texel = u_inv_tex_size0.xy * vec2(0.5, 0.5);

	vec2 original_uv = v_texcoord0.xy * u_tex_size0.xy;
	float mod_val = mod(original_uv.x, 2.0);
	vec2 rounded_uv = vec2(original_uv.x - mod_val, original_uv.y);
	vec4 srcpix = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + half_texel.x);
	
	float cr = srcpix.r;
	float cb = srcpix.b;
	if (mod_val < 1.0)
		gl_FragColor = vec4(ycc_to_rgb(srcpix.g, cb, cr), 1.0) * v_color0;
	else
		gl_FragColor = vec4(ycc_to_rgb(srcpix.a, cb, cr), 1.0) * v_color0;
}
