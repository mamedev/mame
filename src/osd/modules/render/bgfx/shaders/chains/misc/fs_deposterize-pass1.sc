$input v_color0, v_texcoord0, v_texcoord1

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's Deposterize Shader - Pass1
   
   Copyright (C) 2011/2016 Hyllian/Jararaca - sergiogdb@gmail.com

*/

#include "common.sh"

// Samplers
SAMPLER2D(decal, 0);

uniform vec4 EQ_THRESH2;
uniform vec4 DIFF_THRESH2;

vec3 df3(vec3 c1, vec3 c2)
{
      return abs(c1 - c2);
}

bvec3 le3(vec3 A, vec3 B, vec3 param)
{
	return lessThanEqual(df3(A, B), param);
}

bvec3 gt3(vec3 A, vec3 B, vec3 param)
{
	return greaterThan(df3(A, B), param);
}

void main()
{
	vec3 B  = texture2D(decal, v_texcoord1.xy).rgb;
	vec3 E  = texture2D(decal, v_texcoord1.xz).rgb;
	vec3 H  = texture2D(decal, v_texcoord1.xw).rgb;

	vec3 res = (any(gt3(B, H, EQ_THRESH2.xxx)) && (any(le3(E, H, DIFF_THRESH2.xxx)) && any(le3(B, E, EQ_THRESH2.xxx)) || any(le3(B, E, DIFF_THRESH2.xxx)) && any(le3(E, H, EQ_THRESH2.xxx)))) ? 0.5 * (B + H) : E;

	gl_FragColor = vec4(res, 1.0);
}
