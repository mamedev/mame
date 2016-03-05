// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#pragma optimize (on)
#pragma debug (off)

uniform sampler2D color_texture;
uniform sampler2D colortable_texture;
uniform vec2      colortable_sz;         // orig size for full bgr
uniform vec2      colortable_pow2_sz;    // orig size for full bgr

void main()
{
	vec4 color_tex;
	vec2 color_map_coord;
	float colortable_scale = (colortable_sz.x/3.0) / colortable_pow2_sz.x;

	// normalized texture coordinates ..
	color_tex = texture2D(color_texture, gl_TexCoord[0].st) * ((colortable_sz.x/3.0)-1.0)/colortable_pow2_sz.x;// lookup space 

	color_map_coord.x = color_tex.b;
	gl_FragColor.b    = texture2D(colortable_texture, color_map_coord).b;

	color_map_coord.x = color_tex.g + colortable_scale;
	gl_FragColor.g    = texture2D(colortable_texture, color_map_coord).g;

	color_map_coord.x = color_tex.r + 2.0 * colortable_scale;
	gl_FragColor.r    = texture2D(colortable_texture, color_map_coord).r;
}

