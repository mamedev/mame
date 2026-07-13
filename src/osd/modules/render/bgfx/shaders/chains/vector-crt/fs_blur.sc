$input v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Applies one axis of a separable five-tap Gaussian blur to spread the
// downsampled phosphor image into a soft bloom texture.

#include "common.sh"

SAMPLER2D(s_tex, 0);
uniform vec4 u_blur;

void main()
{
	vec2 step = u_blur.xy;
	vec3 color = texture2D(s_tex, v_texcoord0).rgb * 0.2270270270;
	color += texture2D(s_tex, v_texcoord0 + step * 1.3846153846).rgb * 0.3162162162;
	color += texture2D(s_tex, v_texcoord0 - step * 1.3846153846).rgb * 0.3162162162;
	color += texture2D(s_tex, v_texcoord0 + step * 3.2307692308).rgb * 0.0702702703;
	color += texture2D(s_tex, v_texcoord0 - step * 3.2307692308).rgb * 0.0702702703;
	gl_FragColor = vec4(color, 1.0);
}
