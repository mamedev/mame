$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:okaz-code
// Analytic-AA fragment shader for vector lines.
//   v_texcoord0.y = position across the line width [0..1], 0.5 = center.
// The parabolic fade 1 - (2|y-0.5|)^2 gives a smooth, angle-independent width profile because
// the GPU interpolates the coordinate linearly across the quad.

#include "common.sh"

void main()
{
	float d = 2.0 * abs(v_texcoord0.y - 0.5);  // 0 (center) -> 1 (edge)
	float fade = max(0.0, 1.0 - d * d);
	gl_FragColor = v_color0 * vec4(1.0, 1.0, 1.0, fade);
}
