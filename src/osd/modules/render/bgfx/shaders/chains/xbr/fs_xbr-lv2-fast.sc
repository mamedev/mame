$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR-lv2-lq Shader
   
   Copyright (C) 2011/2015 Hyllian/Jararaca - sergiogdb@gmail.com

   Copyright (C) 2011-2015 Hyllian - sergiogdb@gmail.com

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

uniform vec4 u_tex_size0;

SAMPLER2D(decal, 0);

// Uncomment just one of the three params below to choose the corner detection
//#define CORNER_A
#define CORNER_C

#ifdef CORNER_C
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

const vec4 Y  = vec4(0.2126, 0.7152, 0.0722, 0.0);

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
	return (df(c,d) + df(e,f) + 3.0*df(g,h));
//	return (1.0*df(a,b) + 2.0*df(c,d) + 2.0*df(e,f) + 4.0*df(g,h));
}

void main()
{
	vec4 delta  = 1.0 / XBR_SCALE.xxxx;
	vec4 deltaL = vec4(0.5, 1.0, 0.5, 1.0) / XBR_SCALE.xxxx;
	vec4 deltaU = deltaL.yxwz;

	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);

	vec4 A  = texture2D(decal, v_texcoord1.xw);
	vec4 B  = texture2D(decal, v_texcoord1.yw);
	vec4 C  = texture2D(decal, v_texcoord1.zw);

	vec4 D  = texture2D(decal, v_texcoord2.xw);
	vec4 E  = texture2D(decal, v_texcoord2.yw);
	vec4 F  = texture2D(decal, v_texcoord2.zw);

	vec4 G  = texture2D(decal, v_texcoord3.xw);
	vec4 H  = texture2D(decal, v_texcoord3.yw);
	vec4 I  = texture2D(decal, v_texcoord3.zw);

	vec4 b = mul(mat4(B, D, H, F), XBR_Y_WEIGHT.xxxx * Y);
	vec4 c = mul(mat4(C, A, G, I), XBR_Y_WEIGHT.xxxx * Y);
	vec4 e = mul(mat4(E, E, E, E), XBR_Y_WEIGHT.xxxx * Y);
	vec4 a = c.yzwx;
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = (Ao*fp.y+Bo*fp.x); 
	vec4 fx_left = (Ax*fp.y+Bx*fp.x);
	vec4 fx_up   = (Ay*fp.y+By*fp.x);

	vec4 interp_restriction_lv0 = (ne(e,f) * ne(e,h));
    vec4 interp_restriction_lv1 = interp_restriction_lv0;

#ifndef CORNER_A
	interp_restriction_lv1 = clamp(interp_restriction_lv0 * (ge(f,b) * ge(f,c) + ge(h,d) * ge(h,g) + lt(e,g) + lt(e,c)), 0.0, 1.0);
#endif

	vec4 interp_restriction_lv2_left = (ne(e,g) * ne(d,g));
	vec4 interp_restriction_lv2_up   = (ne(e,c) * ne(b,c));

	vec4 fx45i = saturate((fx      + delta  -Co - Ci) / (2.0 * delta ));
	vec4 fx45  = saturate((fx      + delta  -Co     ) / (2.0 * delta ));
	vec4 fx30  = saturate((fx_left + deltaL -Cx     ) / (2.0 * deltaL));
	vec4 fx60  = saturate((fx_up   + deltaU -Cy     ) / (2.0 * deltaU));

	vec4 wd1 = weighted_distance( d, b, g, e, e, c, h, f);
	vec4 wd2 = weighted_distance( a, e, b, f, d, h, e, i);

    vec4 edri     = le(wd1, wd2) * interp_restriction_lv0;
	vec4 edr      = lt(wd1, wd2) * interp_restriction_lv1;
	vec4 edr_left = le(XBR_LV2_COEFFICIENT.xxxx * df(f,g), df(h,c)) * interp_restriction_lv2_left * edr;
	vec4 edr_up   = ge(df(f,g), XBR_LV2_COEFFICIENT.xxxx * df(h,c)) * interp_restriction_lv2_up * edr;

	fx45  = edr * fx45;
	fx30  = edr_left * fx30;
	fx60  = edr_up * fx60;
	fx45i = edri * fx45i;

	vec4 px = le(df(e,f), df(e,h));

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