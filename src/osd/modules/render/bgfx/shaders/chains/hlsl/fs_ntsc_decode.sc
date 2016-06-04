$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// NTSC Decode Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// Autos
uniform vec4 u_source_dims;

// User-supplied
uniform vec4 u_a_value;
uniform vec4 u_b_value;
uniform vec4 u_cc_value;
uniform vec4 u_o_value;
uniform vec4 u_scan_time;
uniform vec4 u_notch_width;
uniform vec4 u_y_freq_response;
uniform vec4 u_i_freq_response;
uniform vec4 u_q_freq_response;
uniform vec4 u_jitter_offset;

// Parametric
uniform vec4 u_jitter_amount;

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_screen, 1);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

void main()
{
	vec4 BaseTexel = texture2D(s_screen, v_texcoord0.xy);

	vec4 zero = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 quarter = vec4(0.25, 0.25, 0.25, 0.25);
	vec4 onehalf = vec4(0.5, 0.5, 0.5, 0.5);
	vec4 one = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 two = vec4(2.0, 2.0, 2.0, 2.0);
	vec4 four = vec4(4.0, 4.0, 4.0, 4.0);
	int iSampleCount = 64;
	vec4 SampleCount = vec4(64.0, 64.0, 64.0, 64.0);
	vec4 HalfSampleCount = SampleCount / two;

	vec4 TimePerSample = u_scan_time.xxxx / (u_source_dims.xxxx * four);

	vec4 PI = vec4(3.1415927, 3.1415927, 3.1415927, 3.1415927);
	vec4 PI2 = vec4(6.2831854, 6.2831854, 6.2831854, 6.2831854);
	
	vec4 Fc_y1 = (u_cc_value.xxxx - u_notch_width.xxxx * onehalf) * TimePerSample;
	vec4 Fc_y2 = (u_cc_value.xxxx + u_notch_width.xxxx * onehalf) * TimePerSample;
	vec4 Fc_y3 = u_y_freq_response.xxxx * TimePerSample;
	vec4 Fc_i = u_i_freq_response.xxxx * TimePerSample;
	vec4 Fc_q = u_q_freq_response.xxxx * TimePerSample;
	vec4 Fc_i_2 = Fc_i * two;
	vec4 Fc_q_2 = Fc_q * two;
	vec4 Fc_y1_2 = Fc_y1 * two;
	vec4 Fc_y2_2 = Fc_y2 * two;
	vec4 Fc_y3_2 = Fc_y3 * two;
	vec4 Fc_i_pi2 = Fc_i * PI2;
	vec4 Fc_q_pi2 = Fc_q * PI2;
	vec4 Fc_y1_pi2 = Fc_y1 * PI2;
	vec4 Fc_y2_pi2 = Fc_y2 * PI2;
	vec4 Fc_y3_pi2 = Fc_y3 * PI2;
	vec4 PI2Length = PI2 / SampleCount;
	
	vec4 W = PI2 * u_cc_value.xxxx * u_scan_time.xxxx;
	vec4 WoPI = W / PI;

	vec4 HOffset = (u_b_value.xxxx + u_jitter_amount.xxxx * u_jitter_offset.xxxx) / WoPI;
	vec4 VScale = (u_a_value.xxxx * u_source_dims.yyyy) / WoPI;

	vec4 YAccum = zero;
	vec4 IAccum = zero;
	vec4 QAccum = zero;

	vec4 Cy = v_texcoord0.yyyy;
	vec4 VPosition = Cy;

	for (int i = 0; i < 64; i = i + 4)
	{
		vec4 n = vec4(i, i, i, i) - vec4(32.0, 32.0, 32.0, 32.0);
		vec4 n4 = n + vec4(0.0, 1.0, 2.0, 3.0);

		vec4 Cx = v_texcoord0.xxxx + (n4 * quarter) / u_source_dims.xxxx;
		vec4 HPosition = Cx;

		vec4 C = texture2D(s_tex, vec2(Cx.x, Cy.x));

		vec4 T = HPosition + HOffset + VPosition * VScale;
		vec4 WT = W * T + u_o_value.xxxx;

		vec4 SincKernel = vec4(0.54, 0.54, 0.54, 0.54) + vec4(0.46, 0.46, 0.46, 0.46) * cos(PI2Length * n4);

		vec4 SincYIn1 = Fc_y1_pi2 * n4;
		vec4 SincYIn2 = Fc_y2_pi2 * n4;
		vec4 SincYIn3 = Fc_y3_pi2 * n4;
		vec4 SincIIn = Fc_i_pi2 * n4;
		vec4 SincQIn = Fc_q_pi2 * n4;

		vec4 SincY1 = SincYIn1 != zero ? sin(SincYIn1) / SincYIn1 : one;
		vec4 SincY2 = SincYIn2 != zero ? sin(SincYIn2) / SincYIn2 : one;
		vec4 SincY3 = SincYIn3 != zero ? sin(SincYIn3) / SincYIn3 : one;

		vec4 IdealY = (Fc_y1_2 * SincY1 - Fc_y2_2 * SincY2) + Fc_y3_2 * SincY3;
		vec4 IdealI = Fc_i_2 * (SincIIn != zero ? sin(SincIIn) / SincIIn : one);
		vec4 IdealQ = Fc_q_2 * (SincQIn != zero ? sin(SincQIn) / SincQIn : one);

		vec4 FilterY = SincKernel * IdealY;
		vec4 FilterI = SincKernel * IdealI;
		vec4 FilterQ = SincKernel * IdealQ;

		YAccum = YAccum + C * FilterY;
		IAccum = IAccum + C * cos(WT) * FilterI;
		QAccum = QAccum + C * sin(WT) * FilterQ;
	}
	
	vec3 YIQ = vec3(
		(YAccum.r + YAccum.g + YAccum.b + YAccum.a),
		(IAccum.r + IAccum.g + IAccum.b + IAccum.a) * 2.0,
		(QAccum.r + QAccum.g + QAccum.b + QAccum.a) * 2.0);

	vec3 RGB = vec3(
		dot(YIQ, vec3(1.0, 0.956, 0.621)),
		dot(YIQ, vec3(1.0, -0.272, -0.647)),
		dot(YIQ, vec3(1.0, -1.106, 1.703)));

	// RGB obviously contains a NaN somewhere along the line! Returns black if vec4 etc. are included, white if just vec4(1.0)
	gl_FragColor = vec4(RGB, BaseTexel.a) * v_color0;
}
