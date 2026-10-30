$input v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Applies one axis of a separable Gaussian blur using texel-centred taps.

#include "common.sh"

SAMPLER2D(s_tex, 0);

// xy: one bloom-texture texel along the blur axis
// z:  Gaussian sigma in bloom-texture texels
uniform vec4 u_blur;

void main()
{
	vec2 texel = u_blur.xy;
	float sigma = max(u_blur.z, 0.001);
	float denominator = 2.0 * sigma * sigma;

	float w0 = 1.0;
	float w1 = exp(-1.0 / denominator);
	float w2 = exp(-4.0 / denominator);
	float w3 = exp(-9.0 / denominator);
	float w4 = exp(-16.0 / denominator);

	float normalization = w0 + 2.0 * (w1 + w2 + w3 + w4);

	vec3 color = texture2D(s_tex, v_texcoord0).rgb * w0;

	color += texture2D(s_tex, v_texcoord0 + texel * 1.0).rgb * w1;
	color += texture2D(s_tex, v_texcoord0 - texel * 1.0).rgb * w1;

	color += texture2D(s_tex, v_texcoord0 + texel * 2.0).rgb * w2;
	color += texture2D(s_tex, v_texcoord0 - texel * 2.0).rgb * w2;

	color += texture2D(s_tex, v_texcoord0 + texel * 3.0).rgb * w3;
	color += texture2D(s_tex, v_texcoord0 - texel * 3.0).rgb * w3;

	color += texture2D(s_tex, v_texcoord0 + texel * 4.0).rgb * w4;
	color += texture2D(s_tex, v_texcoord0 - texel * 4.0).rgb * w4;

	gl_FragColor = vec4(color / normalization, 1.0);
}
