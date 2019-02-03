$input v_color0, v_texcoord0

// license: BSD-3-Clause
// copyright-holders: W. M. Martinez
//-----------------------------------------------------------------------------
// Phosphor Chromaticity to sRGB Transform Effect
//-----------------------------------------------------------------------------

#include "common.sh"

// User-supplied
uniform vec4 u_y_gain;
uniform vec4 u_chroma_a;
uniform vec4 u_chroma_b;
uniform vec4 u_chroma_c;

// Samplers
SAMPLER2D(s_tex, 0);

void main()
{
	vec4 cin = texture2D(s_tex, v_texcoord0);
	vec4 cout = vec4(0.0, 0.0, 0.0, cin.a);
	mat3 xy = mat3(u_chroma_a.xyz, u_chroma_b.xyz, u_chroma_c.xyz);
	const mat3 XYZ_TO_sRGB = mat3(
		 3.2406, -0.9689,  0.0557,
		-1.5372,  1.8758, -0.2040,
		-0.4986,  0.0415,  1.0570
	);

	for (int i = 0; i < 3; ++i) {
		float Y = u_y_gain[i] * cin[i];
		float X = xy[i].x / xy[i].y * Y;
		float Z = (1.0 - xy[i].x - xy[i].y) / xy[i].y * Y;
		cout.rgb += mul(XYZ_TO_sRGB, vec3(X, Y, Z));
	}
	gl_FragColor = cout * v_color0;
}
