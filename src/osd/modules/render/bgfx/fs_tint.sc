$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_tint;
uniform vec4 u_shift;

SAMPLER2D(s_tex, 0);
uniform vec4 u_texsize;

void main()
{
	gl_FragColor = texture2D(s_tex, v_texcoord0.xy + u_shift.xy * u_texsize.xy) * u_tint * v_color0 + vec4(0.0, 0.0, 0.0, 1.0);
}
