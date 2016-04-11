$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	float a = texture2D(s_tex, v_texcoord0).a;
	float r = texture2D(s_tex, v_texcoord1).r;
	float g = texture2D(s_tex, v_texcoord2).g;
	float b = texture2D(s_tex, v_texcoord3).b;
	gl_FragColor = vec4(r, g, b, a) * v_color0;
}
