$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

// Vertex shader for the left-hand pillarbox when displaying 3:4 content on a 16:9 screen.
// Crops off roughly 10% on all borders and takes only a portion of the right-hand side of the content

#include "common.sh"

void main()
{
 	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0 * vec2(1.0 * 0.8, 0.685185 * 0.9) + vec2(0.1, 0.1);
	v_color0 = a_color0;
}