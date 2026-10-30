$input v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Combines persistent phosphor and bloom, then applies exposure, luminance
// tone mapping, gamma correction, and optional edge vignetting for display.

#include "common.sh"

SAMPLER2D(s_accum, 0);
SAMPLER2D(s_bloom, 1);
uniform vec4 u_composite;

void main()
{
	vec3 phosphor = max(
			texture2D(s_accum, v_texcoord0).rgb,
			vec3_splat(0.0));

	vec3 bloom = max(
			texture2D(s_bloom, v_texcoord0).rgb,
			vec3_splat(0.0));

	vec3 hdr =
			(phosphor + bloom * u_composite.x) *
			u_composite.y;

	// Tone-map luminance rather than each RGB channel separately.
	// This retains the saturation and hue of bright vector colors.
	float luminance = dot(
			hdr,
			vec3(0.2126, 0.7152, 0.0722));

	float mappedLuminance =
			1.0 - exp(-luminance);

	vec3 mapped =
			luminance > 0.0001
			? hdr * (mappedLuminance / luminance)
			: vec3_splat(0.0);

	mapped = pow(
			max(mapped, vec3_splat(0.0)),
			vec3_splat(
					1.0 /
					max(u_composite.z, 0.001)));

	vec2 edge =
			v_texcoord0 *
			(vec2_splat(1.0) - v_texcoord0);

	float vignette =
			clamp(
					16.0 * edge.x * edge.y,
					0.0,
					1.0);

	mapped *= mix(
			1.0,
			vignette,
			u_composite.w);

	gl_FragColor = vec4(mapped, 1.0);
}
