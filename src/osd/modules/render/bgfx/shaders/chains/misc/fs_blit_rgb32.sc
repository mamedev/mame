$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	gl_FragColor = vec4(texture2D(s_tex,  v_texcoord0).rgb, 1.0) * v_color0;
}
