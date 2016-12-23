$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's 2xBR v3.7c+ReverseAA (squared) Shader
   
   Copyright (C) 2011/2012 Hyllian/Jararaca - sergiogdb@gmail.com
*/

/*
 *  ReverseAA part of the code
 *
 *  Copyright (c) 2012, Christoph Feck <christoph@maxiom.de>
 *  All Rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "common.sh"

SAMPLER2D(decal, 0);

uniform vec4 u_tex_size0;

const float coef          = 2.0;
const vec4 eq_threshold = vec4(15.0, 15.0, 15.0, 15.0);
const mat4 yuv_weighted  = mat4(48.0 * vec4(0.299, 0.587, 0.114, 0.0), 7.0 * vec4(-0.169, -0.331, 0.499, 0.0), 6.0 * vec4(0.499, -0.418, -0.0813, 0.0), vec4(0.0, 0.0, 0.0, 0.0));

vec4 df(vec4 A, vec4 B)
{
	return vec4(abs(A-B));
}

vec4 lt_coeff(vec4 A, vec4 B)
{
	return vec4(lessThan(df(A, B), vec4(15.0, 15.0, 15.0, 15.0)));
}

vec4 ge_coeff(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(df(A, B), vec4(15.0, 15.0, 15.0, 15.0)));
}

vec4 ne(vec4 A, vec4 B)
{
	return vec4(notEqual(A, B));
}

vec4 lt(vec4 A, vec4 B)
{
	return vec4(lessThan(A, B));
}

vec3 lt3(vec3 A, vec3 B)
{
	return vec3(lessThan(A, B));
}

vec4 le(vec4 A, vec4 B)
{
	return vec4(lessThanEqual(A, B));
}

vec3 le3(vec3 A, vec3 B)
{
	return vec3(lessThanEqual(A, B));
}

vec4 ge(vec4 A, vec4 B)
{
	return vec4(greaterThanEqual(A, B));
}

vec3 gt3(vec3 A, vec3 B)
{
	return vec3(greaterThan(A, B));
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

	vec4 interp_restriction_lv1      = clamp((ne(e,f) * ne(e,h) * (ge_coeff(f,b) * ge_coeff(f,c) + ge_coeff(h,d) * ge_coeff(h,g) + lt_coeff(e,i) * (ge_coeff(f,f4) * ge_coeff(f,i4) + ge_coeff(h,h5) * ge_coeff(h,i5)) + lt_coeff(e,g) + lt_coeff(e,c))), 0.0, 1.0);
	vec4 interp_restriction_lv2_left = (ne(e,g) * ne(d,g));
	vec4 interp_restriction_lv2_up   = (ne(e,c) * ne(b,c));

	vec4 edr      = lt(weighted_distance( e, c, g, i, h5, f4, h, f), weighted_distance( h, d, i5, f, i4, b, e, i)) * interp_restriction_lv1;
	vec4 edr_left = le(coef * df(f,g), df(h,c)) * interp_restriction_lv2_left;
	vec4 edr_up   = ge(df(f,g), coef * df(h,c)) * interp_restriction_lv2_up;

	vec4 px = le(df(e,f), df(e,h));

	vec4 P[4];
	P[0] = px.x > 0.0 ? F : H;
	P[1] = px.y > 0.0 ? B : F;
	P[2] = px.z > 0.0 ? D : B;
	P[3] = px.w > 0.0 ? H : D;

	vec4 res = E;

    vec3 threshold = vec3(0.4, 0.4, 0.4);
    vec3 xx = vec3(0.0, 0.0, 0.0);

    vec3 n1 = B1.xyz;
    vec3 n2 = B.xyz;
    vec3 s  = E.xyz;
    vec3 n3 = H.xyz;
    vec3 n4 = H5.xyz;
    
    vec3 aa = n2 - n1;
    vec3 bb =  s - n2;
    vec3 cc = n3 - s;
    vec3 dd = n4 - n3;

    vec3 t = (7.0 * (bb + cc) - 3.0 * (aa + dd)) / 16.0;
    vec3 m = mix(2.0 * (1.0 - s), 2.0 * s, lt3(s, vec3(0.5, 0.5, 0.5)));

	m = min(m, 2.0 * abs(bb));
	m = min(m, 2.0 * abs(cc));
    vec3 sim1 = (gt3(abs(bb) * 2.0, threshold) + gt3(abs(cc) * 2.0, threshold) + le3(abs(bb) * 2.0, xx) + le3(abs(cc) * 2.0, xx));
    t = clamp(t, -m, m);
   
    vec3 s1 = (2.0 * fp.y - 1.0) * t + s;

    n1 = D0.xyz;
    n2 = D.xyz;
     s = s1;
    n3 = F.xyz;
    n4 = F4.xyz;
    
    aa = n2 - n1;
    bb =  s - n2;
    cc = n3 - s;
    dd = n4 - n3;

    t = (7.0 * (bb + cc) - 3.0 * (aa + dd)) / 16.0;
    m = mix(2.0 * (1.0 - s), 2.0 * s, lt3(s, vec3(0.5, 0.5, 0.5)));

	m = min(m, 2.0 * abs(bb));
	m = min(m, 2.0 * abs(cc));
    vec3 sim2 = (gt3(abs(bb) * 2.0, threshold) + gt3(abs(cc) * 2.0, threshold) + le3(abs(bb) * 2.0, xx) + le3(abs(cc) * 2.0, xx));
    t = clamp(t, -m, m);

    vec3 s0 = (2.0 * fp.x - 1.0) * t + s;

    res = vec4(s0, 1.0);

    if ((sim1.x > 0.0 || sim1.y > 0.0 || sim1.z > 0.0) && (sim2.x > 0.0 || sim2.y > 0.0 || sim2.z > 0.0))
    {
		if (fp.x >= 0.5)
		{
			if (fp.y >= 0.5)
			{
				if (edr.x > 0.0 && edr_left.x > 0.0 && edr_up.x > 0.0)
				{
					res  = mix(E , P[0],  0.833333);
				}
				else if (edr.x > 0.0 && (edr_left.x > 0.0 || edr_up.x > 0.0))
				{
					res  = mix(E , P[0],  0.75);
				}
				else if (edr.y > 0.0 && edr_left.y > 0.0 && edr.w > 0.0 && edr_up.w > 0.0)
				{
					res  = mix(E , P[1],  0.25);
					res  = mix(E , P[3],  0.25);
				}
				else if (edr.y > 0.0 && edr_left.y > 0.0)
				{
					res  = mix(E , P[1],  0.25);
				}
				else if (edr.w > 0.0 && edr_up.w > 0.0)
				{
					res  = mix(E , P[3],  0.25);
				}
				else if (edr.x > 0.0)
				{
					res  = mix(E , P[0],  0.5);
				}
			}
			else
			{
				if (edr.y > 0.0 && edr_left.y > 0.0 && edr_up.y > 0.0)
				{
					res  = mix(E , P[1],  0.833333);
				}
				else if (edr.y > 0.0 && (edr_left.y > 0.0 || edr_up.y > 0.0))
				{
					res  = mix(E , P[1],  0.75);
				}
				else if (edr.z > 0.0 && edr_left.z > 0.0 && edr.x > 0.0 && edr_up.x > 0.0)
				{
					res  = mix(E , P[2],  0.25);
					res  = mix(E , P[0],  0.25);
				}
				else if (edr.z > 0.0 && edr_left.z > 0.0)
				{
					res  = mix(E , P[2],  0.25);
				}
				else if (edr.x > 0.0 && edr_up.x > 0.0)
				{
					res  = mix(E , P[0],  0.25);
				}
				else if (edr.y > 0.0)
				{
					res  = mix(E , P[1],  0.5);
				}
			}
		}
		else
		{
			if (fp.y >= 0.5)
			{
				if (edr.w > 0.0 && edr_left.w > 0.0 && edr_up.w > 0.0)
				{
					res  = mix(E , P[3],  0.833333);
				}
				else if (edr.w > 0.0 && (edr_left.w > 0.0 || edr_up.w > 0.0))
				{
					res  = mix(E , P[3],  0.75);
				}
				else if (edr.x > 0.0 && edr_left.x > 0.0 && edr.z > 0.0 && edr_up.z > 0.0)
				{
					res  = mix(E , P[0],  0.25);
					res  = mix(E , P[2],  0.25);
				}
				else if (edr.x > 0.0 && edr_left.x > 0.0)
				{
					res  = mix(E , P[0],  0.25);
				}
				else if (edr.z > 0.0 && edr_up.z > 0.0)
				{
					res  = mix(E , P[2],  0.25);
				}
				else if (edr.w > 0.0)
				{
					res  = mix(E , P[3],  0.5);
				}
			}
			else
			{
				if (edr.z > 0.0 && edr_left.z > 0.0 && edr_up.z > 0.0)
				{
					res  = mix(E , P[2],  0.833333);
				}
				else if (edr.z > 0.0 && (edr_left.z > 0.0 || edr_up.z > 0.0))
				{
					res  = mix(E , P[2],  0.75);
				}
				else if (edr.w > 0.0 && edr_left.w > 0.0 && edr.y > 0.0 && edr_up.y > 0.0)
				{
					res  = mix(E , P[3],  0.25);
					res  = mix(E , P[1],  0.25);
				}
				else if (edr.w > 0.0 && edr_left.w > 0.0)
				{
					res  = mix(E , P[3],  0.25);
				}
				else if (edr.y > 0.0 && edr_up.y > 0.0)
				{
					res  = mix(E , P[1],  0.25);
				}
				else if (edr.z > 0.0)
				{
					res  = mix(E, P[2],  0.5);
				}
			}
		}
    }

	gl_FragColor = vec4(res.xyz, 1.0);
}