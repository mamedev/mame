$input v_texcoord0, v_color0

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's jinc windowed-jinc 2-lobe sharper with anti-ringing Shader
   
   Copyright (C) 2011-2014 Hyllian/Jararaca - sergiogdb@gmail.com
*/

#include "common.sh"

uniform vec4 JINC2_WINDOW_SINC;
uniform vec4 JINC2_SINC;
uniform vec4 JINC2_AR_STRENGTH;
uniform vec4 u_tex_size0;

SAMPLER2D(s_p, 0);

	/*
	 This is an approximation of Jinc(x)*Jinc(x*r1/r2) for x < 2.5,
	 where r1 and r2 are the first two zeros of jinc function.
	 For a jinc 2-lobe best approximation, use A=0.5 and B=0.825.
	*/  

// A=0.5, B=0.825 is the best jinc approximation for x<2.5. if B=1.0, it's a lanczos filter.
// Increase A to get more blur. Decrease it to get a sharper picture. 
// B = 0.825 to get rid of dithering. Increase B to get a fine sharpness, though dithering returns.

#define halfpi  1.5707963267948966192313216916398
#define pi    3.1415926535897932384626433832795
#define wa    (JINC2_WINDOW_SINC.x * pi)
#define wb    (JINC2_SINC.x * pi)

float df(float A, float B)
{
	return abs(A-B);
}

// Calculates the distance between two points
float d(vec2 pt1, vec2 pt2)
{
  vec2 v = pt2 - pt1;
  return sqrt(dot(v,v));
}

vec4 min4(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return min(a, min(b, min(c, d)));
}

vec4 max4(vec4 a, vec4 b, vec4 c, vec4 d)
{
    return max(a, max(b, max(c, d)));
}

vec4 resampler(vec4 x)
{
	float wawb = wa*wb;
	vec4 wawb4 = vec4(wawb, wawb, wawb, wawb);
	return (x == vec4(0.0, 0.0, 0.0, 0.0)) ? wawb4 : sin(x*wa) * sin(x*wb) / (x*x);
}

void main()
{
	vec3 color;
	mat4 weights;

	vec2 dx = vec2(1.0, 0.0);
	vec2 dy = vec2(0.0, 1.0);

	vec2 pc = v_texcoord0 * u_tex_size0.xy;

	vec2 tc = (floor(pc - vec2(0.5,0.5)) + vec2(0.5,0.5));

	weights[0] = resampler(vec4(d(pc, tc    -dx    -dy), d(pc, tc           -dy), d(pc, tc    +dx    -dy), d(pc, tc+2.0*dx    -dy)));
	weights[1] = resampler(vec4(d(pc, tc    -dx       ), d(pc, tc              ), d(pc, tc    +dx       ), d(pc, tc+2.0*dx       )));
	weights[2] = resampler(vec4(d(pc, tc    -dx    +dy), d(pc, tc           +dy), d(pc, tc    +dx    +dy), d(pc, tc+2.0*dx    +dy)));
	weights[3] = resampler(vec4(d(pc, tc    -dx+2.0*dy), d(pc, tc       +2.0*dy), d(pc, tc    +dx+2.0*dy), d(pc, tc+2.0*dx+2.0*dy)));

	//weights[0][0] = weights[0][3] = weights[3][0] = weights[3][3] = 0.0;

	dx = dx / u_tex_size0.xy;
	dy = dy / u_tex_size0.xy;
	tc = tc / u_tex_size0.xy;

	// reading the texels

	vec4 c00 = texture2D(s_p, tc    -dx    -dy);
	vec4 c10 = texture2D(s_p, tc           -dy);
	vec4 c20 = texture2D(s_p, tc    +dx    -dy);
	vec4 c30 = texture2D(s_p, tc+2.0*dx    -dy);
	vec4 c01 = texture2D(s_p, tc    -dx       );
	vec4 c11 = texture2D(s_p, tc              );
	vec4 c21 = texture2D(s_p, tc    +dx       );
	vec4 c31 = texture2D(s_p, tc+2.0*dx       );
	vec4 c02 = texture2D(s_p, tc    -dx    +dy);
	vec4 c12 = texture2D(s_p, tc           +dy);
	vec4 c22 = texture2D(s_p, tc    +dx    +dy);
	vec4 c32 = texture2D(s_p, tc+2.0*dx    +dy);
	vec4 c03 = texture2D(s_p, tc    -dx+2.0*dy);
	vec4 c13 = texture2D(s_p, tc       +2.0*dy);
	vec4 c23 = texture2D(s_p, tc    +dx+2.0*dy);
	vec4 c33 = texture2D(s_p, tc+2.0*dx+2.0*dy);

	color = mul(weights[0], mat4(c00, c10, c20, c30)).xyz;
	color+= mul(weights[1], mat4(c01, c11, c21, c31)).xyz;
	color+= mul(weights[2], mat4(c02, c12, c22, c32)).xyz;
	color+= mul(weights[3], mat4(c03, c13, c23, c33)).xyz;
	color = color / (dot(mul(weights, vec4(1.0, 1.0, 1.0, 1.0)), vec4(1.0, 1.0, 1.0, 1.0)));

	// Anti-ringing
	//  Get min/max samples
	pc = v_texcoord0;
	c00 = texture2D(s_p, pc              );
	c11 = texture2D(s_p, pc    +dx       );
	c21 = texture2D(s_p, pc    -dx       );
	c12 = texture2D(s_p, pc           +dy);
	c22 = texture2D(s_p, pc           -dy);

	vec4 min_sample = min4(c11, c21, c12, c22);
	vec4 max_sample = max4(c11, c21, c12, c22);
	min_sample = min(min_sample, c00);
	max_sample = max(max_sample, c00);

	vec3 aux = color;

	color = clamp(color, min_sample.xyz, max_sample.xyz);
	color = mix(aux, color, JINC2_AR_STRENGTH.xxx);

	// final sum and weight normalization
	gl_FragColor = vec4(color, 1);
}