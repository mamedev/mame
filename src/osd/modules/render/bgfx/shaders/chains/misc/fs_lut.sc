$input v_color0, v_texcoord0

// license: BSD-3-Clause
// copyright-holders: W. M. Martinez

#include "common.sh"

#define LUT_TEXTURE_WIDTH 4096.0f
#define LUT_SIZE 64.0f
#define LUT_SCALE vec2(1.0f / LUT_TEXTURE_WIDTH, 1.0f / LUT_SIZE)

SAMPLER2D(s_tex, 0);
SAMPLER2D(s_3dlut, 1);

void main()
{
	vec4 bp = texture2D(s_tex, v_texcoord0);
	vec3 color = bp.rgb;
	// NOTE: Do not change the order of parameters here.
	vec3 lutcoord = vec3(color.rg * ((LUT_SIZE - 1.0f) + 0.5f) *
		LUT_SCALE, (LUT_SIZE - 1.0f) * color.b);
	float shift = floor(lutcoord.z);

	lutcoord.x += shift * LUT_SCALE.y;
	color.rgb = mix(texture2D(s_3dlut, lutcoord.xy).rgb,
		texture2D(s_3dlut, vec2(lutcoord.x + LUT_SCALE.y,
		lutcoord.y)).rgb, lutcoord.z - shift);
	gl_FragColor = vec4(color, bp.a) * v_color0;
}
