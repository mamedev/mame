$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "common.sh"

// Samplers
SAMPLER2D(s_tex, 0);
SAMPLER2D(s_pal, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_inv_tex_size0;
uniform vec4 u_inv_tex_size1;

void main()
{
	// Logic taken from fs_blit_yuy16.sc - we need to do this, because
	// the D3D9 BGFX backend claims to support RG8, but does so in
	// a faulty way by using A8L8, which is not an appropriate format
	// for representing an RG8 texture.
	
	vec2 half_texel = u_inv_tex_size0.xy * vec2(0.5, 0.5);
	
	vec2 original_uv = v_texcoord0.xy * u_tex_size0.xy;
	float mod_val = mod(original_uv.x, 2.0);
	vec2 rounded_uv = vec2(original_uv.x - mod_val, original_uv.y);
	vec4 srcpix = texture2D(s_tex, rounded_uv * u_inv_tex_size0.xy + half_texel.x);

	vec2 palette_uv = (srcpix.ra * vec2(256.0, 256.0)) * u_inv_tex_size1.xy;
	if (mod_val < 1.0)
		palette_uv = (srcpix.bg * vec2(256.0, 256.0)) * u_inv_tex_size1.xy;
	
	gl_FragColor = vec4(texture2D(s_pal, palette_uv).rgb, 1.0) * v_color0;
	//gl_FragColor = texture2D(s_tex,  v_texcoord0) * v_color0;
}
