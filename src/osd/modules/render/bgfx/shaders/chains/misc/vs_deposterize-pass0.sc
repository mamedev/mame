$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_color0

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's Deposterize Shader - Pass0 vertex shader
   
   Copyright (C) 2011/2016 Hyllian/Jararaca - sergiogdb@gmail.com

*/

#include "common.sh"

uniform vec4 u_tex_size0;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));

	vec2 ps = vec2(1.0 / u_tex_size0.xy);
	float dx = ps.x;
	float dy = ps.y;
	
	// This line fixes a bug in ATI cards.
	v_texcoord0 = a_texcoord0 + vec2(0.0000001, 0.0000001);
	v_texcoord1 = v_texcoord0.xxxy + vec4(-dx, 0, dx, 0); //  D  E  F
	v_color0 = a_color0;
}
