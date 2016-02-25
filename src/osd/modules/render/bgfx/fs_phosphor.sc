$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_passthrough;
uniform vec4 u_phosphor;

SAMPLER2D(s_tex, 0);
SAMPLER2D(s_prev, 1);

void main()
{
	vec4 curr = texture2D(s_tex,  v_texcoord0);
	vec4 prev = texture2D(s_prev, v_texcoord0);
	vec4 phosphored = vec4(max(curr.r, prev.r * u_phosphor.r), max(curr.g, prev.g * u_phosphor.g), max(curr.b, prev.b * u_phosphor.b), curr.a);
	gl_FragColor = mix(phosphored, curr, u_passthrough.x) * v_color0;
}
