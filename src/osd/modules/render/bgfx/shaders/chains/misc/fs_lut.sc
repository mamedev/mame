$input v_color0, v_texcoord0

// license: BSD-3-Clause
// copyright-holders: W. M. Martinez

#include "common.sh"

// Autos
uniform vec4 u_tex_size1;
uniform vec4 u_inv_tex_size1;

SAMPLER2D(s_tex, 0);
SAMPLER2D(s_3dlut, 1);

void main()
{
	vec4 bp = texture2D(s_tex, v_texcoord0);
	vec3 color = bp.rgb;
	// NOTE: Do not change the order of parameters here.
	vec3 lutcoord = vec3(color.rg * ((u_tex_size1.y - 1.0) + 0.5) * u_inv_tex_size1.xy, (u_tex_size1.y - 1.0) * color.b);
	float shift = floor(lutcoord.z);

	lutcoord.x += shift * u_inv_tex_size1.y;
	color.rgb = mix(texture2D(s_3dlut, lutcoord.xy).rgb,
		texture2D(s_3dlut, vec2(lutcoord.x + u_inv_tex_size1.y,
		lutcoord.y)).rgb, lutcoord.z - shift);
	gl_FragColor = vec4(color, bp.a) * v_color0;
}
