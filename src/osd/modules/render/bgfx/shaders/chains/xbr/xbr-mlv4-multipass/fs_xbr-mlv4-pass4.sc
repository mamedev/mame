$input v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*

   Hyllian's xBR MultiLevel4 Shader - Pass3
   
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

uniform vec4 u_tex_size1;
uniform vec4 u_target_size;

#define round(X) floor((X)+0.5)

const vec3 bin       = vec3( 4.0f,  2.0f,  1.0f);
const vec4 low       = vec4(-64.0f, -64.0f, -64.0f, -64.0f);
const vec4 high      = vec4( 64.0f,  64.0f,  64.0f,  64.0f);

const mat4 sym_vectors = mat4 (1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

vec4 remapFrom01(vec4 v, vec4 low, vec4 high)
{
	return round(mix(low, high, v));
}

float c_df(vec3 c1, vec3 c2)
{
	vec3 df = abs(c1 - c2);
	return df.r + df.g + df.b;
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

float df(float A, float B)
{
	return abs(A-B);
}

vec3 color_mix(vec3 c, vec3 color, vec3 E)
{
	return (c_df(c, E) > c_df(color, E) ? c : color);
}

#define GET_PIXEL(PARAM, PIXEL)			\
	info = PARAM;						\
										\
	frac_val = fract(info / 2.0);	\
	ay.z = round(frac_val);				\
	info = info / 2.0 - frac_val;		\
										\
	frac_val = fract(info / 2.0);		\
	ay.y = round(frac_val);				\
	info = info / 2.0 - frac_val;		\
										\
	frac_val = fract(info / 2.0);		\
	ay.x = round(frac_val);				\
	info = info / 2.0 - frac_val;		\
										\
	frac_val = fract(info / 2.0);		\
	ax.z = round(frac_val);				\
	info = info / 2.0 - frac_val;		\
										\
	frac_val = fract(info / 2.0);		\
	ax.y = round(frac_val);				\
	info = info / 2.0 - frac_val;		\
										\
	ax.x = round(info);					\
	iq.x = dot(ax, bin) - 2.0;			\
	iq.y = dot(ay, bin) - 2.0;			\
	PIXEL = texture2D(ORIG_texture, v_texcoord0 + iq.x * v_texcoord1.xy + iq.y * v_texcoord1.zw).xyz;	\


void main()
{
	float scale_factor = u_target_size.x / u_tex_size1.x;
	
	vec2 fp = fract(v_texcoord0 * u_tex_size1.xy) - vec2(0.5, 0.5); // pos = pixel position

	vec4 UL = texture2D(decal, v_texcoord0 + 0.25 * v_texcoord1.xy + 0.25 * v_texcoord1.zw);
	vec4 UR = texture2D(decal, v_texcoord0 + 0.75 * v_texcoord1.xy + 0.25 * v_texcoord1.zw);
	vec4 DL = texture2D(decal, v_texcoord0 + 0.25 * v_texcoord1.xy + 0.75 * v_texcoord1.zw);
	vec4 DR = texture2D(decal, v_texcoord0 + 0.75 * v_texcoord1.xy + 0.75 * v_texcoord1.zw);

	vec4 ulparam = remapFrom01(UL, low, high); // retrieve 1st pass info
	vec4 urparam = remapFrom01(UR, low, high); // retrieve 1st pass info
	vec4 dlparam = remapFrom01(DL, low, high); // retrieve 1st pass info
	vec4 drparam = remapFrom01(DR, low, high); // retrieve 1st pass info

	vec4 E = texture2D(ORIG_texture, v_texcoord0);

	float info, frac_val;
	vec2 iq;
	vec3 ax, ay, PX, PY, PZ, PW;
	GET_PIXEL(ulparam.w, PX);
	GET_PIXEL(urparam.w, PY);
	GET_PIXEL(dlparam.w, PZ);
	GET_PIXEL(drparam.w, PW);

	vec3 fp1 = vec3(fp, -1);

	vec4 inc   = vec4(abs(ulparam.x / ulparam.y), abs(urparam.x / urparam.y), abs(dlparam.x / dlparam.y), abs(drparam.x / drparam.y));
	vec4 level = max(inc, 1.0 / inc);

	vec4 fx;
	fx.x = saturate(dot(fp1, ulparam.xyz) * scale_factor / (8.0 * level.x) + 0.5);
	fx.y = saturate(dot(fp1, urparam.xyz) * scale_factor / (8.0 * level.y) + 0.5);
	fx.z = saturate(dot(fp1, dlparam.xyz) * scale_factor / (8.0 * level.z) + 0.5);
	fx.w = saturate(dot(fp1, drparam.xyz) * scale_factor / (8.0 * level.w) + 0.5);

	vec3 c1 = mix(E.xyz, PX, fx.x);
	vec3 c2 = mix(E.xyz, PY, fx.y);
	vec3 c3 = mix(E.xyz, PZ, fx.z);
	vec3 c4 = mix(E.xyz, PW, fx.w);

	vec3 color = c1;
	color = color_mix(c2, color, E.xyz);
	color = color_mix(c3, color, E.xyz);
	color = color_mix(c4, color, E.xyz);

	gl_FragColor = vec4(color, 1.0);
}