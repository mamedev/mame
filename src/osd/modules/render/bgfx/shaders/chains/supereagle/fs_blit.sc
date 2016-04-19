$input v_color0, texCoord

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	gl_FragColor = texture2D(s_tex,  texCoord) * v_color0;
}
