// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#pragma optimize (on)
#pragma debug (off)

uniform sampler2D color_texture;
uniform sampler2D colortable_texture;
uniform vec2      colortable_sz;      // ct size
uniform vec2      colortable_pow2_sz; // pow2 ct size

void main()
{
	vec4 color_tex;
	vec2 color_map_coord;

	// normalized texture coordinates ..
	color_tex = texture2D(color_texture, gl_TexCoord[0].st);

	// GL_UNSIGNED_SHORT GL_ALPHA in ALPHA16 conversion:
	// general: f = c / ((2*N)-1), c color bitfield, N number of bits
	// ushort:  c = ((2**16)-1)*f;
	color_map_coord.x = floor( 65535.0 * color_tex.a + 0.5 );

	// map it to the 2D lut table
	color_map_coord.y = floor(color_map_coord.x/colortable_sz.x);
	color_map_coord.x =   mod(color_map_coord.x,colortable_sz.x);

	gl_FragColor = texture2D(colortable_texture, (color_map_coord+vec2(0.5)) / colortable_pow2_sz);
}


