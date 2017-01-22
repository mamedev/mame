$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku, Westley M. Martinez

#include "common.sh"

// User-supplied
uniform vec4 u_passthrough;
uniform vec4 u_phosphor_mode;
uniform vec4 u_phosphor_time;
uniform vec4 u_phosphor_beta;
uniform vec4 u_delta_time;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_prev, 1);

void main()
{
	int mode = int(u_phosphor_mode.x);
	float TAU_FACTOR = -1.0 / log(0.1);
	float GAMMA_INV_FACTOR = 1.0 / 900.0;
	float dt = u_delta_time.x / 1000.0;
	vec4 curr = texture2D(s_tex, v_texcoord0);
	vec4 prev = texture2D(s_prev, v_texcoord0);

	// Gamma correction
	if (curr.r <= 0.04045)
		curr.r /= 12.92;
	else
		curr.r = pow((curr.r + 0.055) / 1.055, 2.4);
	if (curr.g <= 0.04045)
		curr.g /= 12.92;
	else
		curr.g = pow((curr.g + 0.055) / 1.055, 2.4);
	if (curr.b <= 0.04045)
		curr.b /= 12.92;
	else
		curr.b = pow((curr.b + 0.055) / 1.055, 2.4);
	if (prev.r <= 0.04045)
		prev.r /= 12.92;
	else
		prev.r = pow((prev.r + 0.055) / 1.055, 2.4);
	if (prev.g <= 0.04045)
		prev.g /= 12.92;
	else
		prev.g = pow((prev.g + 0.055) / 1.055, 2.4);
	if (prev.b <= 0.04045)
		prev.b /= 12.92;
	else
		prev.b = pow((prev.b + 0.055) / 1.055, 2.4);
	if (mode == 0) {
		prev = vec4(0.0, 0.0, 0.0, 0.0);
	} else if (mode == 1) {
		vec4 tau = u_phosphor_time * TAU_FACTOR;

		prev.r *= tau.r == 0.0 ? 0.0 : exp(-dt / tau.r);
		prev.g *= tau.g == 0.0 ? 0.0 : exp(-dt / tau.g);
		prev.b *= tau.b == 0.0 ? 0.0 : exp(-dt / tau.b);
	} else {
		vec4 beta = u_phosphor_beta;
		vec4 gamma = 1.0 / (u_phosphor_time * GAMMA_INV_FACTOR);

		if (prev.r != 0.0)
			prev.r = pow(gamma.r * dt +
			             pow(1.0 / prev.r, 1.0 / beta.r), -beta.r);
		if (prev.g != 0.0)
			prev.g = pow(gamma.g * dt +
			             pow(1.0 / prev.g, 1.0 / beta.g), -beta.g);
		if (prev.b != 0.0)
			prev.b = pow(gamma.b * dt +
			             pow(1.0 / prev.b, 1.0 / beta.b), -beta.b);
	}

	vec4 color = max(curr, prev);

	// Gamma correction
	if (color.r <= 0.0031308)
		color.r *= 12.92;
	else
		color.r = 1.055 * pow(color.r, 1.0 / 2.4) - 0.055;
	if (color.g <= 0.0031308)
		color.g *= 12.92;
	else
		color.g = 1.055 * pow(color.g, 1.0 / 2.4) - 0.055;
	if (color.b <= 0.0031308)
		color.b *= 12.92;
	else
		color.b = 1.055 * pow(color.b, 1.0 / 2.4) - 0.055;
	gl_FragColor = u_passthrough.x > 0.0 ? curr : color;
}
