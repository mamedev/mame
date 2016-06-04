$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Autos
uniform vec4 u_tex_size0;

// User-supplied
uniform vec4 u_dimension;
uniform vec4 u_radius;

// Samplers
SAMPLER2D(s_tex, 0);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

void main()
{
	vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec2 blur = (u_radius.xx * u_dimension.xy) / u_tex_size0.xy;

	sum += texture2D(s_tex, v_texcoord0 - vec2(12.0, 12.0) * blur) * 0.001133;
	sum += texture2D(s_tex, v_texcoord0 - vec2(11.0, 11.0) * blur) * 0.002316;
	sum += texture2D(s_tex, v_texcoord0 - vec2(10.0, 10.0) * blur) * 0.00445;
	sum += texture2D(s_tex, v_texcoord0 - vec2(9.0, 9.0) * blur) * 0.008033;
	sum += texture2D(s_tex, v_texcoord0 - vec2(8.0, 8.0) * blur) * 0.013627;
	sum += texture2D(s_tex, v_texcoord0 - vec2(7.0, 7.0) * blur) * 0.021724;
	sum += texture2D(s_tex, v_texcoord0 - vec2(6.0, 6.0) * blur) * 0.032542;
	sum += texture2D(s_tex, v_texcoord0 - vec2(5.0, 5.0) * blur) * 0.04581;
	sum += texture2D(s_tex, v_texcoord0 - vec2(4.0, 4.0) * blur) * 0.0606;
	sum += texture2D(s_tex, v_texcoord0 - vec2(3.0, 3.0) * blur) * 0.075333;
	sum += texture2D(s_tex, v_texcoord0 - vec2(2.0, 2.0) * blur) * 0.088001;
	sum += texture2D(s_tex, v_texcoord0 - vec2(1.0, 1.0) * blur) * 0.096603;

	vec4 d0 = texture2D(s_tex, v_texcoord0) * 0.099654;

	sum += texture2D(s_tex, v_texcoord0 + vec2(1.0, 1.0) * blur) * 0.096603;
	sum += texture2D(s_tex, v_texcoord0 + vec2(2.0, 2.0) * blur) * 0.088001;
	sum += texture2D(s_tex, v_texcoord0 + vec2(3.0, 3.0) * blur) * 0.075333;
	sum += texture2D(s_tex, v_texcoord0 + vec2(4.0, 4.0) * blur) * 0.0606;
	sum += texture2D(s_tex, v_texcoord0 - vec2(5.0, 5.0) * blur) * 0.04581;
	sum += texture2D(s_tex, v_texcoord0 - vec2(6.0, 6.0) * blur) * 0.032542;
	sum += texture2D(s_tex, v_texcoord0 - vec2(7.0, 7.0) * blur) * 0.021724;
	sum += texture2D(s_tex, v_texcoord0 - vec2(8.0, 8.0) * blur) * 0.013627;
	sum += texture2D(s_tex, v_texcoord0 - vec2(9.0, 9.0) * blur) * 0.008033;
	sum += texture2D(s_tex, v_texcoord0 - vec2(10.0, 10.0) * blur) * 0.00445;
	sum += texture2D(s_tex, v_texcoord0 - vec2(11.0, 11.0) * blur) * 0.002316;
	sum += texture2D(s_tex, v_texcoord0 - vec2(12.0, 12.0) * blur) * 0.001133;

	gl_FragColor = vec4(sum.rgb, 1.0);
}
