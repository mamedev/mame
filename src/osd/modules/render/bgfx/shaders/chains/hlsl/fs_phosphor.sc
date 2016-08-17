$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "common.sh"

// User-supplied
uniform vec4 u_passthrough;
uniform vec4 u_phosphor;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_prev, 1);

void main()
{
	vec4 curr = texture2D(s_tex, v_texcoord0);
	vec3 prev = texture2D(s_prev, v_texcoord0).rgb * u_phosphor.rgb;

	vec3 maxed = max(curr.rgb, prev);

	gl_FragColor = u_passthrough.x > 0.0 ? curr : vec4(maxed, curr.a);
}
