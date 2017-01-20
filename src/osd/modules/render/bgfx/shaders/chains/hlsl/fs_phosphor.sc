$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku, Westley M. Martinez

#include "common.sh"

// User-supplied
uniform vec4 u_passthrough;
uniform int u_phosphor_mode;
uniform vec4 u_phosphor_time;
uniform vec4 u_phosphor_beta;
uniform vec4 u_delta_time;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_prev, 1);

// Constants
const float TAU_FACTOR = -1.0 / log(0.1);
const float GAMMA_INV_FACTOR = 1.0 / 900.0;

void main()
{
	vec4 curr = texture2D(s_tex, v_texcoord0);
	vec3 prev = texture2D(s_prev, v_texcoord0).rgb;

	if (u_phosphor_mode == 0) {
		prev = vec3(0.0);
	} else if (u_phosphor_mode == 1) {
		vec3 tau = u_phosphor_time * TAU_FACTOR;

		prev.r *= tau.r == 0 ? 0 : exp(-u_delta_time / tau.r);
		prev.g *= tau.g == 0 ? 0 : exp(-u_delta_time / tau.g);
		prev.b *= tau.b == 0 ? 0 : exp(-u_delta_time / tau.b);
	} else {
		vec3 gamma = 1.0 / (u_phosphor_time * GAMMA_INV_FACTOR);

		if (prev.r != 0.0f)
			prev.r = pow(gamma.r * u_delta_time + pow(1 / r,
			         1 / u_phosphor_beta.r), -u_phosphor_beta.r);
		if (prev.g != 0.0f)
			prev.g = pow(gamma.g * u_delta_time + pow(1 / g,
			         1 / u_phosphor_beta.g), -u_phosphor_beta.g);
		if (prev.b != 0.0f)
			prev.b = pow(gamma.b * u_delta_time + pow(1 / b,
			         1 / u_phosphor_beta.b), -u_phosphor_beta.b);
	}

	vec3 maxed = max(curr.rgb, prev);

	gl_FragColor = u_passthrough.x > 0.0 ? curr : vec4(maxed, curr.a);
}
