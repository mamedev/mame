$input v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Attenuates the previous HDR phosphor buffer over time to simulate the
// persistence and gradual decay of light emitted by a vector CRT phosphor.

#include "common.sh"

SAMPLER2D(s_accum, 0);
uniform vec4 u_decay;

void main()
{
	vec3 previous = texture2D(s_accum, v_texcoord0).rgb;
	gl_FragColor = vec4(previous * u_decay.x, 1.0);
}
