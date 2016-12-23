$input v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*

   Hyllian's xBR MultiLevel4 Shader - Pass1
   
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

*/

#include "common.sh"

SAMPLER2D(decal, 0);

#define round(X) floor((X)+0.5)
#define TEX(dx,dy) texture2D(decal, v_texcoord0 + vec2(dx,dy) * v_texcoord1.xy)

const float cf2            = 2.0;
const float cf3            = 4.0;
const float cf4            = 4.0;
const vec4 eq_thresholdold = vec4(15.0, 15.0, 15.0, 15.0);
const vec4 eq_threshold    = vec4( 2.0,  2.0,  2.0,  2.0);
const vec4 eq_threshold3   = vec4(25.0, 25.0, 25.0, 25.0);
const vec4 yuv_weight      = vec4( 4.0,  1.0,  2.0,  0.0);
const mat4 yuvT            = mat4(0.299, -0.169,  0.499, 0.0, 0.587, -0.331, -0.418, 0.0, 0.114,  0.499, -0.0813, 0.0, 0.0, 0.0, 0.0, 0.0); 
const mat4 yuv             = mat4(0.299, 0.587, 0.114, 0.0, -0.169, -0.331, 0.499, 0.0, 0.499, -0.418, -0.0813, 0.0, 0.0, 0.0, 0.0, 0.0);
const mat4 yuv_weighted    = mat4(4.0 * vec4(0.299, 0.587, 0.114, 0.0), 2.0 * vec4(-0.169, -0.331, 0.499, 0.0), vec4(0.499, -0.418, -0.0813, 0.0), vec4(0.0, 0.0, 0.0, 0.0));
const vec4 maximo          = vec4(255.0, 255.0, 255.0, 255.0);


vec4 df(vec4 A, vec4 B)
{
	return vec4(abs(A-B));
}

vec4 rd(vec4 A, vec4 B, vec4 C, vec4 D)
{
    return vec4(greaterThan(df(C,D) / (df(A,B) + 0.000000001), vec4(2.0, 2.0, 2.0, 2.0)));
}

vec4 id(vec4 A, vec4 B, vec4 C, vec4 D)
{
    return vec4(greaterThan(df(C,D), df(A,B)));
}

vec4 nid(vec4 A, vec4 B, vec4 C, vec4 D)
{
    return vec4(lessThanEqual(df(C,D), df(A,B)));
}

vec4 remapTo01(vec4 v, vec4 high)
{
	return (v/high);
}

vec4 lt(vec4 A, vec4 B)
{
	return vec4(lessThan(A, B));
}

vec4 ltt(vec4 A, vec4 B)
{
	return vec4(lessThan(df(A, B), eq_threshold));
}

vec4 get(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(df(A, B), eq_threshold3));
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
}

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec4 A3 = TEX(-1,-3);	vec4 B3 = TEX( 0,-3);	vec4 C3 = TEX( 1,-3);	
	vec4 A1 = TEX(-1,-2);	vec4 B1 = TEX( 0,-2);	vec4 C1 = TEX( 1,-2);
	vec4 A2 = TEX(-3,-1);	vec4 A0 = TEX(-2,-1);	vec4 A  = TEX(-1,-1);
	vec4 B  = TEX( 0,-1);	vec4 C  = TEX( 1,-1);	vec4 C4 = TEX( 2,-1);	vec4 C6 = TEX( 3,-1);
	vec4 D2 = TEX(-3, 0);	vec4 D0 = TEX(-2, 0);	vec4 D  = TEX(-1, 0);	vec4 E  = TEX( 0, 0);
	vec4 F  = TEX( 1, 0);	vec4 F4 = TEX( 2, 0);	vec4 F6 = TEX( 3, 0);
	vec4 G2 = TEX(-3, 1);	vec4 G0 = TEX(-2, 1);	vec4 G  = TEX(-1, 1);	vec4 H  = TEX( 0, 1);
	vec4 I  = TEX( 1, 1);	vec4 I4 = TEX( 2, 1);	vec4 I6 = TEX( 3, 1);
	vec4 G5 = TEX(-1, 2);	vec4 H5 = TEX( 0, 2);	vec4 I5 = TEX( 1, 2);
	vec4 G7 = TEX(-1, 3);	vec4 H7 = TEX( 0, 3);	vec4 I7 = TEX( 1, 3);

	mat4 bdhf = mul(mat4(B, D, H, F), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 b = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(C, A, G, I), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 c = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(E, E, E, E), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 e = mul(bdhf, yuv_weight);

	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	bdhf = mul(mat4(I4, C1, A0, G5), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 i4 = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(I5, C4, A1, G0), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 i5 = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(H5, F4, B1, D0), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 h5 = mul(bdhf, yuv_weight);

	vec4 f4 = h5.yzwx;

	vec4 c1 = i4.yzwx;
	vec4 g0 = i5.wxyz;
	vec4 b1 = h5.zwxy;
	vec4 d0 = h5.wxyz;

	bdhf = mul(mat4(I6, C3, A2, G7), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 i6 = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(I7, C6, A3, G2), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 i7 = mul(bdhf, yuv_weight);

	bdhf = mul(mat4(H7, F6, B3, D2), yuvT);
	bdhf = mat4(abs(bdhf[0]), abs(bdhf[1]), abs(bdhf[2]), abs(bdhf[3]));
	vec4 h7 = mul(bdhf, yuv_weight);

	vec4 f6 = h7.yzwx;

	vec4 c3 = i6.yzwx;
	vec4 g2 = i7.wxyz;
	vec4 b3 = h7.zwxy;
	vec4 d2 = h7.wxyz;

	vec4 interp_restriction_lv1      = ne(e,f ) * ne(e ,h );
	vec4 interp_restriction_lv2_left = ne(e,g ) * ne(d ,g ) * (ltt(e, d ) + ltt(h ,g ));
	vec4 interp_restriction_lv2_up   = ne(e,c ) * ne(b ,c ) * (ltt(e, b ) + ltt(f ,c ));
	vec4 interp_restriction_lv3_left = ne(e,g0) * ne(d0,g0) * (ltt(d ,d0) + ltt(g ,g0));
	vec4 interp_restriction_lv3_up   = ne(e,c1) * ne(b1,c1) * (ltt(b ,b1) + ltt(c ,c1));
	vec4 interp_restriction_lv4_left = ne(e,g2) * ne(d2,g2) * (ltt(d0,d2) + ltt(g0,g2));
	vec4 interp_restriction_lv4_up   = ne(e,c3) * ne(b3,c3) * (ltt(b1,b3) + ltt(c1,c3));

	vec4 wd1 = weighted_distance(e, c, g, i, h5, f4, h, f);
	vec4 wd2 = weighted_distance(h, d, i5, f, i4, b, e, i);

	vec4 edr0      = le(wd1, wd2) * interp_restriction_lv1;
	vec4 edr       = lt(wd1, wd2) * interp_restriction_lv1 * (ge(f,b) * nid(f,c,f,b) + ge(h,d) * nid(h,g,h,d) + lt(e,g) + lt(e,c));
	vec4 edr_left  = le(cf2 * df(f,g ), df(h,c )) * interp_restriction_lv2_left * edr;
	vec4 edr_up    = ge(df(f,g ), cf2 * df(h,c )) * interp_restriction_lv2_up   * edr;
	vec4 edr3_left = le(cf3 * df(f,g0), df(h,c1)) * interp_restriction_lv3_left * edr_left;
	vec4 edr3_up   = ge(df(f,g0), cf3 * df(h,c1)) * interp_restriction_lv3_up   * edr_up;
	vec4 edr4_left = le(cf4 * df(f,g2), df(h,c3)) * interp_restriction_lv4_left * edr3_left;
	vec4 edr4_up   = ge(df(f,g2), cf4 * df(h,c3)) * interp_restriction_lv4_up   * edr3_up;

	vec4 info;
	info.x = (edr4_left.x > 0.0 && edr4_up.x == 0.0) ? 8.0 : ((edr4_up.x > 0.0 && edr4_left.x == 0.0) ? 7.0 : ((edr3_left.x > 0.0 && edr3_up.x == 0.0) ? 6.0 : ((edr3_up.x > 0.0 && edr3_left.x == 0.0) ? 5.0 : ((edr_left.x > 0.0 && edr_up.x == 0.0) ? 4.0 : ((edr_up.x > 0.0 && edr_left.x == 0.0) ? 3.0 : (edr.x > 0.0 ? 2.0 : (edr0.x > 0.0 ? 1.0 : 0.0)))))));
	info.y = (edr4_left.y > 0.0 && edr4_up.y == 0.0) ? 8.0 : ((edr4_up.y > 0.0 && edr4_left.y == 0.0) ? 7.0 : ((edr3_left.y > 0.0 && edr3_up.y == 0.0) ? 6.0 : ((edr3_up.y > 0.0 && edr3_left.y == 0.0) ? 5.0 : ((edr_left.y > 0.0 && edr_up.y == 0.0) ? 4.0 : ((edr_up.y > 0.0 && edr_left.y == 0.0) ? 3.0 : (edr.y > 0.0 ? 2.0 : (edr0.y > 0.0 ? 1.0 : 0.0)))))));
	info.z = (edr4_left.z > 0.0 && edr4_up.z == 0.0) ? 8.0 : ((edr4_up.z > 0.0 && edr4_left.z == 0.0) ? 7.0 : ((edr3_left.z > 0.0 && edr3_up.z == 0.0) ? 6.0 : ((edr3_up.z > 0.0 && edr3_left.z == 0.0) ? 5.0 : ((edr_left.z > 0.0 && edr_up.z == 0.0) ? 4.0 : ((edr_up.z > 0.0 && edr_left.z == 0.0) ? 3.0 : (edr.z > 0.0 ? 2.0 : (edr0.z > 0.0 ? 1.0 : 0.0)))))));
	info.w = (edr4_left.w > 0.0 && edr4_up.w == 0.0) ? 8.0 : ((edr4_up.w > 0.0 && edr4_left.w == 0.0) ? 7.0 : ((edr3_left.w > 0.0 && edr3_up.w == 0.0) ? 6.0 : ((edr3_up.w > 0.0 && edr3_left.w == 0.0) ? 5.0 : ((edr_left.w > 0.0 && edr_up.w == 0.0) ? 4.0 : ((edr_up.w > 0.0 && edr_left.w == 0.0) ? 3.0 : (edr.w > 0.0 ? 2.0 : (edr0.w > 0.0 ? 1.0 : 0.0)))))));

	gl_FragColor = vec4(remapTo01(info, maximo));
}