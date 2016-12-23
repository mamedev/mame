$input v_texcoord0, v_texcoord1, v_texcoord2, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's xBR LV2 - noblend - pass1 Shader
  
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

#define round(X) floor((X) + 0.5)

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

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
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

    vec4 b = mul(mat4(B, D, H, F), yuv_weighted[0] );
    vec4 e = mul(mat4(E, E, E, E), yuv_weighted[0] );
    vec4 d = b.yzwx;
    vec4 f = b.wxyz;
    vec4 h = b.zwxy;

    vec4 Ao = vec4( 1.0, -1.0, -1.0, 1.0 );
    vec4 Bo = vec4( 1.0,  1.0, -1.0,-1.0 );
    vec4 Co = vec4( 1.5,  0.5, -0.5, 0.5 );
    vec4 Ax = vec4( 1.0, -1.0, -1.0, 1.0 );
    vec4 Bx = vec4( 0.5,  2.0, -0.5,-2.0 );
    vec4 Cx = vec4( 1.0,  1.0, -0.5, 0.0 );
    vec4 Ay = vec4( 1.0, -1.0, -1.0, 1.0 );
    vec4 By = vec4( 2.0,  0.5, -2.0,-0.5 );
    vec4 Cy = vec4( 2.0,  0.0, -1.0, 0.5 );

    // These inequations define the line below which interpolation occurs.
    vec4 fx      = ge(Co, Ao*fp.y+Bo*fp.x);
    vec4 fx_left = ge(Cx, Ax*fp.y+Bx*fp.x);
    vec4 fx_up   = ge(Cy, Ay*fp.y+By*fp.x);

    vec4 info = texture2D(decal, v_texcoord0);

    vec3 i = remapFrom01(info.xyz, vec3(0.0, 0.0, 0.0), vec3(15.0, 15.0, 15.0));

    vec4 edr      = unpack_info(i.x);
    vec4 edr_left = unpack_info(i.y);
    vec4 edr_up   = unpack_info(i.z);

    fx      = edr      * fx;
    fx_left = edr_left * fx_left;
    fx_up   = edr_up   * fx_up;

    vec4 nc = max(fx, max(fx_left, fx_up));
    vec4 px = le(df(e,f), df(e,h));

    vec3 res1 = nc.x > 0.0 ? px.x > 0.0 ? F.xyz : H.xyz : nc.y > 0.0 ? px.y > 0.0 ? B.xyz : F.xyz : nc.z > 0.0 ? px.z > 0.0 ? D.xyz : B.xyz : E.xyz;
    vec3 res2 = nc.w > 0.0 ? px.w > 0.0 ? H.xyz : D.xyz : nc.z > 0.0 ? px.z > 0.0 ? D.xyz : B.xyz : nc.y > 0.0 ? px.y > 0.0 ? B.xyz : F.xyz : E.xyz;

    vec2 df12 = abs(mul(mat3(res1, res2, vec3(0.0, 0.0, 0.0)), yuv_weighted[0].xyz).xy - e.xy);
    vec3 res = mix(res1, res2, step(df12.x, df12.y));

	gl_FragColor = vec4(res, 1.0);
}