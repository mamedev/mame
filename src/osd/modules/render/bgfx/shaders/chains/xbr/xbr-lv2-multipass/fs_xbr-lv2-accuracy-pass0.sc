$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR LV2 Accuracy - pass0 Shader
   
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

SAMPLER2D(decal, 0);

uniform vec4 XBR_EQ_THRESHOLD;
uniform vec4 XBR_LV2_COEFFICIENT;

#define XBR_RED_COEF 17.0
#define XBR_GREEN_COEF 20.0
#define XBR_BLUE_COEF 3.0

#define coef XBR_LV2_COEFFICIENT.xxxx

const vec4 dtt = vec4(65536.0, 256.0, 1.0, 0.0);

vec4 remapTo01(vec4 v, vec4 low, vec4 high)
{
	return saturate((v - low)/(high-low));
}

float df1(vec4 A, vec4 B)
{
	float rmean = (A.r + B.r) / 2.0;
	vec4 diff = A - B;
	vec4 K = vec4(XBR_RED_COEF + rmean, XBR_GREEN_COEF, XBR_BLUE_COEF - rmean, 0.0);

	return sqrt(dot(K * diff, diff));
}

vec4 df(mat4 A, mat4 B)
{
	return vec4(df1(A[0],B[0]), df1(A[1],B[1]), df1(A[2],B[2]), df1(A[3],B[3]));
}

vec4 lt(vec4 A, vec4 B)
{
	return vec4(lessThan(A, B));
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
}

vec4 lt4(mat4 A, mat4 B)
{
	return vec4(lessThan(df(A, B), XBR_EQ_THRESHOLD.xxxx));
}

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 weighted_distance(mat4 a, mat4 b, mat4 c, mat4 d, mat4 e, mat4 f, mat4 g, mat4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec4 A1 = texture2D(decal, v_texcoord1.xw);
	vec4 B1 = texture2D(decal, v_texcoord1.yw);
	vec4 C1 = texture2D(decal, v_texcoord1.zw);

	vec4 A  = texture2D(decal, v_texcoord2.xw);
	vec4 B  = texture2D(decal, v_texcoord2.yw);
	vec4 C  = texture2D(decal, v_texcoord2.zw);

	vec4 D  = texture2D(decal, v_texcoord3.xw);
	vec4 E  = texture2D(decal, v_texcoord3.yw);
	vec4 F  = texture2D(decal, v_texcoord3.zw);

	vec4 G  = texture2D(decal, v_texcoord4.xw);
	vec4 H  = texture2D(decal, v_texcoord4.yw);
	vec4 I  = texture2D(decal, v_texcoord4.zw);

	vec4 G5 = texture2D(decal, v_texcoord5.xw);
	vec4 H5 = texture2D(decal, v_texcoord5.yw);
	vec4 I5 = texture2D(decal, v_texcoord5.zw);

	vec4 A0 = texture2D(decal, v_texcoord6.xy);
	vec4 D0 = texture2D(decal, v_texcoord6.xz);
	vec4 G0 = texture2D(decal, v_texcoord6.xw);

	vec4 C4 = texture2D(decal, v_texcoord7.xy);
	vec4 F4 = texture2D(decal, v_texcoord7.xz);
	vec4 I4 = texture2D(decal, v_texcoord7.xw);

	vec4 b_ = mul(mat4(B, D, H, F), dtt);
	vec4 c_ = mul(mat4(C, A, G, I), dtt);
	vec4 e_ = mul(mat4(E, E, E, E), dtt);
	vec4 d_ = b_.yzwx;
	vec4 f_ = b_.wxyz;
	vec4 g_ = c_.zwxy;
	vec4 h_ = b_.zwxy;
	vec4 i_ = c_.wxyz;

	vec4 i4_ = mul(mat4(I4, C1, A0, G5), dtt);
	vec4 i5_ = mul(mat4(I5, C4, A1, G0), dtt);
	vec4 h5_ = mul(mat4(H5, F4, B1, D0), dtt);
	vec4 f4_ = h5_.yzwx;

	mat4 b  = mat4(B, D, H, F);
	mat4 c  = mat4(C, A, G, I);
	mat4 e  = mat4(E, E, E, E);
	mat4 d  = mat4(D, H, F, B);
	mat4 f  = mat4(F, B, D, H);
	mat4 g  = mat4(G, I, C, A);
	mat4 h  = mat4(H, F, B, D);
	mat4 i  = mat4(I, C, A, G);

	mat4 i4 = mat4(I4, C1, A0, G5);
	mat4 i5 = mat4(I5, C4, A1, G0);
	mat4 h5 = mat4(H5, F4, B1, D0);
	mat4 f4 = mat4(F4, B1, D0, H5);

	vec4 interp_restriction_lv1      = ne(e_,f_) * ne(e_,h_);
	vec4 interp_restriction_lv2_left = ne(e_,g_) * ne(d_,g_);
	vec4 interp_restriction_lv2_up   = ne(e_,c_) * ne(b_,c_);

	vec4 wd1 = weighted_distance(e, c, g, i, h5, f4, h, f);
	vec4 wd2 = weighted_distance(h, d, i5, f, i4, b, e, i);

	vec4 one      = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 edri     = le(wd1,wd2) * interp_restriction_lv1;
	vec4 edr      = clamp(lt(wd1,wd2) * ((one - edri.yzwx) + (one - edri.wxyz)) * interp_restriction_lv1, 0.0, 1.0);
	vec4 edr_left = le(coef * df(f,g), df(h,c)) * interp_restriction_lv2_left * edr * (one - edri.yzwx) * lt4(e, c);
	vec4 edr_up   = ge(df(f,g), coef * df(h,c)) * interp_restriction_lv2_up   * edr * (one - edri.wxyz) * lt4(e, g);

	vec4 info;
	info.x = dot(edr,      vec4(8.0, 4.0, 2.0, 1.0));
	info.y = dot(edr_left, vec4(8.0, 4.0, 2.0, 1.0));
	info.z = dot(edr_up,   vec4(8.0, 4.0, 2.0, 1.0));
	info.w = dot(edri,     vec4(8.0, 4.0, 2.0, 1.0));

	gl_FragColor = remapTo01(info, vec4(0.0, 0.0, 0.0, 0.0), vec4(255.0, 255.0, 255.0, 255.0));
}