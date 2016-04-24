$input v_texcoord0, v_texcoord1, v_texcoord2, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR level 3 pass1 Shader
   
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

uniform vec4 u_tex_size0;

#define round(X) floor((X)+0.5)

const mat4 sym_vectors  = mat4(1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

const vec3 lines0 = vec3(1.0, 1.0, 0.75);
const vec3 lines1 = vec3(1.0, 1.0, 0.5);
const vec3 lines2 = vec3(2.0, 1.0, 0.5);
const vec3 lines3 = vec3(1.0, 2.0, 0.5);
const vec3 lines4 = vec3(3.0, 1.0, 0.5);
const vec3 lines5 = vec3(1.0, 3.0, 0.5);

const vec3 lines6 = vec3(-1.0,  2.0, 0.5);
const vec3 lines7 = vec3( 2.0, -1.0, 0.5);
const vec3 lines8 = vec3(-1.0,  3.0, 0.5);
const vec3 lines9 = vec3( 3.0, -1.0, 0.5);

const vec3 lines10 = vec3(3.0, 1.0, 1.5);
const vec3 lines11 = vec3(1.0, 3.0, 1.5);

float remapFrom01(float v, float high)
{
	return (high*v + 0.5);
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
	vec2 pos = fract(v_texcoord0 * u_tex_size0.xy) - vec2(0.5, 0.5); // pos = pixel position
	vec4 dir = vec4(sign(pos), 0.0, 0.0); // dir = pixel direction

	vec2 g1 = dir.xy *( saturate(-dir.y * dir.x) * v_texcoord1.zw + saturate( dir.y * dir.x) * v_texcoord1.xy);
	vec2 g2 = dir.xy *( saturate( dir.y * dir.x) * v_texcoord1.zw + saturate(-dir.y * dir.x) * v_texcoord1.xy);

	vec4 F   = texture2D(ORIG_texture, v_texcoord0 +g1);
	vec4 B   = texture2D(ORIG_texture, v_texcoord0 -g2);
	vec4 D   = texture2D(ORIG_texture, v_texcoord0 -g1);
	vec4 H   = texture2D(ORIG_texture, v_texcoord0 +g2);
	vec4 E   = texture2D(ORIG_texture, v_texcoord0    );

	vec4 F4  = texture2D(ORIG_texture, v_texcoord0 +2.0*g1);
	vec4 I   = texture2D(ORIG_texture, v_texcoord0  +g1+g2);
	vec4 H5  = texture2D(ORIG_texture, v_texcoord0 +2.0*g2);

	vec4 icomp    = round(saturate(mul(dir, sym_vectors))); // choose info component
	float  info     = remapFrom01(dot(texture2D(decal, v_texcoord0     ), icomp), 255.0); // retrieve 1st pass info
	float  info_nr  = remapFrom01(dot(texture2D(decal, v_texcoord0 + g1), icomp), 255.0); // 1st pass info from neighbor r
	float  info_nd  = remapFrom01(dot(texture2D(decal, v_texcoord0 + g2), icomp), 255.0); // 1st pass info from neighbor d

	info = info / 2.0 - fract(info / 2.0);
	info = info / 2.0 - fract(info / 2.0);
	vec2 px;

	float frac_val = fract(info / 2.0);
	px.x = round(frac_val);
	info = info / 2.0 - frac_val;

	frac_val = fract(info / 2.0);
	px.y = round(frac_val);
	info = info / 2.0 - frac_val;

	vec4 flags = unpack_info(info); // retrieve 1st pass flags

	frac_val = fract(info_nr / 2.0);
	float edr3_nrl = round(frac_val);
	info_nr = info_nr / 2.0 - frac_val;
	info_nr = info_nr / 2.0 - fract(info_nr / 2.0);
	info_nr = info_nr / 2.0 - fract(info_nr / 2.0);
	
	frac_val = fract(info_nr / 2.0);
	float pxr = round(frac_val);
	info_nr = info_nr / 2.0 - frac_val;

	info_nd = info_nd / 2.0 - fract(info_nd / 2.0);

	frac_val = fract(info_nd / 2.0);
	float edr3_ndu = round(frac_val);
	info_nd = info_nd / 2.0 - frac_val;

	info_nd = info_nd / 2.0 - fract(info_nd / 2.0);

	frac_val = fract(info_nd / 2.0);
	float pxd = round(frac_val);
	info_nd = info_nd / 2.0 - frac_val;

	float aux = round(dot(vec4(8.0, 4.0, 2.0, 1.0), flags));
	vec3 slep;

	if (aux >= 6.0)
	{
		slep = (aux == 6.0 ? lines6 : (aux == 7.0 ? lines7 : (aux == 8.0 ? lines8 : (aux == 9.0 ? lines9 : (aux == 10.0 ? lines10 : lines11)))));
	}
	else
	{
		slep = (aux == 0.0 ? lines0 : (aux == 1.0 ? lines1 : (aux == 2.0 ? lines2 : (aux == 3.0 ? lines3 : (aux == 4.0 ? lines4 : lines5)))));
	}

	vec2 fp = (dir.x * dir.y) > 0.0 ? abs(pos) : abs(pos.yx);

	vec3 fp1 = vec3(fp.yx, -1);

	vec3 color = E.xyz;
	float fx;

	if (aux < 10.0)
	{
		fx    = saturate(dot(fp1, slep) / (2.0 * v_texcoord2.x) + 0.5);
		color = mix(E.xyz, mix(mix(H.xyz, F.xyz, px.y), mix(D.xyz, B.xyz, px.y), px.x), fx); // interpolate if there's edge
	}
	else if (edr3_nrl > 0.0)
	{
		fx    = saturate(dot(fp1, lines10) / (2.0 * v_texcoord2.x) + 0.5);
		color = mix(E.xyz, mix(I.xyz, F4.xyz, pxr), fx); // interpolate if there's edge
	}
	else if (edr3_ndu > 0.0)
	{
		fx    = saturate(dot(fp1, lines11) / (2.0 * v_texcoord2.x) + 0.5);
		color = mix(E.xyz, mix(H5.xyz, I.xyz, pxd), fx); // interpolate if there's edge
	}

	gl_FragColor = vec4(color, 1.0);
}