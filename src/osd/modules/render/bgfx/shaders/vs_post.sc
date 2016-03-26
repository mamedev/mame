$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_color0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_swap_xy;

uniform vec4 u_shadow_uv_offset;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;
	v_texcoord1 = a_position.xy + ((u_swap_xy.x != 0.0) ? u_shadow_uv_offset.yx : u_shadow_uv_offset.xy);
	v_color0 = a_color0;
}
