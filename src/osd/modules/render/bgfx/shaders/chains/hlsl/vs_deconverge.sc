$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "common.sh"

// Autos
uniform vec4 u_source_size;

// User-supplied
uniform vec4 u_converge_red;
uniform vec4 u_converge_green;
uniform vec4 u_converge_blue;
uniform vec4 u_radial_converge_red;
uniform vec4 u_radial_converge_green;
uniform vec4 u_radial_converge_blue;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));

	vec2 half_value = vec2(0.5, 0.5);

	v_texcoord0 = a_texcoord0;
	v_texcoord1 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_red.xy  ) + half_value + u_converge_red.xy   * (vec2(1.0, 1.0) / u_source_size.xy);
	v_texcoord2 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_green.xy) + half_value + u_converge_green.xy * (vec2(1.0, 1.0) / u_source_size.xy);
	v_texcoord3 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_blue.xy ) + half_value + u_converge_blue.xy  * (vec2(1.0, 1.0) / u_source_size.xy);

	v_color0 = a_color0;
}
