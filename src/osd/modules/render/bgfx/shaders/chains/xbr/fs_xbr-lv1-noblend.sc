$input v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR-lv1-noblend Shader
   
   Copyright (C) 2011-2014 Hyllian - sergiogdb@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

*/

#include "common.sh"

uniform vec4 XBR_Y_WEIGHT;
uniform vec4 XBR_EQ_THRESHOLD;

uniform vec4 u_tex_size0;

SAMPLER2D(decal, 0);

// Uncomment just one of the three params below to choose the corner detection
//#define CORNER_A
//#define CORNER_B
#define CORNER_C

float RGBtoYUV(vec3 color)
{
	return dot(color, XBR_Y_WEIGHT.xxx * vec3(0.2126, 0.7152, 0.0722));
}

float df(float A, float B)
{
	return abs(A-B);
}

bool eq(float A, float B)
{
	return (df(A, B) < XBR_EQ_THRESHOLD.x);
}

float weighted_distance(float a, float b, float c, float d, float e, float f, float g, float h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

/*
    xBR LVL1 works over the pixels below:

        |B |C |
     |D |E |F |F4|
     |G |H |I |I4|
        |H5|I5|

    Consider E as the central pixel. xBR LVL1 needs only to look at 12 texture pixels.
*/

void main()
{
	vec2 pos  = fract(v_texcoord0 * u_tex_size0.xy) - vec2(0.5, 0.5); // pos = pixel position
	vec2 dir = sign(pos); // dir = pixel direction

	vec2 g1 = dir * v_texcoord1.xy;
	vec2 g2 = dir * v_texcoord1.zw;

	vec3 B = texture2D(decal, v_texcoord0 +g1   ).xyz;
	vec3 C = texture2D(decal, v_texcoord0 +g1-g2).xyz;
	vec3 D = texture2D(decal, v_texcoord0    +g2).xyz;
	vec3 E = texture2D(decal, v_texcoord0       ).xyz;
	vec3 F = texture2D(decal, v_texcoord0    -g2).xyz;
	vec3 G = texture2D(decal, v_texcoord0 -g1+g2).xyz;
	vec3 H = texture2D(decal, v_texcoord0 -g1   ).xyz;
	vec3 I = texture2D(decal, v_texcoord0 -g1-g2).xyz;

	vec3 F4 = texture2D(decal,v_texcoord0    -2.0*g2   ).xyz;
	vec3 I4 = texture2D(decal,v_texcoord0 -g1-2.0*g2   ).xyz;
	vec3 H5 = texture2D(decal,v_texcoord0 -2.0*g1      ).xyz;
	vec3 I5 = texture2D(decal,v_texcoord0 -2.0*g1-g2   ).xyz;

	float b = RGBtoYUV( B );
	float c = RGBtoYUV( C );
	float d = RGBtoYUV( D );
	float e = RGBtoYUV( E );
	float f = RGBtoYUV( F );
	float g = RGBtoYUV( G );
	float h = RGBtoYUV( H );
	float i = RGBtoYUV( I );

	float i4 = RGBtoYUV( I4 );
	float i5 = RGBtoYUV( I5 );
	float h5 = RGBtoYUV( H5 );
	float f4 = RGBtoYUV( F4 );

	bool fx = (dot(dir,pos) > 0.5); // inequations of straight lines.

// It uses CORNER_C if none of the others are defined.
#ifdef CORNER_A
	bool interp_restriction_lv1      = ((e!=f) && (e!=h));
#elif CORNER_B
	bool interp_restriction_lv1      = ((e!=f) && (e!=h)  &&  ( !eq(f,b) && !eq(h,d) || eq(e,i) && !eq(f,i4) && !eq(h,i5) || eq(e,g) || eq(e,c) ) );
#else
	bool interp_restriction_lv1      = ((e!=f) && (e!=h)  && ( !eq(f,b) && !eq(f,c) || !eq(h,d) && !eq(h,g) || eq(e,i) && (!eq(f,f4) && !eq(f,i4) || !eq(h,h5) && !eq(h,i5)) || eq(e,g) || eq(e,c)) );
#endif
	bool edr = (weighted_distance(e, c, g, i, h5, f4, h, f) < weighted_distance(h, d, i5, f, i4, b, e, i)) && interp_restriction_lv1; // edr = edge detection rule

	bool nc = (edr && fx); // new_color

	bool px = (df(e,f) <= df(e,h)); // px = pixel

	vec3 res = nc ? px ? F : H : E;

	// final sum and weight normalization
	gl_FragColor = vec4(res, 1.0);
}