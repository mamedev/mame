$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "common.sh"

uniform vec4 u_tex_size0;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;

	vec2 ps = 1.0 / u_tex_size0.xy;
	float dx = ps.x;
	float dy = ps.y;
	v_texcoord1 = v_texcoord0.xxxy + vec4( -dx,    0.0,  dx, -2.0*dy); // A1 B1 C1
	v_texcoord2 = v_texcoord0.xxxy + vec4( -dx,    0.0,  dx,     -dy); //  A  B  C
	v_texcoord3 = v_texcoord0.xxxy + vec4( -dx,    0.0,  dx,     0.0); //  D  E  F
	v_texcoord4 = v_texcoord0.xxxy + vec4( -dx,    0.0,  dx,      dy); //  G  H  I
	v_texcoord5 = v_texcoord0.xxxy + vec4( -dx,    0.0,  dx,  2.0*dy); // G5 H5 I5
	v_texcoord6 = v_texcoord0.xyyy + vec4(-2.0*dx, -dy, 0.0,      dy); // A0 D0 G0
	v_texcoord7 = v_texcoord0.xyyy + vec4( 2.0*dx, -dy, 0.0,      dy); // C4 F4 I4
	v_color0 = a_color0;
}
