$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR level 3 pass0 Shader

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

const float coef          = 2.0;
const float cf            = 4.0;
const mat4 yuv_weighted   = mat4(48.0 * vec4(0.299, 0.587, 0.114, 0.0), 7.0 * vec4(-0.169, -0.331, 0.499, 0.0), 6.0 * vec4(0.499, -0.418, -0.0813, 0.0), vec4(0.0, 0.0, 0.0, 0.0));

vec4 df(vec4 A, vec4 B)
{
	return vec4(abs(A-B));
}

vec4 remapTo01(vec4 v, vec4 high)
{
	return (v/high);
}

vec4 remapFrom01(vec4 v, vec4 high)
{
	return (high*v + vec4(0.5, 0.5, 0.5, 0.5));
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

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

#define FILTRO(EDR0, EDR, LEFT, UP, LEFT3, UP3, PX0, PX3, PX1, LIN0, LIN3, LIN1, PX)	\
	if (LEFT > 0.0 && UP == 0.0)			\
	{										\
		PX0  = vec2(0.0, PX);				\
		PX3  = vec2( PX, 0.0);				\
		if (LEFT3 > 0.0)					\
		{									\
			LIN0 = vec4(0.0, 1.0, 0.0, 0.0);\
			LIN3 = vec4(1.0, 0.0, 0.0, 0.0);\
		}									\
		else								\
		{									\
			LIN0 = vec4(0.0, 0.0, 1.0, 0.0);\
			LIN3 = vec4(0.0, 1.0, 1.0, 0.0);\
		}									\
	}										\
	else if (UP > 0.0 && LEFT == 0.0)		\
	{										\
		PX0  = vec2(0.0, 1.0 - PX);			\
		PX1  = vec2(1.0 - PX, 0.0);			\
		if (UP3 > 0.0)						\
		{									\
			LIN0 = vec4(0.0, 1.0, 0.0, 1.0);\
			LIN1 = vec4(1.0, 0.0, 0.0, 1.0);\
		}									\
		else								\
		{									\
			LIN0 = vec4(0.0, 0.0, 1.0, 1.0);\
			LIN1 = vec4(0.0, 1.0, 1.0, 1.0);\
		}									\
	}										\
	else if (EDR > 0.0)						\
	{										\
		LEFT = UP = LEFT3 = UP3 = 0.0;		\
		PX0  = vec2(0.0, PX);				\
		LIN0 = vec4(0.0, 0.0, 0.0, 1.0);	\
	}										\
	else if (EDR0 > 0.0)					\
	{										\
		LEFT = UP = LEFT3 = UP3 = 0.0;		\
		PX0  = vec2(0.0, PX);				\
		LIN0 = vec4(0.0, 0.0, 0.0, 0.0);	\
	}



void main()
{
	vec2 px0, px1, px2, px3;

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

	vec4 c1 = i4.yzwx;
	vec4 g0 = i5.wxyz;
	vec4 b1 = h5.zwxy;
	vec4 d0 = h5.wxyz;

	vec4 interp_restriction_lv0      = ne(e,f) * ne(e,h);
	vec4 interp_restriction_lv1      = (ge(f,b) * ge(f,c) + ge(h,d) * ge(h,g) + lt(e,i) * (ge(f,f4) * ge(f,i4) + ge(h,h5) * ge(h,i5)) + lt(e,g) + lt(e,c));
	vec4 interp_restriction_lv2_left = ne(e,g)  * ne(d,g);
	vec4 interp_restriction_lv2_up   = ne(e,c)  * ne(b,c);
	vec4 interp_restriction_lv3_left = ne(e,g0) * ne(d0,g0);
	vec4 interp_restriction_lv3_up   = ne(e,c1) * ne(b1,c1);

	vec4 edr0 = lt(weighted_distance(e, c, g, i, h5, f4, h, f), weighted_distance(h, d, i5, f, i4, b, e, i)) * interp_restriction_lv0;

	vec4 edr       = edr0 * interp_restriction_lv1;
	vec4 edr_left  = le(coef * df(f,g), df(h,c)) * interp_restriction_lv2_left * edr;
	vec4 edr_up    = ge(df(f,g), coef * df(h,c)) * interp_restriction_lv2_up   * edr;
	vec4 edr3_left = le(cf * df(f,g0), df(h,c1)) * interp_restriction_lv3_left * edr_left;
	vec4 edr3_up   = ge(df(f,g0), cf * df(h,c1)) * interp_restriction_lv3_up   * edr_up;

	vec4 px  = le(df(e,f), df(e,h));

	vec4 lin0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 lin1 = lin0;
	vec4 lin2 = lin0;
	vec4 lin3 = lin0;

	FILTRO(edr0.x, edr.x, edr_left.x, edr_up.x, edr3_left.x, edr3_up.x, px0, px3, px1, lin0, lin3, lin1, px.x);
	FILTRO(edr0.y, edr.y, edr_left.y, edr_up.y, edr3_left.y, edr3_up.y, px1, px0, px2, lin1, lin0, lin2, px.y);
	FILTRO(edr0.z, edr.z, edr_left.z, edr_up.z, edr3_left.z, edr3_up.z, px2, px1, px3, lin2, lin1, lin3, px.z);
	FILTRO(edr0.w, edr.w, edr_left.w, edr_up.w, edr3_left.w, edr3_up.w, px3, px2, px0, lin3, lin2, lin0, px.w);

	vec4 info = mul(vec4(1.0, 2.0, 4.0, 8.0), mat4(	edr3_left,
													edr3_up,
													px0.x,  px1.x,  px2.x,  px3.x,
													px0.y,  px1.y,  px2.y,  px3.y));

	info += mul(vec4(16.0, 32.0, 64.0, 128.0), mat4(lin0.x, lin1.x, lin2.x, lin3.x,
													lin0.y, lin1.y, lin2.y, lin3.y,
													lin0.z, lin1.z, lin2.z, lin3.z,
													lin0.w, lin1.w, lin2.w, lin3.w));

	gl_FragColor = vec4(remapTo01(info, vec4(255.0, 255.0, 255.0, 255.0)));
}
