$input a_position, a_texcoord0
$output v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Generates a full-screen post-processing quad and forwards its texture
// coordinates for the decay, downsample, blur, and composite passes.

#include "common.sh"

void main()
{
	gl_Position = vec4(a_position.x * 2.0 - 1.0, 1.0 - a_position.y * 2.0, 0.0, 1.0);
	v_texcoord0 = a_texcoord0;
}
