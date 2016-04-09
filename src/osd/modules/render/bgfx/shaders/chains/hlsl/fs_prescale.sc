$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);

uniform vec4 u_source_dims;
uniform vec4 u_tex_size0;

void main()
{
	vec2 Scale = u_tex_size0.xy / u_source_dims.xy;

	vec2 TexelDims = v_texcoord0.xy * u_source_dims.xy;
	vec2 i = floor(TexelDims);
	vec2 s = fract(TexelDims);

	// Figure out where in the texel to sample to get the correct pre-scaled bilinear.
	vec2 CenterDistance = s - 0.5;
	vec2 RegionRange = 0.5 - 0.5 / Scale;
	vec2 f = (CenterDistance - clamp(CenterDistance, -RegionRange, RegionRange)) * Scale + 0.5;

	vec2 TexCoord = (i + f) / u_source_dims.xy;

	gl_FragColor = texture2D(s_tex, v_texcoord0.xy) * v_color0;
}
