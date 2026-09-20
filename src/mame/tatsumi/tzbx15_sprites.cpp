// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese

#include "emu.h"
#include "tzbx15_sprites.h"
#include "screen.h"

// TZB215 on Apache 3
// TZB315 on Round Up 5, Big Fight, Cycle Warriors
// differences, if any, unknown
// (does not appear to be CLUT size, even if that would have made sense.
//  Round Up 5 uses the smaller CLUT like Apache 3 yet is confirmed to be a TZB315)

DEFINE_DEVICE_TYPE(TZB215_SPRITES, tzb215_device, "tzb215_sprites", "Tatsumi TZB215 Rotating Sprites")
DEFINE_DEVICE_TYPE(TZB315_SPRITES, tzb315_device, "tzb315_sprites", "Tatsumi TZB315 Rotating Sprites")

tzbx15_device::tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_palette_clut(*this, "palette_clut")
	, m_palette_base(*this, finder_base::DUMMY_TAG)
	, m_spriteram(*this, finder_base::DUMMY_TAG)
	, m_sprites_l_rom(*this, "sprites_l")
	, m_sprites_h_rom(*this, "sprites_h")
{
}

tzbx15_device::tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, type, tag, owner, clock)
{
	m_rom_clut_size = clut_size;
}

tzb215_device::tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, TZB215_SPRITES, tag, owner, clock, clut_size)
{
}

tzb215_device::tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tzbx15_device(mconfig, TZB215_SPRITES, tag, owner, clock, 0)
{
}

tzb315_device::tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, TZB315_SPRITES, tag, owner, clock, clut_size)
{
}

tzb315_device::tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tzbx15_device(mconfig, TZB315_SPRITES, tag, owner, clock, 0)
{
}




static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 8,12,0,4, 24,28,16,20 },
	{ STEP8(0,4*8) },
	32*8
};

GFXDECODE_MEMBER( tzbx15_device::gfxinfo )
	GFXDECODE_DEVICE("sprites_l", 0, spritelayout, 0, 256)
	GFXDECODE_DEVICE("sprites_h", 0, spritelayout, 0, 256)
GFXDECODE_END

void tzbx15_device::device_start()
{
	m_rom_clut_offset = memregion("sprites_l")->bytes() - m_rom_clut_size;

	m_shadow_pen_array = make_unique_clear<uint8_t[]>(m_rom_clut_size * 2);

	decode_gfx(gfxinfo);
	gfx(0)->set_colors(m_rom_clut_size / 8);
	gfx(1)->set_colors(m_rom_clut_size / 8);
}

void tzbx15_device::device_reset()
{
}

void tzbx15_device::device_add_mconfig(machine_config &config)
{
	// 4096 or 8192 arranged as series of CLUTs
	PALETTE(config, m_palette_clut).set_format(palette_device::xRGB_555, m_rom_clut_size * 2);
}

// The low nine bits are a signed 8-fractional-bit shear (tan(theta)),
// not a binary angle. The game compensates the scale by cos(theta) and
// uses alternate ROM descriptors and flips for the other octants.
// Sample the complete ROM-defined object about its RAM X/Y anchor. This
// avoids both the old 512-pixel temporary-bitmap limit and tile-edge cracks.
template<class BitmapClass>
void tzbx15_device::draw_rotated_sprite(BitmapClass &bitmap, const rectangle &cliprect,
		int index, int color, int x, int y, int scale, int rotation, bool flipx, bool flipy,
		int write_priority_only)
{
	const uint8_t *const header = m_sprites_l_rom + index * 4;
	const int top = header[0] & 0xf8;
	const int rows = (int(header[2]) - top + 7) / 8;
	if (!scale || rows <= 0)
		return;

	struct strip { int left, right, base; } strips[32];
	int left = 2048, right = 0;
	for (int row = 0; row < rows; ++row)
	{
		const uint8_t *const descriptor = (row & 1)
				? &m_sprites_l_rom[index * 4 + 4 + (row / 2) * 4]
				: &m_sprites_h_rom[index * 4 + (row / 2) * 4];
		strips[row] = { descriptor[1] * 8, (descriptor[1] + descriptor[0] + 1) * 8,
				(descriptor[2] | (descriptor[3] << 8)) * 2 };
		left = std::min(left, strips[row].left);
		right = std::max(right, strips[row].right);
	}

	const double zoom = double(scale) / 128.0;
	const double shear = double(util::sext(rotation, 9)) / 256.0;
	const double a = flipx ? -zoom : zoom;
	const double d = flipy ? -zoom : zoom;
	const double b = -a * shear;
	const double c = d * shear;
	const double determinant = a * d - b * c;
	const int bottom = top + rows * 8;
	const bool fill = BIT(header[3], 7);
	if (fill)
	{
		// End-of-line fill extends the last source pixel, including through
		// rotation and flips. Bound it by the inverse image of the viewport.
		for (int px : { cliprect.min_x, cliprect.max_x + 1 })
			for (int py : { cliprect.min_y, cliprect.max_y + 1 })
				right = std::max(right, int(std::ceil((d * (px - x) - b * (py - y)) / determinant)) + 1);
	}
	double minx = bitmap.width(), maxx = -1, miny = bitmap.height(), maxy = -1;
	for (int u : { left, right })
		for (int v : { top, bottom })
		{
			const double px = x + a * u + b * v;
			const double py = y + c * u + d * v;
			minx = std::min(minx, px); maxx = std::max(maxx, px);
			miny = std::min(miny, py); maxy = std::max(maxy, py);
		}
	rectangle bounds(int(std::floor(minx)), int(std::ceil(maxx)) - 1,
			int(std::floor(miny)), int(std::ceil(maxy)) - 1);
	bounds &= cliprect;
	bounds &= bitmap.cliprect();
	const unsigned palette = 16 * (color % gfx(0)->colors());
	const pen_t *const pens = &m_palette_clut->pen(palette);
	const uint8_t *const shadows = m_shadow_pen_array.get() + palette;
	for (int dy = bounds.min_y; dy <= bounds.max_y; ++dy)
		for (int dx = bounds.min_x; dx <= bounds.max_x; ++dx)
		{
			const double px = dx + 0.5 - x, py = dy + 0.5 - y;
			const int u = int(std::floor((d * px - b * py) / determinant));
			const int v = int(std::floor((a * py - c * px) / determinant));
			if (v < top || v >= bottom)
				continue;
			const strip &line = strips[(v - top) / 8];
			if (u < line.left || (!fill && u >= line.right))
				continue;
			const int source_x = std::min(u, line.right - 1);
			const int tile = line.base + (source_x - line.left) / 8;
			gfx_element *const source = gfx(tile & 1);
			const uint8_t pixel = source->get_data((tile >> 1) % source->elements())[
					(v & 7) * source->rowbytes() + (source_x & 7)];
			if (pixel)
			{
				if (write_priority_only)
					bitmap.pix(dy, dx) = shadows[pixel];
				else if (!shadows[pixel])
					bitmap.pix(dy, dx) = pens[pixel];
			}
		}
}

template<class BitmapClass>
void tzbx15_device::roundupt_drawgfxzoomrotate(
		BitmapClass &dest_bmp, const rectangle &clip,
		gfx_element *gfx, uint32_t code, uint32_t color,
		int flipx, int flipy, uint32_t ssx, uint32_t ssy,
		int scalex, int scaley, int rotate,
		int write_priority_only)
{
	if (!scalex || !scaley) return;

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/

	// force clip to bitmap boundary
	rectangle myclip = clip;
	myclip &= dest_bmp.cliprect();

	if (gfx)
	{
		const pen_t *pal = &m_palette_clut->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
		const uint8_t *shadow_pens = m_shadow_pen_array.get() + (gfx->granularity() * (color % gfx->colors()));
		const uint8_t *code_base = gfx->get_data(code % gfx->elements());

		int block_size = 8 * scalex;
		int sprite_screen_height = ((ssy&0xffff)+block_size)>>16;
		int sprite_screen_width = ((ssx&0xffff)+block_size)>>16;

		if (sprite_screen_width && sprite_screen_height)
		{
			// compute sprite increment per screen pixel
			int dx = (gfx->width()<<16)/sprite_screen_width;
			int dy = (gfx->height()<<16)/sprite_screen_height;

			int sx;//=ssx>>16;
			int sy;//=ssy>>16;

//          int ex = sx+sprite_screen_width;
//          int ey = sy+sprite_screen_height;


			if (ssx&0x80000000) sx=0-(0x10000 - (ssx>>16)); else sx=ssx>>16;
			if (ssy&0x80000000) sy=0-(0x10000 - (ssy>>16)); else sy=ssy>>16;
			int ex = sx+sprite_screen_width;
			int ey = sy+sprite_screen_height;
			int x_index_base;
			if( flipx )
			{
				x_index_base = (sprite_screen_width-1)*dx;
				dx = -dx;
			}
			else
			{
				x_index_base = 0;
			}

			int y_index;
			if( flipy )
			{
				y_index = (sprite_screen_height-1)*dy;
				dy = -dy;
			}
			else
			{
				y_index = 0;
			}

			if( sx < myclip.min_x)
			{
				// clip left
				int pixels = myclip.min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < myclip.min_y )
			{
				// clip top
				int pixels = myclip.min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			if( ex > myclip.max_x+1 )
			{
				// clip right
				int pixels = ex-myclip.max_x-1;
				ex -= pixels;
			}
			if( ey > myclip.max_y+1 )
			{
				// clip bottom
				int pixels = ey-myclip.max_y-1;
				ey -= pixels;
			}

			// skip if inner loop doesn't draw anything
			if( ex > sx )
			{

				for( int y=sy; y<ey; y++ )
				{
					uint8_t const *const source = code_base + (y_index>>16) * gfx->rowbytes();
					typename BitmapClass::pixel_t *const dest = &dest_bmp.pix(y);

					int x_index = x_index_base;
					for( int x=sx; x<ex; x++ )
					{
						int c = source[x_index>>16];
						if( c )
						{
							// Only draw shadow pens if writing priority buffer
							if (write_priority_only)
								dest[x]=shadow_pens[c];
							else if (!shadow_pens[c])
								dest[x]=pal[c];
						}
						x_index += dx;
					}

					y_index += dy;
				}
			}
		}
	}
}

/*
    Sprite RAM itself uses an index into two ROM tables to actually draw the object.

    Sprite RAM format:

    Word 0: 0xf000 - ?
            0x0fff - Index into ROM sprite table
    Word 1: 0x8000 - X Flip
            0x4000 - Y Flip
            0x3000 - ?
            0x0ff8 - Color
            0x0007 - ?
    Word 2: 0xffff - X position
    Word 3: 0xffff - Y position
    Word 4: 0x01ff - Scale
    Word 5: 0x01ff - Signed shear (8 fractional bits)

    Sprite ROM table format, alternate lines come from each bank, with the
    very first line indicating control information:

    First bank:
    Byte 0: Y source offset (in pixels, before scaling/rotation).
    Byte 1: Always 0?
    Byte 2: Number of source scanlines to render from (so unaffected by destination scale).
    Byte 3: Bit 7 extends the final pixel of each source line.

    Other banks:
    Byte 0: Width of line in tiles (-1)
    Byte 1: X offset to start drawing line at (multipled by scale * 8)
    Bytes 2/3: Tile index to start fetching tiles from (increments per tile).

*/
template<class BitmapClass>
void tzbx15_device::draw_sprites_main(BitmapClass &bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	// Sprite data is double buffered
	for (int offs = rambank;offs < rambank + 0x800;offs += 6)
	{
		int y =         m_spriteram[offs+3];
		int x =         m_spriteram[offs+2];
		int scale =     m_spriteram[offs+4] & 0x1ff;
		int color =     m_spriteram[offs+1] >> 3 & 0x1ff;
		int flip_x =    m_spriteram[offs+1] & 0x8000;
		int flip_y =    m_spriteram[offs+1] & 0x4000;
		int rotate =    m_rotation_enabled ? (m_spriteram[offs+5] & 0x1ff) : 0;

		int index = m_spriteram[offs];

//      if (m_spriteram[offs+1]&0x7)
//          color=machine().rand()%0xff;

		/* End of sprite list marker */
		if (index == 0xffff || m_spriteram[offs + 4] == 0xffff) // todo
			return;

		if (index >= 0x4000)
			continue;

		if (m_rotation_enabled && (rotate || BIT(m_sprites_l_rom[index * 4 + 3], 7)))
		{
			draw_rotated_sprite(bitmap, cliprect, index, color, int16_t(x), int16_t(y),
					scale, rotate, flip_x, flip_y, write_priority_only);
			continue;
		}

		uint8_t const *src1 = m_sprites_l_rom + (index * 4);
		uint8_t const *src2 = m_sprites_h_rom + (index * 4);

		int lines = src1[2];
		int y_offset = src1[0]&0xf8;

		lines -= y_offset;

		int render_x = x << 16;
		int render_y = y << 16;
		scale = scale << 9; /* 0x80 becomes 0x10000 */

		if (flip_y)
			render_y -= y_offset * scale;
		else
			render_y += y_offset * scale;

		src1 += 4;
		for (int row = 0; lines > 0; ++row, lines -= 8)
		{
			const uint8_t *const descriptor = (row & 1) ? src1 : src2;
			const int width = descriptor[0] + 1;
			const int offset = descriptor[1] * scale * 8;
			int base = (descriptor[2] | (descriptor[3] << 8)) * 2;
			int xpos = flip_x ? render_x - offset - scale * 8 : render_x + offset;
			for (int w = 0; w < width; ++w, ++base)
			{
				roundupt_drawgfxzoomrotate(bitmap, cliprect, gfx(base & 1), base >> 1,
						color, flip_x, flip_y, xpos, render_y, scale, scale, 0, write_priority_only);
				xpos += flip_x ? -scale * 8 : scale * 8;
			}
			if (row & 1) src1 += 4; else src2 += 4;
			render_y += flip_y ? -8 * scale : 8 * scale;
		}
	}
}

void tzbx15_device::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	draw_sprites_main(bitmap, cliprect, write_priority_only, rambank);
}

void tzbx15_device::draw_sprites(bitmap_ind8& bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	draw_sprites_main(bitmap, cliprect, write_priority_only, rambank);
}

/*
 * Object palettes are build from a series of cluts stored in the object roms.
 *
 *  We update 'Mame palettes' from the clut here in order to simplify the
 *  draw routines.  We also note down any uses of the 'shadow' pen (index 255).
 */
void tzbx15_device::update_cluts()
{
	const int length = m_rom_clut_size * 2;
	const uint8_t* bank1 = m_sprites_l_rom + m_rom_clut_offset;
	const uint8_t* bank2 = m_sprites_h_rom + m_rom_clut_offset;

	for (int i = 0; i < length; i+=8)
	{
		m_palette_clut->set_pen_color(i + 0, m_palette_base->pen_color(bank1[1] + m_sprite_palette_base));
		m_shadow_pen_array[i+0]=(bank1[1] == 255);
		m_palette_clut->set_pen_color(i + 1, m_palette_base->pen_color(bank1[0] + m_sprite_palette_base));
		m_shadow_pen_array[i+1]=(bank1[0] == 255);
		m_palette_clut->set_pen_color(i + 2, m_palette_base->pen_color(bank1[3] + m_sprite_palette_base));
		m_shadow_pen_array[i+2]=(bank1[3] == 255);
		m_palette_clut->set_pen_color(i + 3, m_palette_base->pen_color(bank1[2] + m_sprite_palette_base));
		m_shadow_pen_array[i+3]=(bank1[2] == 255);

		m_palette_clut->set_pen_color(i + 4, m_palette_base->pen_color(bank2[1] + m_sprite_palette_base));
		m_shadow_pen_array[i+4]=(bank2[1] == 255);
		m_palette_clut->set_pen_color(i + 5, m_palette_base->pen_color(bank2[0] + m_sprite_palette_base));
		m_shadow_pen_array[i+5]=(bank2[0] == 255);
		m_palette_clut->set_pen_color(i + 6, m_palette_base->pen_color(bank2[3] + m_sprite_palette_base));
		m_shadow_pen_array[i+6]=(bank2[3] == 255);
		m_palette_clut->set_pen_color(i + 7, m_palette_base->pen_color(bank2[2] + m_sprite_palette_base));
		m_shadow_pen_array[i+7]=(bank2[2] == 255);

		bank1+=4;
		bank2+=4;
	}
}
