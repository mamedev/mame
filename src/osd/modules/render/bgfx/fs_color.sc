$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Color Convolution Effect
//-----------------------------------------------------------------------------

// NB: intentionally wasteful of uniforms in order for easier slider utilization

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

uniform vec4 u_red_ratios;
uniform vec4 u_grn_ratios;
uniform vec4 u_blu_ratios;
uniform vec4 u_offset;
uniform vec4 u_scale;
uniform vec4 u_saturation;

#define RedRatios u_red_ratios.xyz
#define GrnRatios u_grn_ratios.xyz
#define BluRatios u_blu_ratios.xyz
#define Offset u_offset.xyz
#define Scale u_scale.xyz
#define Saturation u_saturation.x

SAMPLER2D(DiffuseSampler, 0);

void main()
{
	vec4 BaseTexel = texture2D(DiffuseSampler, v_texcoord0);

	vec3 OutRGB = BaseTexel.rgb;

	// RGB Tint & Shift
	float ShiftedRed = dot(OutRGB, RedRatios);
	float ShiftedGrn = dot(OutRGB, GrnRatios);
	float ShiftedBlu = dot(OutRGB, BluRatios);

	// RGB Scale & Offset
	vec3 OutTexel = vec3(ShiftedRed, ShiftedGrn, ShiftedBlu) * Scale + Offset;

	// Saturation
	vec3 Grayscale = vec3(0.299, 0.587, 0.114);
	float OutLuma = dot(OutTexel, Grayscale);
	vec3 OutChroma = OutTexel - OutLuma;
	vec3 Saturated = OutLuma + OutChroma * Saturation;

	gl_FragColor = vec4(Saturated, BaseTexel.a);
}
