$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR LV2 C (squared) - pass0 Shader
   
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

SAMPLER2D(decal, 0);

#define round(X) floor((X)+0.5)

const float coef        = 2.0;
const vec4 eq_threshold = vec4(15.0, 15.0, 15.0, 15.0);
const float y_weight    = 48.0;
const float u_weight    = 7.0;
const float v_weight    = 6.0;
const mat4 yuv          = mat4(
	 0.299,  0.587,  0.114,  0.0,
	-0.169, -0.331,  0.499,  0.0,
	 0.499, -0.418, -0.0813, 0.0,
	 0.0, 0.0, 0.0, 0.0
);

const mat4 yuv_weighted = mat4(
	48.0 * vec4( 0.299,  0.587,  0.114,  0.0),
	 7.0 * vec4(-0.169, -0.331,  0.499,  0.0),
	 6.0 * vec4( 0.499, -0.418, -0.0813, 0.0),
	 0.0 * vec4( 0.0,    0.0,    0.0,    0.0)
);
const vec4 delta        = vec4(0.4, 0.4, 0.4, 0.4);

vec3 remapTo01(vec3 v, vec3 low, vec3 high)
{
	return saturate((v - low) / (high-low));
}

vec4 df(vec4 A, vec4 B)
{
	return vec4(abs(A-B));
}

float c_df(vec3 c1, vec3 c2)
{
	vec3 df = abs(c1 - c2);
	return df.r + df.g + df.b;
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
}

vec4 lt(vec4 A, vec4 B)
{
	return vec4(lessThan(A, B));
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

	vec4 b = mul(mat4(B, D, H, F), yuv_weighted[0]);
	vec4 c = mul(mat4(C, A, G, I), yuv_weighted[0]);
	vec4 e = mul(mat4(E, E, E, E), yuv_weighted[0]);
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4 = mul(mat4(I4, C1, A0, G5), yuv_weighted[0]);
	vec4 i5 = mul(mat4(I5, C4, A1, G0), yuv_weighted[0]);
	vec4 h5 = mul(mat4(H5, F4, B1, D0), yuv_weighted[0]);
	vec4 f4 = h5.yzwx;

	vec4 interp_restriction_lv1      = clamp(ne(e,f) * ne(e,h) * (ge(f,b) * ge(f,c) + ge(h,d) * ge(h,g) + lt(e,i) * (ge(f,f4) * ge(f,i4) + ge(h,h5) * ge(h,i5)) + lt(e,g) + lt(e,c)), 0.0, 1.0);
	vec4 interp_restriction_lv2_left = ne(e,g) * ne(d,g);
	vec4 interp_restriction_lv2_up   = ne(e,c) * ne(b,c);

	vec4 edr      = lt(weighted_distance(e, c, g, i, h5, f4, h, f), weighted_distance(h, d, i5, f, i4, b, e, i)) * interp_restriction_lv1;
	vec4 edr_left = le(coef * df(f,g), df(h,c)) * interp_restriction_lv2_left * edr;
	vec4 edr_up   = ge(df(f,g), coef * df(h,c)) * interp_restriction_lv2_up   * edr;

	vec3 info;
	info.x = dot(edr,      vec4(8.0, 4.0, 2.0, 1.0));
	info.y = dot(edr_left, vec4(8.0, 4.0, 2.0, 1.0));
	info.z = dot(edr_up,   vec4(8.0, 4.0, 2.0, 1.0));

	gl_FragColor = vec4(remapTo01(info, vec3(0.0, 0.0, 0.0), vec3(255.0, 255.0, 255.0)), 1.0);
}