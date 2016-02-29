$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_ratio_amount;
uniform vec4 u_red_ratios;
uniform vec4 u_grn_ratios;
uniform vec4 u_blu_ratios;

SAMPLER2D(s_tex, 0);

void main()
{
	vec4 texel = texture2D(s_tex, v_texcoord0);
	float red = dot(texel.rgb, u_red_ratios.rgb);
	float grn = dot(texel.rgb, u_grn_ratios.rgb);
	float blu = dot(texel.rgb, u_blu_ratios.rgb);
	vec3 mixed = mix(texel.rgb, vec3(red, grn, blu), u_ratio_amount.x);
	gl_FragColor = vec4(mixed, texel.a) * v_color0;
}
