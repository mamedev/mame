$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

/*
   Bob-and-ghost Deinterlacing
   Author: hunterk
   License: Public domain
   
   Note: This shader is designed to work with the typical interlaced output from an emulator, which displays both even and odd fields twice.
   As such, it is inappropriate for general video use unless the video has already been similarly woven beforehand.
*/

#include "common.sh"

// Samplers
SAMPLER2D(s0, 0);

uniform vec4 u_tex_size0;

#define one_pixel vec2(1.0 / u_tex_size0.xy)

void main()
{
	vec4 exponent = vec4(2.2, 2.2, 2.2, 2.2);
	vec4 inv_exponent = 1.0 / exponent;
	vec4 res = vec4(pow(texture2D(s0, v_texcoord0), exponent));
	vec4 color;
	float y;

	// assume anything with a vertical resolution greater than 400 lines is interlaced
	if (u_tex_size0.y > 400.0) 
	{
		y = u_tex_size0.y * v_texcoord0.y;// FIXME + IN.frame_count;
		res = pow(vec4(texture2D(s0, v_texcoord0 + vec2(0.0, one_pixel.y))), exponent);
		color = pow((vec4(texture2D(s0, v_texcoord0 - vec2(0.0, 0.5 * one_pixel.y))) + vec4(texture2D(s0, v_texcoord0 + vec2(0.0, 0.5 * one_pixel.y)))) / 2.0, exponent);
	}
	else
	{
		y = 2.0 * u_tex_size0.y * v_texcoord0.y;
		color = res;
	}

	if (mod(y, 2.0) > 0.99999)
	{
		res = res;
	}
	else
	{
		res = vec4(pow(texture2D(s0, v_texcoord0), exponent));
	}
	
	gl_FragColor = vec4(pow((res + color) / 2.0, inv_exponent));
}
