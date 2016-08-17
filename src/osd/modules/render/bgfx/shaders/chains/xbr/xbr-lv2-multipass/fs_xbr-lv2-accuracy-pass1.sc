$input v_texcoord0, v_texcoord1, v_texcoord2, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR LV2 Accuracy - pass1 Shader
   
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
SAMPLER2D(ORIG_texture, 1);

uniform vec4 XBR_SCALE;
uniform vec4 u_tex_size0;

#define XBR_RED_COEF 17.0
#define XBR_GREEN_COEF 20.0
#define XBR_BLUE_COEF 3.0

#define round(X) floor((X)+0.5)

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


float df1(vec4 A, vec4 B)
{
	float rmean = (A.r + B.r)/2.0;
	vec4 diff = A - B;
	vec4 K = vec4(XBR_RED_COEF+rmean, XBR_GREEN_COEF, XBR_BLUE_COEF-rmean, 0.0);

	return sqrt(dot(K * diff, diff));
}

vec4 df(mat4 A, mat4 B)
{
	return vec4(df1(A[0],B[0]), df1(A[1],B[1]), df1(A[2],B[2]), df1(A[3],B[3]));
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec4 remapFrom01(vec4 v, vec4 low, vec4 high)
{
	return round(mix(low, high, v));
}

vec4 unpack_info(float i)
{
	vec4 info;
	float frac_val = fract(i / 2.0f);
	info.w = round(frac_val);
	i = i / 2.0f - frac_val;
	
	frac_val = fract(i / 2.0f);
	info.z = round(frac_val);
	i = i / 2.0f - frac_val;

	frac_val = fract(i / 2.0f);
	info.y = round(frac_val);
	info.x = i / 2.0f - frac_val;

	return info;
}

void main()
{
	vec4 delta  = 1.0 / XBR_SCALE.xxxx;
	vec4 deltaL = vec4(0.5, 1.0, 0.5, 1.0) / XBR_SCALE.xxxx;
	vec4 deltaU = deltaL.yxwz;

	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);

	vec4 B  = texture2D(ORIG_texture, v_texcoord1.xy);
	vec4 D  = texture2D(ORIG_texture, v_texcoord2.xw);
	vec4 E  = texture2D(ORIG_texture, v_texcoord2.yw);
	vec4 F  = texture2D(ORIG_texture, v_texcoord2.zw);
	vec4 H  = texture2D(ORIG_texture, v_texcoord1.xw);

	mat4 b = mat4(B, D, H, F);
	mat4 e = mat4(E, E, E, E);
	mat4 d = mat4(D, H, F, B);
	mat4 f = mat4(F, B, D, H);
	mat4 h = mat4(H, F, B, D);

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = (Ao*fp.y+Bo*fp.x); 
	vec4 fx_left = (Ax*fp.y+Bx*fp.x);
	vec4 fx_up   = (Ay*fp.y+By*fp.x);

	vec4 fx45i = saturate((fx      + delta  -Co - Ci) / (2.0 * delta ));
	vec4 fx45  = saturate((fx      + delta  -Co     ) / (2.0 * delta ));
	vec4 fx30  = saturate((fx_left + deltaL -Cx     ) / (2.0 * deltaL));
	vec4 fx60  = saturate((fx_up   + deltaU -Cy     ) / (2.0 * deltaU));

	vec4 info  = texture2D(decal, v_texcoord0);

	vec4 i = remapFrom01(info, vec4(0.0, 0.0, 0.0, 0.0), vec4(255.0, 255.0, 255.0, 255.0));

	vec4 edr      = unpack_info(i.x);
	vec4 edr_left = unpack_info(i.y);
	vec4 edr_up   = unpack_info(i.z);
	vec4 edri     = unpack_info(i.w);

	fx45i = edri * fx45i;
	fx45  = edr * fx45;
	fx30  = edr_left * fx30;
	fx60  = edr_up * fx60;

	vec4 px = le(df(e,f), df(e,h));

	vec4 maximos = max(max(fx30, fx60), max(fx45, fx45i));

	vec4 res1 = E;
	res1 = mix(res1, mix(H, F, px.x), maximos.x);
	res1 = mix(res1, mix(B, D, px.z), maximos.z);
	
	vec4 res2 = E;
	res2 = mix(res2, mix(F, B, px.y), maximos.y);
	res2 = mix(res2, mix(D, H, px.w), maximos.w);
	
	vec3 E_mix = (df1(E, res2) >= df1(E, res1)) ? vec3(1.0, 1.0, 1.0) : vec3(0.0, 0.0, 0.0);
	vec3 res = mix(res1.xyz, res2.xyz, E_mix);

	gl_FragColor = vec4(res, 1.0);
}