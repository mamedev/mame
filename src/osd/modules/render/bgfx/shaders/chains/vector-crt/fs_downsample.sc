$input v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Hans Andersson
// Reduces the full-resolution phosphor buffer with a four-sample box filter,
// producing the lower-resolution source used by the bloom blur passes.

#include "common.sh"

SAMPLER2D(s_accum, 0);
uniform vec4 u_source_texel;

void main()
{
	vec2 offset = u_source_texel.xy * 0.5;
	vec3 color = texture2D(s_accum, v_texcoord0 + vec2(-offset.x, -offset.y)).rgb;
	color += texture2D(s_accum, v_texcoord0 + vec2( offset.x, -offset.y)).rgb;
	color += texture2D(s_accum, v_texcoord0 + vec2(-offset.x,  offset.y)).rgb;
	color += texture2D(s_accum, v_texcoord0 + vec2( offset.x,  offset.y)).rgb;
	gl_FragColor = vec4(color * 0.25, 1.0);
}
