$input v_texcoord0, v_texcoord1, v_texcoord2, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR LV2 - pass1 Shader
   
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
SAMPLER2D(ORIG_texture, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_target_dims;

#define round(X) floor((X)+0.5)

const float coef          = 2.0;
const vec4 eq_threshold = vec4(15.0, 15.0, 15.0, 15.0);
const float y_weight      = 48.0;
const float u_weight      = 7.0;
const float v_weight      = 6.0;
const mat4 yuv            = mat4(
	 0.299,  0.587,  0.114,  0.0,
	-0.169, -0.331,  0.499,  0.0,
	 0.499, -0.418, -0.0813, 0.0,
	 0.0, 0.0, 0.0, 0.0
);

const mat4 yuv_weighted   = mat4(
	48.0 * vec4( 0.299,  0.587,  0.114,  0.0),
	 7.0 * vec4(-0.169, -0.331,  0.499,  0.0),
	 6.0 * vec4( 0.499, -0.418, -0.0813, 0.0),
	 0.0 * vec4( 0.0,    0.0,    0.0,    0.0)
);

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

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}


vec3 remapFrom01(vec3 v, vec3 low, vec3 high)
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
	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);

	vec4 B  = texture2D(ORIG_texture, v_texcoord1.xy);
	vec4 D  = texture2D(ORIG_texture, v_texcoord2.xw);
	vec4 E  = texture2D(ORIG_texture, v_texcoord2.yw);
	vec4 F  = texture2D(ORIG_texture, v_texcoord2.zw);
	vec4 H  = texture2D(ORIG_texture, v_texcoord1.xw);

	vec4 b = mul(mat4(B, D, H, F), yuv_weighted[0]);
	vec4 e = mul(mat4(E, E, E, E), yuv_weighted[0]);
	vec4 d = b.yzwx;
	vec4 f = b.wxyz;
	vec4 h = b.zwxy;

	vec4 Ao = vec4(1.0, -1.0, -1.0,  1.0);
	vec4 Bo = vec4(1.0,  1.0, -1.0, -1.0);
	vec4 Co = vec4(1.5,  0.5, -0.5,  0.5);
	vec4 Ax = vec4(1.0, -1.0, -1.0,  1.0);
	vec4 Bx = vec4(0.5,  2.0, -0.5, -2.0);
	vec4 Cx = vec4(1.0,  1.0, -0.5,  0.0);
	vec4 Ay = vec4(1.0, -1.0, -1.0,  1.0);
	vec4 By = vec4(2.0,  0.5, -2.0, -0.5);
	vec4 Cy = vec4(2.0,  0.0, -1.0,  0.5);

	// These inequations define the line below which interpolation occurs.
	vec4 fx      = (Ao*fp.y+Bo*fp.x); 
	vec4 fx_left = (Ax*fp.y+Bx*fp.x);
	vec4 fx_up   = (Ay*fp.y+By*fp.x);

	vec2 delta = vec2(u_tex_size1.x / u_target_dims.x, 0.5 * u_tex_size1.x / u_target_dims.x);

	vec4 fx45 = saturate((fx      + delta.xxxx - Co) / (2.0 * delta.xxxx));
	vec4 fx30 = saturate((fx_left + delta.yxyx - Cx) / (2.0 * delta.yxyx));
	vec4 fx60 = saturate((fx_up   + delta.xyxy - Cy) / (2.0 * delta.xyxy));

	vec4 info  = texture2D(decal, v_texcoord0);

	vec3 i = remapFrom01(info.xyz, vec3(0.0, 0.0, 0.0), vec3(255.0, 255.0, 255.0));

	vec4 edr      = unpack_info(i.x);
	vec4 edr_left = unpack_info(i.y);
	vec4 edr_up   = unpack_info(i.z);

	fx45 = edr * fx45;
	fx30 = edr_left * fx30;
	fx60 = edr_up * fx60;

	vec4 px = le(df(e,f), df(e,h));

	vec4 maximo = max(max(fx30, fx60), fx45);

	mat4 pix = mat4(mix(E, mix(H, F, px.x), maximo.x), mix(E, mix(F, B, px.y), maximo.y), mix(E, mix(B, D, px.z), maximo.z), mix(E, mix(D, H, px.w), maximo.w));
	vec4 pixel = mul(pix, yuv_weighted[0]);
	
	vec4 diff = df(pixel,e);

	vec3 res = pix[0].xyz;
	float mx = diff.x;

	if (diff.y > mx)
	{
		res = pix[1].xyz;
		mx = diff.y;
	}
	if (diff.z > mx)
	{
		res = pix[2].xyz;
		mx = diff.z;
	}
	if (diff.w > mx)
	{
		res = pix[3].xyz;
	}

	gl_FragColor = vec4(res, 1.0);
}