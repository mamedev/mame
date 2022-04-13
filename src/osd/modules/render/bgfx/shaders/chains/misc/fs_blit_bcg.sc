$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_pal, 1);

uniform vec4 u_inv_tex_size1;

void main()
{
	vec4 src = texture2D(s_tex, v_texcoord0);
	vec2 blu_uv = vec2(src.b * 256.0, 0.0) * u_inv_tex_size1.xy;
	vec2 grn_uv = vec2(src.g * 256.0, 1.0) * u_inv_tex_size1.xy;
	vec2 red_uv = vec2(src.r * 256.0, 2.0) * u_inv_tex_size1.xy;
	float blu = texture2D(s_pal, blu_uv).b;
	float grn = texture2D(s_pal, grn_uv).g;
	float red = texture2D(s_pal, red_uv).r;
	gl_FragColor = vec4(red, grn, blu, src.a) * v_color0;
}
