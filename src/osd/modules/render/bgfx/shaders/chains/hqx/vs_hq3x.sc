$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_color0

// license:LGPL-2.1+
// copyright-holders:Jules Blok,Cameron Zemek,Maxim Stepin

#include "common.sh"

// Autos
uniform vec4 u_tex_size0;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_color0 = a_color0;
	
	vec2 ps = vec2(1.0, 1.0) / u_tex_size0.xy;
	float dx = ps.x;
	float dy = ps.y;

	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+

	v_texcoord0 = a_texcoord0;
	v_texcoord1 = a_texcoord0.xxxy + vec4(-dx, 0.0, dx, -dy); //  w1 | w2 | w3
	v_texcoord2 = a_texcoord0.xxxy + vec4(-dx, 0.0, dx, 0.0); //  w4 | w5 | w6
	v_texcoord3 = a_texcoord0.xxxy + vec4(-dx, 0.0, dx,  dy); //  w7 | w8 | w9
}
