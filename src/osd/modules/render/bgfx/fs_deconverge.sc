$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

// NB: intentionally wasteful of uniforms in order for easier slider utilization

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

SAMPLER2D(DiffuseSampler, 0);

void main()
{
	float r = texture2D(DiffuseSampler, v_texcoord0).r;
	float g = texture2D(DiffuseSampler, v_texcoord1).g;
	float b = texture2D(DiffuseSampler, v_texcoord2).b;
	gl_FragColor = vec4(r, g, b, 1.0) * v_color0;
}
