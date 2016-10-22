$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Autos
uniform vec4 u_tex_size0;

// User-supplied
uniform vec4 u_defocus;

// Samplers
SAMPLER2D(s_tex, 0);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

void main()
{
	// previously this pass was applied two times with offsets of 0.25, 0.5, 0.75, 1.0
	// now this pass is applied only once with offsets of 0.25, 0.55, 1.0, 1.6 to achieve the same appearance as before till a maximum defocus of 2.0
	// 0.075x² + 0.225x + 0.25
	const vec2 Coord1Offset = vec2(-1.60,  0.25);
	const vec2 Coord2Offset = vec2(-1.00, -0.55);
	const vec2 Coord3Offset = vec2(-0.55,  1.00);
	const vec2 Coord4Offset = vec2(-0.25, -1.60);
	const vec2 Coord5Offset = vec2( 0.25,  1.60);
	const vec2 Coord6Offset = vec2( 0.55, -1.00);
	const vec2 Coord7Offset = vec2( 1.00,  0.55);
	const vec2 Coord8Offset = vec2( 1.60, -0.25);

	// imaginary texel dimensions independed from source and target dimension
	vec2 TexelDims = vec2_splat(1.0 / 1024.0);

	vec2 DefocusTexelDims = u_defocus.xy * TexelDims.xy;

	vec4 d0 = texture2D(s_tex, v_texcoord0);
	vec4 d1 = texture2D(s_tex, v_texcoord0 + Coord1Offset * DefocusTexelDims);
	vec4 d2 = texture2D(s_tex, v_texcoord0 + Coord2Offset * DefocusTexelDims);
	vec4 d3 = texture2D(s_tex, v_texcoord0 + Coord3Offset * DefocusTexelDims);
	vec4 d4 = texture2D(s_tex, v_texcoord0 + Coord4Offset * DefocusTexelDims);
	vec4 d5 = texture2D(s_tex, v_texcoord0 + Coord5Offset * DefocusTexelDims);
	vec4 d6 = texture2D(s_tex, v_texcoord0 + Coord6Offset * DefocusTexelDims);
	vec4 d7 = texture2D(s_tex, v_texcoord0 + Coord7Offset * DefocusTexelDims);
	vec4 d8 = texture2D(s_tex, v_texcoord0 + Coord8Offset * DefocusTexelDims);

	vec4 blurred = (d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8) / 9.0;

	blurred.a = blurred.a + d0.a;
	gl_FragColor = blurred * v_color0;
}
