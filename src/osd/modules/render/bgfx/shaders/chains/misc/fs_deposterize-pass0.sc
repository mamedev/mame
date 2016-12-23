$input v_color0, v_texcoord0, v_texcoord1

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's Deposterize Shader - Pass0
   
   Copyright (C) 2011/2016 Hyllian/Jararaca - sergiogdb@gmail.com

*/

#include "common.sh"

// Samplers
SAMPLER2D(decal, 0);

uniform vec4 EQ_THRESH1;
uniform vec4 DIFF_THRESH1;

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
	vec3 D = texture2D(decal, v_texcoord1.xw).rgb;
	vec3 E = texture2D(decal, v_texcoord1.yw).rgb;
	vec3 F = texture2D(decal, v_texcoord1.zw).rgb;

	vec3 res = (any(gt3(D, F, EQ_THRESH1.xxx)) && (any(le3(E, F, DIFF_THRESH1.xxx)) && any(le3(D, E, EQ_THRESH1.xxx)) || any(le3(D, E, DIFF_THRESH1.xxx)) && any(le3(E, F, EQ_THRESH1.xxx)))) ? 0.5 * (D + F) : E;

	gl_FragColor = vec4(res, 1.0);
}
