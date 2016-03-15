$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// NTSC Effect
//-----------------------------------------------------------------------------

// NB: intentionally wasteful of uniforms in order for easier slider utilization

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_a_value;
uniform vec4 u_b_value;
uniform vec4 u_cc_value;
uniform vec4 u_p_value;
uniform vec4 u_scan_time;
uniform vec4 u_jitter_amount;
uniform vec4 u_jitter_offset;

uniform vec4 u_source_dims;

SAMPLER2D(DiffuseSampler, 0);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

void main()
{
	vec2 PValueSourceTexel = vec2(u_p_value.x, 0.0) * u_source_dims.xy;

	vec2 C0 = v_texcoord0 + PValueSourceTexel * vec2(0.0,  0.0);
	vec2 C1 = v_texcoord0 + PValueSourceTexel * vec2(0.25, 0.0);
	vec2 C2 = v_texcoord0 + PValueSourceTexel * vec2(0.5,  0.0);
	vec2 C3 = v_texcoord0 + PValueSourceTexel * vec2(0.75, 0.0);
	vec4 Cx = vec4(C0.x, C1.x, C2.x, C3.x);
	vec4 Cy = vec4(C0.y, C1.y, C2.y, C3.y);
	vec4 Texel0 = texture2D(DiffuseSampler, C0);
	vec4 Texel1 = texture2D(DiffuseSampler, C1);
	vec4 Texel2 = texture2D(DiffuseSampler, C2);
	vec4 Texel3 = texture2D(DiffuseSampler, C3);

	const vec4 YDot = vec4(0.299, 0.587, 0.114, 0.0);
	const vec4 IDot = vec4(0.595716, -0.274453, -0.321263, 0.0);
	const vec4 QDot = vec4(0.211456, -0.522591, 0.311135, 0.0);
	vec4 Y = vec4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	vec4 I = vec4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	vec4 Q = vec4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

	const vec4 PI = vec4(3.1415927, 3.1415927, 3.1415927, 3.1415927);
	const vec4 PI2 = vec4(6.2831854, 6.2831854, 6.2831854, 6.2831854);
	vec4 W = PI2 * u_cc_value.xxxx * u_scan_time.xxxx;
	vec4 WoPI = W / PI;

	vec4 HOffset = (u_b_value.xxxx + u_jitter_amount.xxxx * u_jitter_offset.xxxx) / WoPI;
	vec4 VScale = (u_a_value.xxxx * u_source_dims.yyyy) / WoPI;

	vec4 T = Cx + HOffset + Cy * VScale;
	vec4 TW = T * W;

	vec4 output_rgb = Y + I * cos(TW) + Q * sin(TW);
	
	gl_FragColor = vec4(output_rgb.xyz, 1.0);
}
