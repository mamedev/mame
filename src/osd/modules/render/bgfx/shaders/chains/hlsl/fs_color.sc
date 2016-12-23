$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Color Convolution Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// User-supplied
uniform vec4 u_red_ratios;
uniform vec4 u_grn_ratios;
uniform vec4 u_blu_ratios;
uniform vec4 u_offset;
uniform vec4 u_scale;
uniform vec4 u_saturation;

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	vec4 BaseTexel = texture2D(s_tex, v_texcoord0);

	vec3 OutRGB = BaseTexel.rgb;

	// RGB Tint & Shift
	float ShiftedRed = dot(OutRGB, u_red_ratios.xyz);
	float ShiftedGrn = dot(OutRGB, u_grn_ratios.xyz);
	float ShiftedBlu = dot(OutRGB, u_blu_ratios.xyz);

	// RGB Scale & Offset
	vec3 OutTexel = vec3(ShiftedRed, ShiftedGrn, ShiftedBlu) * u_scale.xyz + u_offset.xyz;

	// Saturation
	vec3 Grayscale = vec3(0.299, 0.587, 0.114);
	float OutLuma = dot(OutTexel, Grayscale);
	vec3 OutChroma = OutTexel - OutLuma;
	vec3 Saturated = OutLuma + OutChroma * u_saturation.x;

	gl_FragColor = vec4(Saturated, BaseTexel.a);
}
