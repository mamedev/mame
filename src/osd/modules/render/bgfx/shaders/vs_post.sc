$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_color0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../../3rdparty/bgfx/examples/common/common.sh"

// Autos
uniform vec4 u_source_dims;
uniform vec4 u_screen_dims;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;
	v_texcoord1 = a_texcoord0.xy * u_source_dims.xy / u_screen_dims.x;
	v_color0 = a_color0;
}
