$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// NTSC Effect
//-----------------------------------------------------------------------------

// NB: intentionally wasteful of uniforms in order for easier slider utilization

#include "common.sh"

// Autos
uniform vec4 u_source_dims;

// User-supplied
uniform vec4 u_a_value;
uniform vec4 u_b_value;
uniform vec4 u_cc_value;
uniform vec4 u_p_value;
uniform vec4 u_scan_time;
uniform vec4 u_jitter_offset;

// Parametric
uniform vec4 u_jitter_amount;

// Samplers
SAMPLER2D(s_tex, 0);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

void main()
{
	vec2 PValueSourceTexel = vec2(u_p_value.x, 0.0) / u_source_dims.xy;

	vec2 C0 = v_texcoord0 + PValueSourceTexel * vec2(0.0,  0.0);
	vec2 C1 = v_texcoord0 + PValueSourceTexel * vec2(0.25, 0.0);
	vec2 C2 = v_texcoord0 + PValueSourceTexel * vec2(0.5,  0.0);
	vec2 C3 = v_texcoord0 + PValueSourceTexel * vec2(0.75, 0.0);
	vec4 Cx = vec4(C0.x, C1.x, C2.x, C3.x);
	vec4 Cy = vec4(C0.y, C1.y, C2.y, C3.y);
	vec4 Texel0 = texture2D(s_tex, C0);
	vec4 Texel1 = texture2D(s_tex, C1);
	vec4 Texel2 = texture2D(s_tex, C2);
	vec4 Texel3 = texture2D(s_tex, C3);

	vec4 HPosition = Cx;
	vec4 VPosition = Cy;

	const vec4 YDot = vec4(0.299, 0.587, 0.114, 0.0);
	const vec4 IDot = vec4(0.595716, -0.274453, -0.321263, 0.0);
	const vec4 QDot = vec4(0.211456, -0.522591, 0.311135, 0.0);
	vec4 Y = vec4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	vec4 I = vec4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	vec4 Q = vec4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

	const float PI = 3.1415927;
	const float PI2 = 6.2831854;
	
	float W = PI2 * u_cc_value.x * u_scan_time.x;
	float WoPI = W / PI;

	float HOffset = (u_a_value.x + u_jitter_amount.x * u_jitter_offset.x) / WoPI;
	float VScale =  (u_b_value.x * u_source_dims.y) / WoPI;

	vec4 T = HPosition + vec4(HOffset, HOffset, HOffset, HOffset) + VPosition * vec4(VScale, VScale, VScale, VScale);
	vec4 TW = T * W;

	gl_FragColor = Y + I * cos(TW) + Q * sin(TW);
}