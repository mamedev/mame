$input v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR-lv2-3d Shader
   
   Copyright (C) 2011-2016 Hyllian - sergiogdb@gmail.com

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

   Incorporates some of the ideas from SABR shader. Thanks to Joshua Street.
*/

#include "common.sh"

uniform vec4 XBR_SCALE;
uniform vec4 XBR_Y_WEIGHT;
uniform vec4 XBR_EQ_THRESHOLD;
uniform vec4 XBR_LV2_COEFFICIENT;
uniform vec4 XBR_RES;

uniform vec4 u_tex_size0;

SAMPLER2D(decal, 0);

// Uncomment just one of the three params below to choose the corner detection
//#define CORNER_A
//#define CORNER_B
#define CORNER_C
//#define CORNER_D

#ifndef CORNER_A
  #define SMOOTH_TIPS
#endif

const vec4 Ao = vec4( 1.0, -1.0, -1.0, 1.0 );
const vec4 Bo = vec4( 1.0,  1.0, -1.0,-1.0 );
const vec4 Co = vec4( 1.5,  0.5, -0.5, 0.5 );
const vec4 Ax = vec4( 1.0, -1.0, -1.0, 1.0 );
const vec4 Bx = vec4( 0.5,  2.0, -0.5,-2.0 );
const vec4 Cx = vec4( 1.0,  1.0, -0.5, 0.0 );
const vec4 Ay = vec4( 1.0, -1.0, -1.0, 1.0 );
const vec4 By = vec4( 2.0,  0.5, -2.0,-0.5 );
const vec4 Cy = vec4( 2.0,  0.0, -1.0, 0.5 );
const vec4 Ci = vec4(0.25, 0.25, 0.25, 0.25);

const vec4 Y = vec4(0.2126, 0.7152, 0.0722, 0.0);

vec4 df(vec4 A, vec4 B)
{
	return abs(A - B);
}

float c_df(vec3 c1, vec3 c2)
{
	vec3 df = abs(c1 - c2);
	return df.r + df.g + df.b;
}

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec4 lt(vec4 A, vec4 B)
{
	return vec4(lessThan(A, B));
}

vec4 eq(vec4 A, vec4 B)
{
	return vec4(equal(A, B));
}

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 abslt(vec4 A, vec4 B)
{
	return lt(df(A, B), XBR_EQ_THRESHOLD.xxxx);
}

vec4 absge(vec4 A, vec4 B)
{
	return ge(df(A, B), XBR_EQ_THRESHOLD.xxxx);
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec4 delta  = 1.0 / XBR_SCALE.xxxx;
	vec4 deltaL = vec4(0.5, 1.0, 0.5, 1.0) / XBR_SCALE.xxxx;
	vec4 deltaU = deltaL.yxwz;

	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy / XBR_RES.xx);

	vec2 tex = (floor(v_texcoord0 * u_tex_size0.xy / XBR_RES.xx) + vec2(0.5, 0.5)) * XBR_RES.xx / u_tex_size0.xy;

	vec2 dx = v_texcoord1.xy;
	vec2 dy = v_texcoord1.zw;

	vec4 A = texture2D(decal, v_texcoord0 -dx -dy);
	vec4 B = texture2D(decal, v_texcoord0     -dy);
	vec4 C = texture2D(decal, v_texcoord0 +dx -dy);
	vec4 D = texture2D(decal, v_texcoord0 -dx    );
	vec4 E = texture2D(decal, v_texcoord0        );
	vec4 F = texture2D(decal, v_texcoord0 +dx    );
	vec4 G = texture2D(decal, v_texcoord0 -dx +dy);
	vec4 H = texture2D(decal, v_texcoord0     +dy);
	vec4 I = texture2D(decal, v_texcoord0 +dx +dy);

	vec4  A1 = texture2D(decal, v_texcoord0     -dx -2.0*dy);
	vec4  B1 = texture2D(decal, v_texcoord0         -2.0*dy);
	vec4  C1 = texture2D(decal, v_texcoord0     +dx -2.0*dy);
	vec4  G5 = texture2D(decal, v_texcoord0     -dx +2.0*dy);
	vec4  H5 = texture2D(decal, v_texcoord0         +2.0*dy);
	vec4  I5 = texture2D(decal, v_texcoord0     +dx +2.0*dy);
	vec4  A0 = texture2D(decal, v_texcoord0 -2.0*dx     -dy);
	vec4  D0 = texture2D(decal, v_texcoord0 -2.0*dx        );
	vec4  G0 = texture2D(decal, v_texcoord0 -2.0*dx     +dy);
	vec4  C4 = texture2D(decal, v_texcoord0 +2.0*dx     -dy);
	vec4  F4 = texture2D(decal, v_texcoord0 +2.0*dx        );
	vec4  I4 = texture2D(decal, v_texcoord0 +2.0*dx     +dy);

	vec4 F6 = texture2D(decal, tex +dx+0.25*dx+0.25*dy);
	vec4 F7 = texture2D(decal, tex +dx+0.25*dx-0.25*dy);
	vec4 F8 = texture2D(decal, tex +dx-0.25*dx-0.25*dy);
	vec4 F9 = texture2D(decal, tex +dx-0.25*dx+0.25*dy);

	vec4 B6 = texture2D(decal, tex +0.25*dx+0.25*dy-dy);
	vec4 B7 = texture2D(decal, tex +0.25*dx-0.25*dy-dy);
	vec4 B8 = texture2D(decal, tex -0.25*dx-0.25*dy-dy);
	vec4 B9 = texture2D(decal, tex -0.25*dx+0.25*dy-dy);

	vec4 D6 = texture2D(decal, tex -dx+0.25*dx+0.25*dy);
	vec4 D7 = texture2D(decal, tex -dx+0.25*dx-0.25*dy);
	vec4 D8 = texture2D(decal, tex -dx-0.25*dx-0.25*dy);
	vec4 D9 = texture2D(decal, tex -dx-0.25*dx+0.25*dy);

	vec4 H6 = texture2D(decal, tex +0.25*dx+0.25*dy+dy);
	vec4 H7 = texture2D(decal, tex +0.25*dx-0.25*dy+dy);
	vec4 H8 = texture2D(decal, tex -0.25*dx-0.25*dy+dy);
	vec4 H9 = texture2D(decal, tex -0.25*dx+0.25*dy+dy);

	vec4 b = mul(mat4(B, D, H, F), XBR_Y_WEIGHT.xxxx * Y);
	vec4 c = mul(mat4(C, A, G, I), XBR_Y_WEIGHT.xxxx * Y);
	vec4 e = mul(mat4(E, E, E, E), XBR_Y_WEIGHT.xxxx * Y);
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4 = mul(mat4(I4, C1, A0, G5), XBR_Y_WEIGHT.xxxx * Y);
	vec4 i5 = mul(mat4(I5, C4, A1, G0), XBR_Y_WEIGHT.xxxx * Y);
	vec4 h5 = mul(mat4(H5, F4, B1, D0), XBR_Y_WEIGHT.xxxx * Y);
	vec4 f4 = h5.yzwx;

	vec4 f0 = mul(mat4(F6, B6, D6, H6), XBR_Y_WEIGHT.xxxx * Y);
	vec4 f1 = mul(mat4(F7, B7, D7, H7), XBR_Y_WEIGHT.xxxx * Y);
	vec4 f2 = mul(mat4(F8, B8, D8, H8), XBR_Y_WEIGHT.xxxx * Y);
	vec4 f3 = mul(mat4(F9, B9, D9, H9), XBR_Y_WEIGHT.xxxx * Y);

	vec4 h0 = f0.wxyz;
	vec4 h1 = f1.wxyz;
	vec4 h2 = f2.wxyz;
	vec4 h3 = f3.wxyz;

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = (Ao*fp.y+Bo*fp.x); 
	vec4 fx_left = (Ax*fp.y+Bx*fp.x);
	vec4 fx_up   = (Ay*fp.y+By*fp.x);

	vec4 block_3d = (eq(f0,f1) * eq(f1,f2) * eq(f2,f3) * eq(h0,h1) * eq(h1,h2) * eq(h2,h3));
	vec4 interp_restriction_lv0 = (ne(e,f) * ne(e,h) * block_3d);
	vec4 interp_restriction_lv1 = interp_restriction_lv0;

#ifdef CORNER_B
	interp_restriction_lv1 = (interp_restriction_lv0 * (ge(f,b) * ge(h,d) + eq(e,i) * ge(f,i4) * ge(h,i5) + eq(e,g) + eq(e,c)));
#endif
#ifdef CORNER_D
	vec4 c1 = i4.yzwx;
	vec4 g0 = i5.wxyz;
	interp_restriction_lv1 = (interp_restriction_lv0 * (ge(f,b) * ge(h,d) + eq(e,i) * ge(f,i4) * ge(h,i5) + eq(e,g) + eq(e,c) ) * (ne(f,f4) * ne(f,i) + ne(h,h5) * ne(h,i) + ne(h,g) + ne(f,c) + eq(b,c1) * eq(d,g0)));
#endif
#ifdef CORNER_C
	interp_restriction_lv1 = (interp_restriction_lv0 * (ge(f,b) * ge(f,c) + ge(h,d) * ge(h,g) + eq(e,i) * (ge(f,f4) * ge(f,i4) + ge(h,h5) * ge(h,i5)) + eq(e,g) + eq(e,c)));
#endif

	vec4 interp_restriction_lv2_left = (ne(e,g) * ne(d,g));
	vec4 interp_restriction_lv2_up   = (ne(e,c) * ne(b,c));

	vec4 fx45i = saturate((fx      + delta  -Co - Ci) / (2.0 * delta ));
	vec4 fx45  = saturate((fx      + delta  -Co     ) / (2.0 * delta ));
	vec4 fx30  = saturate((fx_left + deltaL -Cx     ) / (2.0 * deltaL));
	vec4 fx60  = saturate((fx_up   + deltaU -Cy     ) / (2.0 * deltaU));

	vec4 wd1 = weighted_distance( e, c, g, i, h5, f4, h, f);
	vec4 wd2 = weighted_distance( h, d, i5, f, i4, b, e, i);

	vec4 edri     = le(wd1,wd2) * interp_restriction_lv0;
	vec4 edr      = lt(wd1,wd2) * interp_restriction_lv1;
	vec4 edr_left = le(XBR_LV2_COEFFICIENT.xxxx * df(f,g), df(h,c)) * interp_restriction_lv2_left * edr;
	vec4 edr_up   = ge(df(f,g), XBR_LV2_COEFFICIENT.xxxx * df(h,c)) * interp_restriction_lv2_up * edr;

	fx45  = edr * fx45;
	fx30  = edr_left * fx30;
	fx60  = edr_up * fx60;
	fx45i = edri * fx45i;

	vec4 px = le(df(e,f3), df(e,h1));

#ifdef SMOOTH_TIPS
	vec4 maximos = max(max(fx30, fx60), max(fx45, fx45i));
#else
	vec4 maximos = max(max(fx30, fx60), fx45);
#endif

	vec3 res1 = E.xyz;
	res1 = mix(res1, mix(H.xyz, F.xyz, px.x), maximos.x);
	res1 = mix(res1, mix(B.xyz, D.xyz, px.z), maximos.z);
	
	vec3 res2 = E.xyz;
	res2 = mix(res2, mix(F.xyz, B.xyz, px.y), maximos.y);
	res2 = mix(res2, mix(D.xyz, H.xyz, px.w), maximos.w);
	
	vec3 E_mix = (c_df(E.xyz, res2) >= c_df(E.xyz, res1)) ? vec3(1.0, 1.0, 1.0) : vec3(0.0, 0.0, 0.0);
	vec3 res = mix(res1, res2, E_mix);

	gl_FragColor = vec4(res, 1.0);
}