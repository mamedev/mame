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

#define round(X) floor((X)+0.5)

const float coef         = 2.0;
const float cf           = 4.0;
const float eq_threshold = 15.0;
const mat4 yuv_weighted  = mat4(48.0 * vec4(0.299, 0.587, 0.114, 0.0), 7.0 * vec4(-0.169, -0.331, 0.499, 0.0), 6.0 * vec4(0.499, -0.418, -0.0813, 0.0), vec4(0.0, 0.0, 0.0, 0.0));
const vec4 maximo        = vec4(255.0, 255.0, 255.0, 255.0);
const vec4 low           = vec4(-64.0, -64.0, -64.0, -64.0);
const vec4 high          = vec4( 64.0,  64.0,  64.0,  64.0);

const mat4 sym_vectors = mat4(1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

// Bx, Ay, C
const vec3 lines0 =  vec3( 4.0, 4.0, 4.0);  //  0  NL
const vec3 lines1 =  vec3( 4.0, 4.0, 3.0);  //  1  LV0
const vec3 lines2 =  vec3( 4.0, 4.0, 2.0);  //  2  LV1
const vec3 lines3 =  vec3( 8.0, 4.0, 2.0);  //  3  LV2u
const vec3 lines4 =  vec3( 4.0, 8.0, 2.0);  //  4  LV2l
const vec3 lines5 =  vec3(12.0, 4.0, 2.0);  //  5  LV3u
const vec3 lines6 =  vec3( 4.0,12.0, 2.0);  //  6  LV3l
const vec3 lines7 =  vec3(16.0, 4.0, 2.0);  //  7  LV4u
const vec3 lines8 =  vec3( 4.0,16.0, 2.0);  //  8  LV4l

const vec3 lines9 =  vec3(12.0, 4.0, 6.0);  //  9  LV3u
const vec3 lines10 = vec3( 4.0,12.0, 6.0);  // 10  LV3l
const vec3 lines11 = vec3(16.0, 4.0, 6.0);  // 11  LV4u
const vec3 lines12 = vec3( 4.0,16.0, 6.0);  // 12  LV4l

vec4 remapTo01(vec4 v, vec4 low, vec4 high)
{
	return saturate((v - low)/(high-low));
}

float remapFrom01(float v, float high)
{
	return round(high*v);
}

float df(float A, float B)
{
	return abs(A-B);
}

float weighted_distance(float a, float b, float c, float d, float e, float f, float g, float h)
{
	return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

void main()
{
	vec2 pos = fract(v_texcoord0 * u_tex_size1.xy) - vec2(0.5, 0.5); // pos = pixel position
	vec4 dir = vec4(sign(pos), 0.0, 0.0);

	vec2 g1 = dir.xy * (saturate(-dir.y * dir.x) * v_texcoord1.zw + saturate( dir.y * dir.x) * v_texcoord1.xy);
	vec2 g2 = dir.xy * (saturate( dir.y * dir.x) * v_texcoord1.zw + saturate(-dir.y * dir.x) * v_texcoord1.xy);

	vec4 E   = texture2D(ORIG_texture, v_texcoord0    );
	vec4 F   = texture2D(ORIG_texture, v_texcoord0 +g1);
	vec4 H   = texture2D(ORIG_texture, v_texcoord0 +g2);
	vec4 I   = texture2D(ORIG_texture, v_texcoord0 +g1+g2);
	vec4 F4  = texture2D(ORIG_texture, v_texcoord0 +2.0*g1);
	vec4 H5  = texture2D(ORIG_texture, v_texcoord0 +2.0*g2);

	float e = dot(E, yuv_weighted[0]);
	float f = dot(F, yuv_weighted[0]);
	float h = dot(H, yuv_weighted[0]);
	float i = dot(I, yuv_weighted[0]);
	float f4= dot(F4, yuv_weighted[0]);
	float h5= dot(H5, yuv_weighted[0]);

	vec4 icomp    = round(saturate(mul(dir, sym_vectors))); // choose info component

	float infoE    = remapFrom01(dot(texture2D(decal, v_texcoord0   ), icomp), 255.0); // retrieve 1st pass info
	float infoF    = remapFrom01(dot(texture2D(decal, v_texcoord0+g1), icomp), 255.0); // 1st pass info from neighbor r
	float infoH    = remapFrom01(dot(texture2D(decal, v_texcoord0+g2), icomp), 255.0); // 1st pass info from neighbor d

	vec4 lparam;
	float px;
	vec2 addr;

	if (infoF == 8.0)
	{
		lparam.xyz = lines12;
		px = float(df(f,f4) <= df(f,i));
		addr.x = 2.0 * px + saturate(1.0 - px);
		addr.y = saturate(1.0-px);
	}
	else if (infoH == 7.0)
	{
		lparam.xyz = lines11;
		px = float(df(h,h5) <= df(h,i));
		addr.x = saturate(1.0-px);
		addr.y = 2.0 * px + saturate(1.0 - px);
	}
	else if (infoF == 6.0)
	{
		lparam.xyz = lines10;
		px = float(df(f,f4) <= df(f,i));
		addr.x = 2.0 * px + saturate(1.0 - px);
		addr.y = saturate(1.0-px);
	}
	else if (infoH == 5.0)
	{
		lparam.xyz = lines9;
		px = float(df(h,h5) <= df(h,i));
		addr.x = saturate(1.0 - px);
		addr.y = 2.0 * px + saturate(1.0 - px);
	}
	else
	{
		px = float(df(e,f) <= df(e,h));
		addr.x = px;
		addr.y = saturate(1.0 - px);

		lparam.xyz = (infoE == 8.0) ? lines8 : ((infoE == 7.0) ? lines7 : ((infoE == 6.0) ? lines6 : ((infoE == 5.0) ? lines5 : ((infoE == 4.0) ? lines4 : ((infoE == 3.0) ? lines3 : ((infoE == 2.0) ? lines2 : ((infoE == 1.0) ? lines1 : lines0)))))));
	}

	bool inv = ((dir.x * dir.y) < 0.0);

	// Rotate address from relative to absolute.
	addr = addr * dir.yx;
	addr = (inv ? addr.yx : addr);

	// Rotate straight line equation from relative to absolute.
	lparam.xy = lparam.xy * dir.yx;
	lparam.xy = (inv ? lparam.yx : lparam.xy);

	addr.x += 2.0;
	addr.y += 2.0;

	lparam.w = addr.x * 8.0 + addr.y;

	gl_FragColor = vec4(remapTo01(lparam, low, high));
}