$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR-lv3 Shader
   
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

uniform vec4 XBR_Y_WEIGHT;
uniform vec4 XBR_EQ_THRESHOLD;
uniform vec4 XBR_EQ_THRESHOLD2;
uniform vec4 XBR_LV2_COEFFICIENT;

uniform vec4 u_tex_size0;

SAMPLER2D(decal, 0);

const vec4 yuv   = vec4(0.299, 0.587, 0.114, 0.0);
const vec4 delta = vec4(0.4, 0.4, 0.4, 0.4);

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

vec4 abslt2(vec4 A, vec4 B)
{
	return lt(df(A, B), XBR_EQ_THRESHOLD2.xxxx);
}

vec4 absge(vec4 A, vec4 B)
{
	return ge(df(A, B), XBR_EQ_THRESHOLD.xxxx);
}

vec4 absge2(vec4 A, vec4 B)
{
	return ge(df(A, B), XBR_EQ_THRESHOLD2.xxxx);
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);

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

	vec4 b = mul(mat4(B, D, H, F), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 c = mul(mat4(C, A, G, I), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 e = mul(mat4(E, E, E, E), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 g = c.zwxy;
	vec4 h = b.zwxy;
	vec4 i = c.wxyz;

	vec4 i4 = mul(mat4(I4, C1, A0, G5), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 i5 = mul(mat4(I5, C4, A1, G0), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 h5 = mul(mat4(H5, F4, B1, D0), XBR_Y_WEIGHT.xxxx * yuv);
	vec4 f4 = h5.yzwx;

	vec4 c1 = i4.yzwx;
	vec4 g0 = i5.wxyz;
	vec4 b1 = h5.zwxy;
	vec4 d0 = h5.wxyz;

	vec4 Ao = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 Bo = vec4( 1.0,  1.0, -1.0,-1.0 );
	vec4 Co = vec4( 1.5,  0.5, -0.5, 0.5 );
	vec4 Ax = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 Bx = vec4( 0.5,  2.0, -0.5,-2.0 );
	vec4 Cx = vec4( 1.0,  1.0, -0.5, 0.0 );
	vec4 Ay = vec4( 1.0, -1.0, -1.0, 1.0 );
	vec4 By = vec4( 2.0,  0.5, -2.0,-0.5 );
	vec4 Cy = vec4( 2.0,  0.0, -1.0, 0.5 );

	vec4 Az = vec4( 6.0, -2.0, -6.0, 2.0 );
	vec4 Bz = vec4( 2.0, 6.0, -2.0, -6.0 );
	vec4 Cz = vec4( 5.0, 3.0, -3.0, -1.0 );
	vec4 Aw = vec4( 2.0, -6.0, -2.0, 6.0 );
	vec4 Bw = vec4( 6.0, 2.0, -6.0,-2.0 );
	vec4 Cw = vec4( 5.0, -1.0, -3.0, 3.0 );

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = (Ao * fp.y + Bo * fp.x); 
	vec4 fx_left = (Ax * fp.y + Bx * fp.x);
	vec4 fx_up   = (Ay * fp.y + By * fp.x);
	vec4 fx3_left= (Az * fp.y + Bz * fp.x);
	vec4 fx3_up  = (Aw * fp.y + Bw * fp.x);

	vec4 interp_restriction_lv0 = (ne(e,f) * ne(e,h));

// It uses CORNER_C if none of the others are defined.
#ifdef CORNER_A
	vec4 interp_restriction_lv1 = interp_restriction_lv0;
#elif CORNER_B
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * ( ge(f,b) * ge(h,d) + lt(e,i) * ge(f,i4) * ge(h,i5) + lt(e,g) + lt(e,c) ) );
#elif CORNER_D
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * ( ge(f,b) * ge(h,d) + lt(e,i) * ge(f,i4) * ge(h,i5) + lt(e,g) + lt(e,c) ) * (f!=f4 * f!=i + h!=h5 * h!=i + h!=g + f!=c + lt(b,c1) * lt(d,g0)));
#else
	vec4 interp_restriction_lv1 = (interp_restriction_lv0 * ( ge(f,b) * ge(f,c) + ge(h,d) * ge(h,g) + lt(e,i) * (ge(f,f4) * ge(f,i4) + ge(h,h5) * ge(h,i5)) + lt(e,g) + lt(e,c)) );
#endif

	interp_restriction_lv1 = clamp(interp_restriction_lv1, 0.0, 1.0);
	
	vec4 interp_restriction_lv2_left = ne(e,g) * ne(d,g);
	vec4 interp_restriction_lv2_up   = ne(e,c) * ne(b,c);
	vec4 interp_restriction_lv3_left = abslt2(g,g0) * absge2(d0,g0);
	vec4 interp_restriction_lv3_up   = abslt2(c,c1) * absge2(b1,c1);

	vec4 fx45 = smoothstep(Co - delta, Co + delta, fx);
	vec4 fx30 = smoothstep(Cx - delta, Cx + delta, fx_left);
	vec4 fx60 = smoothstep(Cy - delta, Cy + delta, fx_up);
	vec4 fx15 = smoothstep(Cz - delta, Cz + delta, fx3_left);
	vec4 fx75 = smoothstep(Cw - delta, Cw + delta, fx3_up);

	vec4 edr      = lt(weighted_distance(e, c, g, i, h5, f4, h, f), weighted_distance( h, d, i5, f, i4, b, e, i)) * interp_restriction_lv1;
	vec4 edr_left = le(XBR_LV2_COEFFICIENT.xxxx * df(f,g), df(h,c)) * interp_restriction_lv2_left;
	vec4 edr_up   = ge(df(f,g), XBR_LV2_COEFFICIENT.xxxx * df(h,c)) * interp_restriction_lv2_up;
	
	vec4 edr3_left = interp_restriction_lv3_left;
	vec4 edr3_up = interp_restriction_lv3_up;

	vec4 nc45 = edr *            fx45;
	vec4 nc30 = edr * edr_left * fx30;
	vec4 nc60 = edr * edr_up   * fx60;
	vec4 nc15 = edr * edr_left * edr3_left * fx15;
	vec4 nc75 = edr * edr_up   * edr3_up   * fx75;

	vec4 px = le(df(e,f), df(e,h));

	vec4 nc = nc75 + nc15 + nc30 + nc60 + nc45;

	vec4 final45 = nc45 * fx45;
	vec4 final30 = nc30 * fx30;
	vec4 final60 = nc60 * fx60;
	vec4 final15 = nc15 * fx15;
	vec4 final75 = nc75 * fx75;

	vec4 maximo = max(max(max(final15, final75),max(final30, final60)), final45);

	float blend1;
	vec3 pix1;
	     if (nc.x > 0.0) {pix1 = px.x > 0.0 ? F.xyz : H.xyz; blend1 = maximo.x;}
	else if (nc.y > 0.0) {pix1 = px.y > 0.0 ? B.xyz : F.xyz; blend1 = maximo.y;}
	else if (nc.z > 0.0) {pix1 = px.z > 0.0 ? D.xyz : B.xyz; blend1 = maximo.z;}
	else if (nc.w > 0.0) {pix1 = px.w > 0.0 ? H.xyz : D.xyz; blend1 = maximo.w;}

	float blend2;
	vec3 pix2;
	     if (nc.w > 0.0) {pix2 = px.w > 0.0 ? H.xyz : D.xyz; blend2 = maximo.w;}
	else if (nc.z > 0.0) {pix2 = px.z > 0.0 ? D.xyz : B.xyz; blend2 = maximo.z;}
	else if (nc.y > 0.0) {pix2 = px.y > 0.0 ? B.xyz : F.xyz; blend2 = maximo.y;}
	else if (nc.x > 0.0) {pix2 = px.x > 0.0 ? F.xyz : H.xyz; blend2 = maximo.x;}

	vec3 res1 = mix(E.xyz, pix1, blend1);
	vec3 res2 = mix(E.xyz, pix2, blend2);

	vec3 E_mix = (c_df(E.xyz, res2) >= c_df(E.xyz, res1)) ? vec3(1.0, 1.0, 1.0) : vec3(0.0, 0.0, 0.0);
	vec3 res = mix(res1, res2, E_mix);

	gl_FragColor = vec4(res, 1.0);
}