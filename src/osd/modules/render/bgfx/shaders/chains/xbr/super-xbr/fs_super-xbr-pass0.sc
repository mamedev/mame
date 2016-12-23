$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   
  *******  Super XBR Shader - pass0  *******
   
  Copyright (c) 2015 Hyllian - sergiogdb@gmail.com

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

SAMPLER2D(s0, 0);

uniform vec4 XBR_EDGE_STR;
uniform vec4 XBR_WEIGHT;
uniform vec4 XBR_ANTI_RINGING;
uniform vec4 u_tex_size0;

#define wp1  1.0
#define wp2  0.0
#define wp3  0.0
#define wp4  2.0
#define wp5 -1.0
#define wp6  0.0

#define weight1 (XBR_WEIGHT.x * 1.29633 / 10.0)
#define weight2 (XBR_WEIGHT.x * 1.75068 / 10.0 / 2.0)

float RGBtoYUV(vec4 color)
{
	return dot(color.xyz, vec3(0.2126, 0.7152, 0.0722));
}

float df(float A, float B)
{
	return abs(A-B);
}

/*
                              P1
     |P0|B |C |P1|         C     F4          |a0|b1|c2|d3|
     |D |E |F |F4|      B     F     I4       |b0|c1|d2|e3|   |e1|i1|i2|e2|
     |G |H |I |I4|   P0    E  A  I     P3    |c0|d1|e2|f3|   |e3|i3|i4|e4|
     |P2|H5|I5|P3|      D     H     I5       |d0|e1|f2|g3|
                           G     H5
                              P2
*/


float d_wd(float b0, float b1, float c0, float c1, float c2, float d0, float d1, float d2, float d3, float e1, float e2, float e3, float f2, float f3)
{
	return (wp1*(df(c1,c2) + df(c1,c0) + df(e2,e1) + df(e2,e3)) + wp2*(df(d2,d3) + df(d0,d1)) + wp3*(df(d1,d3) + df(d0,d2)) + wp4*df(d1,d2) + wp5*(df(c0,c2) + df(e1,e3)) + wp6*(df(b0,b1) + df(f2,f3)));
}

float hv_wd(float i1, float i2, float i3, float i4, float e1, float e2, float e3, float e4)
{
	return ( wp4*(df(i1,i2)+df(i3,i4)) + wp1*(df(i1,e1)+df(i2,e2)+df(i3,e3)+df(i4,e4)) + wp3*(df(i1,e2)+df(i3,e4)+df(e1,i2)+df(e3,i4)));
}

vec4 min4(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return min(a, min(b, min(c, d)));
}

vec4 max4(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return max(a, max(b, max(c, d)));
}

void main()
{
	vec4 P0 = texture2D(s0, v_texcoord1.xy);
	vec4 P1 = texture2D(s0, v_texcoord1.zy);
	vec4 P2 = texture2D(s0, v_texcoord1.xw);
	vec4 P3 = texture2D(s0, v_texcoord1.zw);

	vec4  B = texture2D(s0, v_texcoord2.xy);
	vec4  C = texture2D(s0, v_texcoord2.zy);
	vec4 H5 = texture2D(s0, v_texcoord2.xw);
	vec4 I5 = texture2D(s0, v_texcoord2.zw);

	vec4  D = texture2D(s0, v_texcoord3.xy);
	vec4 F4 = texture2D(s0, v_texcoord3.zy);
	vec4  G = texture2D(s0, v_texcoord3.xw);
	vec4 I4 = texture2D(s0, v_texcoord3.zw);

	vec4  E = texture2D(s0, v_texcoord4.xy);
	vec4  F = texture2D(s0, v_texcoord4.zy);
	vec4  H = texture2D(s0, v_texcoord4.xw);
	vec4  I = texture2D(s0, v_texcoord4.zw);

	float b = RGBtoYUV( B );
	float c = RGBtoYUV( C );
	float d = RGBtoYUV( D );
	float e = RGBtoYUV( E );
	float f = RGBtoYUV( F );
	float g = RGBtoYUV( G );
	float h = RGBtoYUV( H );
	float i = RGBtoYUV( I );

	float i4 = RGBtoYUV( I4 ); float p0 = RGBtoYUV( P0 );
	float i5 = RGBtoYUV( I5 ); float p1 = RGBtoYUV( P1 );
	float h5 = RGBtoYUV( H5 ); float p2 = RGBtoYUV( P2 );
	float f4 = RGBtoYUV( F4 ); float p3 = RGBtoYUV( P3 );


	/* Calc edgeness in diagonal directions. */
	float d_edge  = (d_wd( d, b, g, e, c, p2, h, f, p1, h5, i, f4, i5, i4 ) - d_wd( c, f4, b, f, i4, p0, e, i, p3, d, h, i5, g, h5 ));

	/* Calc edgeness in horizontal/vertical directions. */
	float hv_edge = (hv_wd(f, i, e, h, c, i5, b, h5) - hv_wd(e, f, h, i, d, f4, g, i4));

	float limits = XBR_EDGE_STR.x + 0.000001;
	float edge_strength = smoothstep(0.0, limits, abs(d_edge));

	/* Filter weights. Two taps only. */
	vec4 w1 = vec4(-weight1, weight1+0.5, weight1+0.5, -weight1);
	vec4 w2 = vec4(-weight2, weight2+0.25, weight2+0.25, -weight2);

	/* Filtering and normalization in four direction generating four colors. */
	vec3 c1 = mul(w1, mat4( P2,   H,   F,   P1 )).xyz;
	vec3 c2 = mul(w1, mat4( P0,   E,   I,   P3 )).xyz;
	vec3 c3 = mul(w2, mat4(D+G, E+H, F+I, F4+I4)).xyz;
	vec3 c4 = mul(w2, mat4(C+B, F+E, I+H, I5+H5)).xyz;

	/* Smoothly blends the two strongest directions (one in diagonal and the other in vert/horiz direction). */
	vec4 d_mix = d_edge >= 0.0 ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(0.0, 0.0, 0.0, 0.0);
	vec3 hv_mix = hv_edge >= 0.0 ? vec3(1.0, 1.0, 1.0) : vec3(0.0, 0.0, 0.0);
	vec3 color = mix(mix(c1, c2, d_mix.xyz), mix(c3, c4, hv_mix), vec3(1.0, 1.0, 1.0) - vec3(edge_strength, edge_strength, edge_strength));

	/* Anti-ringing code. */
	float anti_ring = 1.0 - XBR_ANTI_RINGING.x;
	vec4 min_sample = min4(E, F, H, I) + anti_ring * mix((P2-H)*(F-P1), (P0-E)*(I-P3), d_mix);
	vec4 max_sample = max4(E, F, H, I) - anti_ring * mix((P2-H)*(F-P1), (P0-E)*(I-P3), d_mix);
	color = clamp(color, min_sample.xyz, max_sample.xyz);

	gl_FragColor = vec4(color, 1.0);
}