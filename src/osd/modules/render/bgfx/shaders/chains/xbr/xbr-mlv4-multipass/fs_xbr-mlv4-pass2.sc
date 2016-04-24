$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*

   Hyllian's xBR MultiLevel4 Shader - Pass2
   
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
SAMPLER2D(ORIG_texture, 1);

#define round(X) floor((X)+0.5)

const float cf2           = 2.0;
const float cf3           = 4.0;
const float cf4           = 4.0;
const vec4 eq_threshold   = vec4(15.0, 15.0, 15.0, 15.0);
const vec4 eq_threshold2  = vec4( 5.0,  5.0,  5.0,  5.0);
const vec4 eq_threshold3  = vec4(25.0, 25.0, 25.0, 25.0);
const mat4 yuv_weighted   = mat4(48.0 * vec4(0.299, 0.587, 0.114, 0.0), 7.0 * vec4(-0.169, -0.331, 0.499, 0.0), 6.0 * vec4(0.499, -0.418, -0.0813, 0.0), vec4(0.0, 0.0, 0.0, 0.0));
const vec4 maximo         = vec4(255.0, 255.0, 255.0, 255.0);

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

vec4 remapFrom01(vec4 v, vec4 high)
{
	return round(high*v);
}

vec4 eq(vec4 A, vec4 B)
{
	return vec4(equal(A, B));
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

vec4 gt(vec4 A, vec4 B)
{
	return vec4(greaterThan(A, B));
}

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

vec4 select(vec4 A, vec4 B, vec4 C)
{
	vec4 result;
	result.x = ((A.x > 0.0) ? B.x : C.x);
	result.y = ((A.y > 0.0) ? B.y : C.y);
	result.z = ((A.z > 0.0) ? B.z : C.z);
	result.w = ((A.w > 0.0) ? B.w : C.w);
	return result;
}

vec4 pe_check(vec4 pe, vec4 jag, float cf1, float cf2)
{
	pe.x = (pe.x == cf1 || pe.x == cf2) ? (jag.x > 0.0 ? pe.x : (pe.x - 2.0)) : pe.x;
	pe.y = (pe.y == cf1 || pe.y == cf2) ? (jag.y > 0.0 ? pe.y : (pe.y - 2.0)) : pe.y;
	pe.z = (pe.z == cf1 || pe.z == cf2) ? (jag.z > 0.0 ? pe.z : (pe.z - 2.0)) : pe.z;
	pe.w = (pe.w == cf1 || pe.w == cf2) ? (jag.w > 0.0 ? pe.w : (pe.w - 2.0)) : pe.w;
	return pe;
}

void main()
{
	vec4 PA  = texture2D(decal, v_texcoord2.xw);
	vec4 PB  = texture2D(decal, v_texcoord2.yw);
	vec4 PC  = texture2D(decal, v_texcoord2.zw);

	vec4 PD  = texture2D(decal, v_texcoord3.xw);
	vec4 PE  = texture2D(decal, v_texcoord3.yw);
	vec4 PF  = texture2D(decal, v_texcoord3.zw);

	vec4 PG  = texture2D(decal, v_texcoord4.xw);
	vec4 PH  = texture2D(decal, v_texcoord4.yw);
	vec4 PI  = texture2D(decal, v_texcoord4.zw);

	vec4 A1 = texture2D(ORIG_texture, v_texcoord1.xw);
	vec4 B1 = texture2D(ORIG_texture, v_texcoord1.yw);
	vec4 C1 = texture2D(ORIG_texture, v_texcoord1.zw);

	vec4 A  = texture2D(ORIG_texture, v_texcoord2.xw);
	vec4 B  = texture2D(ORIG_texture, v_texcoord2.yw);
	vec4 C  = texture2D(ORIG_texture, v_texcoord2.zw);

	vec4 D  = texture2D(ORIG_texture, v_texcoord3.xw);
	vec4 E  = texture2D(ORIG_texture, v_texcoord3.yw);
	vec4 F  = texture2D(ORIG_texture, v_texcoord3.zw);

	vec4 G  = texture2D(ORIG_texture, v_texcoord4.xw);
	vec4 H  = texture2D(ORIG_texture, v_texcoord4.yw);
	vec4 I  = texture2D(ORIG_texture, v_texcoord4.zw);

	vec4 G5 = texture2D(ORIG_texture, v_texcoord5.xw);
	vec4 H5 = texture2D(ORIG_texture, v_texcoord5.yw);
	vec4 I5 = texture2D(ORIG_texture, v_texcoord5.zw);

	vec4 A0 = texture2D(ORIG_texture, v_texcoord6.xy);
	vec4 D0 = texture2D(ORIG_texture, v_texcoord6.xz);
	vec4 G0 = texture2D(ORIG_texture, v_texcoord6.xw);

	vec4 C4 = texture2D(ORIG_texture, v_texcoord7.xy);
	vec4 F4 = texture2D(ORIG_texture, v_texcoord7.xz);
	vec4 I4 = texture2D(ORIG_texture, v_texcoord7.xw);

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

	vec4 pe = remapFrom01(PE, maximo);
	vec4 pf = remapFrom01(PF, maximo);
	vec4 ph = remapFrom01(PH, maximo);
	vec4 pb = remapFrom01(PB, maximo);
	vec4 pd = remapFrom01(PD, maximo);

	vec4 f2 = vec4(pf.z, pb.w, pd.x, ph.y);
	vec4 h2 = vec4(ph.z, pf.w, pb.x, pd.y);
	vec4 f1 = vec4(pf.y, pb.z, pd.w, ph.x);
	vec4 h3 = vec4(ph.w, pf.x, pb.y, pd.z);

	vec4 zero = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 one  = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 two  = vec4(2.0, 2.0, 2.0, 2.0);
	vec4 four = vec4(4.0, 4.0, 4.0, 4.0);
	vec4 nbrs = select(gt(pe.yzwx,  one) + gt(pe.wxyz,  one), one, zero);
	vec4 jag1 = select(gt(     f2,  one) + gt(     h2,  one), one, zero);
	vec4 jag2 = select(gt(     f2,  two) + gt(     h2,  two), one, zero);
	vec4 jag3 = select(gt(     f2, four) + gt(     h2, four), one, zero);

	pe = pe_check(pe, jag3, 7.0, 8.0);
	pe = pe_check(pe, jag2, 5.0, 6.0);

	vec4 jag91 = clamp((id(h,i,e,h) + id(i4,i,f4,i4)) * gt(f2, one) * gt(f1, one), 0.0, 1.0);
	vec4 jag92 = clamp((id(f,i,e,f) + id(i5,i,h5,i5)) * gt(h2, one) * gt(h3, one), 0.0, 1.0);
	vec4 jag93 = clamp(rd(h,g,e,g)												 , 0.0, 1.0);
	vec4 jag94 = clamp(rd(f,c,e,c)												 , 0.0, 1.0);
	vec4 jag9  = clamp(jag91 * jag93 + jag92 * jag94							 , 0.0, 1.0);

	vec4 pe_select = eq(pe, zero) + ((one - nbrs) + jag1) * jag9;
	
	pe.x = pe_select.x > 0.0 ? pe.x : 1.0;
	pe.y = pe_select.y > 0.0 ? pe.y : 1.0;
	pe.z = pe_select.z > 0.0 ? pe.z : 1.0;
	pe.w = pe_select.w > 0.0 ? pe.w : 1.0;

	gl_FragColor = vec4(remapTo01(pe, maximo));
}