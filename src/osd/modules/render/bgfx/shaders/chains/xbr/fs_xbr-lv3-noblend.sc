$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR-lv3-noblend - Shader
   
   Copyright (C) 2011/2016 Hyllian - sergiogdb@gmail.com

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

uniform vec4 XBR_EQ_THRESHOLD;
uniform vec4 XBR_EQ_THRESHOLD2;
uniform vec4 XBR_LV2_COEFFICIENT;

uniform vec4 u_tex_size0;

SAMPLER2D(decal, 0);

// Uncomment just one of the four params below to choose the corner detection
//#define CORNER_A
//#define CORNER_B
//#define CORNER_C
#define CORNER_D

vec4 df(vec4 A, vec4 B)
{
	return abs(A - B);
}

float c_df(vec3 c1, vec3 c2)
{
	return dot(abs(c1 - c2), vec3(1.0, 1.0, 1.0));
}

vec4 eq(vec4 A, vec4 B)
{
	return vec4(lessThan(df(A, B), XBR_EQ_THRESHOLD.xxxx));
}

vec4 eq2(vec4 A, vec4 B)
{
	return vec4(lessThan(df(A, B), XBR_EQ_THRESHOLD2.xxxx));
}

vec4 neq(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(df(A, B), XBR_EQ_THRESHOLD.xxxx));
}

vec4 neq2(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(df(A, B), XBR_EQ_THRESHOLD2.xxxx));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec4 Y = vec4(0.2126, 0.7152, 0.0722, 0.0);

	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);

	vec4 A1 = texture2D(decal, v_texcoord1.xw);
	vec4 B1 = texture2D(decal, v_texcoord1.yw);
	vec4 C1 = texture2D(decal, v_texcoord1.zw);

	vec4 A = texture2D(decal, v_texcoord2.xw);
	vec4 B = texture2D(decal, v_texcoord2.yw);
	vec4 C = texture2D(decal, v_texcoord2.zw);

	vec4 D = texture2D(decal, v_texcoord3.xw);
	vec4 E = texture2D(decal, v_texcoord3.yw);
	vec4 F = texture2D(decal, v_texcoord3.zw);

	vec4 G = texture2D(decal, v_texcoord4.xw);
	vec4 H = texture2D(decal, v_texcoord4.yw);
	vec4 I = texture2D(decal, v_texcoord4.zw);

	vec4 G5 = texture2D(decal, v_texcoord5.xw);
	vec4 H5 = texture2D(decal, v_texcoord5.yw);
	vec4 I5 = texture2D(decal, v_texcoord5.zw);

	vec4 A0 = texture2D(decal, v_texcoord6.xy);
	vec4 D0 = texture2D(decal, v_texcoord6.xz);
	vec4 G0 = texture2D(decal, v_texcoord6.xw);

	vec4 C4 = texture2D(decal, v_texcoord7.xy);
	vec4 F4 = texture2D(decal, v_texcoord7.xz);
	vec4 I4 = texture2D(decal, v_texcoord7.xw);

	vec4 b = instMul(Y, mat4(B, D, H, F));
	vec4 c = instMul(Y, mat4(C, A, G, I));
	vec4 e = instMul(Y, mat4(E, E, E, E));
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4 = instMul(Y, mat4(I4, C1, A0, G5));
	vec4 i5 = instMul(Y, mat4(I5, C4, A1, G0));
	vec4 h5 = instMul(Y, mat4(H5, F4, B1, D0));
	vec4 f4 = h5.yzwx;

	vec4 c4 = i5.yzwx;
	vec4 g5 = i4.wxyz;

	vec4 c1 = i4.yzwx;
	vec4 g0 = i5.wxyz;
	vec4 b1 = h5.zwxy;
	vec4 d0 = h5.wxyz;


	vec4 Ao = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 Bo = vec4( 1.0, 1.0, -1.0,-1.0 );
	vec4 Co = vec4( 1.5, 0.5, -0.5, 0.5 );
	vec4 Ax = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 Bx = vec4( 0.5, 2.0, -0.5,-2.0 );
	vec4 Cx = vec4( 1.0, 1.0, -0.5, 0.0 );
	vec4 Ay = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 By = vec4( 2.0, 0.5, -2.0,-0.5 );
	vec4 Cy = vec4( 2.0, 0.0, -1.0, 0.5 );

	vec4 Az = vec4( 6.0, -2.0, -6.0, 2.0 );
	vec4 Bz = vec4( 2.0, 6.0, -2.0, -6.0 );
	vec4 Cz = vec4( 5.0, 3.0, -3.0, -1.0 );
	vec4 Aw = vec4( 2.0, -6.0, -2.0, 6.0 );
	vec4 Bw = vec4( 6.0, 2.0, -6.0,-2.0 );
	vec4 Cw = vec4( 5.0, -1.0, -3.0, 3.0 );

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = vec4(greaterThan(Ao * fp.y + Bo * fp.x, Co));
	vec4 fx_left = vec4(greaterThan(Ax * fp.y + Bx * fp.x, Cx));
	vec4 fx_up   = vec4(greaterThan(Ay * fp.y + By * fp.x, Cy));
	vec4 fx3_left= vec4(greaterThan(Az * fp.y + Bz * fp.x, Cz));
	vec4 fx3_up  = vec4(greaterThan(Aw * fp.y + Bw * fp.x, Cw));

	vec4 interp_restriction_lv0 = (vec4(notEqual(e,f)) * vec4(notEqual(e,h)));

	// It uses CORNER_C if none of the others are defined.
#ifdef CORNER_A
	vec4 interp_restriction_lv1 = interp_restriction_lv0;
#elif defined(CORNER_B)
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * (neq(f,b) * neq(h,d) + eq(e,i) * neq(f,i4) * neq(h,i5) + eq(e,g) + eq(e,c)));
#elif defined(CORNER_D)
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * (neq(f,b) * neq(h,d) + eq(e,i) * neq(f,i4) * neq(h,i5) + eq(e,g) + eq(e,c)) * (vec4(notEqual(f,f4)) * vec4(notEqual(f,i)) + vec4(notEqual(h,h5)) * vec4(notEqual(h,i)) + vec4(notEqual(h,g)) + vec4(notEqual(f,c)) + eq(b,c1) * eq(d,g0)));
#elif defined(CORNER_C)
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * (neq(f,b) * neq(f,c) + neq(h,d) * neq(h,g) + eq(e,i) * (neq(f,f4) * neq(f,i4) + neq(h,h5) * neq(h,i5)) + eq(e,g) + eq(e,c)));
#endif

	interp_restriction_lv1 = clamp(interp_restriction_lv1, 0.0, 1.0);
	
	vec4 interp_restriction_lv2_left = vec4(notEqual(e,g)) * vec4(notEqual(d,g));
	vec4 interp_restriction_lv2_up   = vec4(notEqual(e,c)) * vec4(notEqual(b,c));
	vec4 interp_restriction_lv3_left = eq2(g,g0) * vec4(notEqual(d0,g0));
	vec4 interp_restriction_lv3_up   = eq2(c,c1) * vec4(notEqual(b1,c1));

	vec4 wd1 = weighted_distance(e, c, g, i, h5, f4, h, f);
	vec4 wd2 = weighted_distance(h, d, i5, f, i4, b, e, i);

	vec4 one      = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 zero     = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec4 edri     = vec4(lessThanEqual(wd1,wd2)) * interp_restriction_lv1;
	vec4 edr      = vec4(lessThan(wd1,wd2)) * interp_restriction_lv1 * ((1.0 - edri.yzwx) + (1.0 - edri.wxyz));
	edr = clamp(edr, 0.0, 1.0);
	
	vec4 hcDiff = df(h,c);
	vec4 fgDiff = df(f,g);
	vec4 edr_left = vec4(lessThanEqual(XBR_LV2_COEFFICIENT.xxxx * fgDiff, hcDiff)) * interp_restriction_lv2_left * edr * (1.0 - edri.yzwx) * eq(e,c);
	vec4 edr_up   = vec4(greaterThanEqual(fgDiff, XBR_LV2_COEFFICIENT.xxxx * hcDiff)) * interp_restriction_lv2_up * edr * (1.0 - edri.wxyz) * eq(e,g);
	vec4 edr3_left = interp_restriction_lv3_left;
	vec4 edr3_up = interp_restriction_lv3_up;

	vec4 nc = edr * (fx + edr_left * (fx_left + edr3_left * fx3_left * eq(e,c4)) + edr_up * (fx_up + edr3_up * fx3_up * eq(e,g5)));

	bvec4 px = lessThanEqual(df(e,f), df(e,h));

	vec3 res1 = nc.x > 0.0 ? px.x ? F.xyz : H.xyz : nc.y > 0.0 ? px.y ? B.xyz : F.xyz : nc.z > 0.0 ? px.z ? D.xyz : B.xyz : nc.w > 0.0 ? px.w ? H.xyz : D.xyz : E.xyz;
	vec3 res2 = nc.w > 0.0 ? px.w ? H.xyz : D.xyz : nc.z > 0.0 ? px.z ? D.xyz : B.xyz : nc.y > 0.0 ? px.y ? B.xyz : F.xyz : nc.x > 0.0 ? px.x ? F.xyz : H.xyz : E.xyz;

	vec2 df12 = abs(mul(mat3(res1, res2, zero.xyz), Y.xyz).xy - e.xy);

	vec3 res = mix(res1, res2, (df12.y >= df12.x) ? one.xyz : zero.xyz);

	gl_FragColor = vec4(res, 1.0);
}