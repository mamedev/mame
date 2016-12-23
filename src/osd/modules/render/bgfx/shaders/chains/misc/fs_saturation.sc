$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// User-supplied
uniform vec4 u_saturation;

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	vec4 base = texture2D(s_tex, v_texcoord0);
	vec3 rgb = base.rgb;

	vec3 gray = vec3(0.299, 0.587, 0.114);
	float luma = dot(rgb, gray);
	vec3 chroma = rgb - luma;
	vec3 saturated = luma + chroma * u_saturation.x;

	gl_FragColor = vec4(saturated, base.a);
}
