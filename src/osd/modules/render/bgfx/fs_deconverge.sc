$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	float r = texture2D(s_tex, v_texcoord0).r;
	float g = texture2D(s_tex, v_texcoord1).g;
	float b = texture2D(s_tex, v_texcoord2).b;
	gl_FragColor = vec4(r, g, b, 1.0) * v_color0;
}
