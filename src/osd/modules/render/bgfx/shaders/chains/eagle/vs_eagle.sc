$input a_position, a_texcoord0, a_color0
$output v_color0, texCoord, t1, t2, t3, t4, t5, t6, t7, t8

// license:GPL-2.0+
// copyright-holders:The DOSBox Team

/*              SuperEagle code               */
/*  Copied from the Dosbox source code        */
/*  Copyright (C) 2002-2007  The DOSBox Team  */
/*  License: GNU-GPL                          */
/*  Adapted by guest(r) on 16.4.2007          */    

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

	texCoord = a_texcoord0.xy;
	t1.xy = texCoord + vec2(-dx,-dy);
	t1.zw = texCoord + vec2(-dx,  0);
	t2.xy = texCoord + vec2(+dx,-dy);
	t2.zw = texCoord + vec2(+dx+dx,-dy);
	t3.xy = texCoord + vec2(-dx,  0);
	t3.zw = texCoord + vec2(+dx,  0);
	t4.xy = texCoord + vec2(+dx+dx,  0);
	t4.zw = texCoord + vec2(-dx,+dy);
	t5.xy = texCoord + vec2(  0,+dy);
	t5.zw = texCoord + vec2(+dx,+dy);
	t6.xy = texCoord + vec2(+dx+dx,+dy);
	t6.zw = texCoord + vec2(-dx,+dy+dy);
	t7.xy = texCoord + vec2(  0,+dy+dy);
	t7.zw = texCoord + vec2(+dx,+dy+dy);
	t8.xy = texCoord + vec2(+dx+dx,+dy+dy);
}
