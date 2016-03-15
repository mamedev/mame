$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0, v_texcoord1, v_texcoord2

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_texsize;
uniform vec4 u_screenrect;

uniform vec4 u_swap_xy;
uniform vec4 u_quad_dims;

uniform vec4 u_converge_red;
uniform vec4 u_converge_green;
uniform vec4 u_converge_blue;
uniform vec4 u_radial_converge_red;
uniform vec4 u_radial_converge_green;
uniform vec4 u_radial_converge_blue;

uniform vec4 u_source_dims;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));

	vec2 HalfSourceRect = u_screenrect.xy * 0.5;

	vec2 QuadRatio = vec2(1.0, (u_swap_xy.x > 0.0) ? u_quad_dims.y / u_quad_dims.x : u_quad_dims.x / u_quad_dims.y);

	// radial converge offset to "translate" the most outer pixel as thay would be translated by the linar converge with the same amount
	vec2 radialConvergeOffset = 2.0 * u_screenrect.xy;

	v_texcoord0 = (a_texcoord0 - HalfSourceRect) * (1.0 + u_radial_converge_red.xy   * radialConvergeOffset) + HalfSourceRect + u_converge_red.xy   * (vec2(1.0, 1.0) / u_source_dims.xy);
	v_texcoord1 = (a_texcoord0 - HalfSourceRect) * (1.0 + u_radial_converge_green.xy * radialConvergeOffset) + HalfSourceRect + u_converge_green.xy * (vec2(1.0, 1.0) / u_source_dims.xy);
	v_texcoord2 = (a_texcoord0 - HalfSourceRect) * (1.0 + u_radial_converge_blue.xy  * radialConvergeOffset) + HalfSourceRect + u_converge_blue.xy  * (vec2(1.0, 1.0) / u_source_dims.xy);

	v_color0 = a_color0;
}
