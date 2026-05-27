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
	m_temp_bitmap.allocate(512, 512);

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

void tzbx15_device::mycopyrozbitmap_core(bitmap_ind8 &bitmap, const bitmap_rgb32 &srcbitmap,
		int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
		const rectangle &clip, int transparent_color)
{ }

void tzbx15_device::mycopyrozbitmap_core(bitmap_rgb32 &bitmap, const bitmap_rgb32 &srcbitmap,
	int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
	const rectangle &clip, int transparent_color)
{
	//  const int xmask = srcbitmap.width()-1;
	//  const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcwidth << 16;
	const int heightshifted = srcheight << 16;

	uint32_t startx = 0;
	uint32_t starty = 0;

	int sx = dstx;
	int sy = dsty;
	int ex = dstx + srcwidth;
	int ey = dsty + srcheight;

	if (sx < clip.min_x) sx = clip.min_x;
	if (ex > clip.max_x) ex = clip.max_x;
	if (sy < clip.min_y) sy = clip.min_y;
	if (ey > clip.max_y) ey = clip.max_y;

	if (sx <= ex)
	{
		while (sy <= ey)
		{
			int x = sx;
			uint32_t cx = startx;
			uint32_t cy = starty;
			uint32_t *dest = &bitmap.pix(sy, sx);

			while (x <= ex)
			{
				if (cx < widthshifted && cy < heightshifted)
				{
					int c = srcbitmap.pix(cy >> 16, cx >> 16);

					if (c != transparent_color)
						*dest = c;
				}

				cx += incxx;
				cy += incxy;
				x++;
				dest++;
			}
			startx += incyx;
			starty += incyy;
			sy++;
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

			int incxx=0x10000;//(int)((float)dx * cos(theta));
//          int incxy=0x0;//(int)((float)dy * -sin(theta));
			int incyx=0x0;//(int)((float)dx * sin(theta));
//          int incyy=0x10000;//(int)((float)dy * cos(theta));

			if (ssx&0x80000000) sx=0-(0x10000 - (ssx>>16)); else sx=ssx>>16;
			if (ssy&0x80000000) sy=0-(0x10000 - (ssy>>16)); else sy=ssy>>16;
			int ex = sx+sprite_screen_width;
			int ey = sy+sprite_screen_height;
			int x_index_base;
			if( flipx )
			{
				x_index_base = (sprite_screen_width-1)*dx;
				dx = -dx;
				incxx=-incxx;
				incyx=-incyx;
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
#if 0
				int startx=0;
				int starty=0;

				//int incxx=0x10000;
				//int incxy=0;
				//int incyx=0;
				//int incyy=0x10000;
				double theta=rotate * ((2.0 * M_PI)/512.0);
				double c=cos(theta);
				double s=sin(theta);

				//if (ey-sy > 0) dy=dy / (ey-sy);
				{
					float angleAsRadians=(float)rotate * (7.28f / 512.0f);
					//float ccx = cosf(angleAsRadians);
					//float ccy = sinf(angleAsRadians);
					float a=0;

				}

				for( int y=sy; y<ey; y++ )
				{
					uint32_t *const dest = &dest_bmp.pix(y);
					int cx = startx;
					int cy = starty;

					int x_index = x_index_base;
					for( int x=sx; x<ex; x++ )
					{
						const uint8_t *source = code_base + (cy>>16) * gfx->rowbytes();
						int c = source[(cx >> 16)];
						if( c != transparent_color )
						{
							if (write_priority_only)
								dest[x]=shadow_pens[c];
							else
								dest[x]=pal[c];
						}
						cx += incxx;
						cy += incxy;
					}
					startx += incyx;
					starty += incyy;
				}
#endif
#if 1 // old
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
#endif
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
    Word 5: 0x01ff - Rotation

    Sprite ROM table format, alternate lines come from each bank, with the
    very first line indicating control information:

    First bank:
    Byte 0: Y destination offset (in scanlines, unaffected by scale).
    Byte 1: Always 0?
    Byte 2: Number of source scanlines to render from (so unaffected by destination scale).
    Byte 3: Usually 0, sometimes 0x80??

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
		int rotate =    0;//m_spriteram[offs+5]&0x1ff; // Todo:  Turned off for now

		int index = m_spriteram[offs];

//      if (m_spriteram[offs+1]&0x7)
//          color=machine().rand()%0xff;

		/* End of sprite list marker */
		if (index == 0xffff || m_spriteram[offs + 4] == 0xffff) // todo
			return;

		if (index >= 0x4000)
			continue;

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

		if (rotate)
		{
			render_y = 0;
			m_temp_bitmap.fill(0);
		}

		int extent_x = 0, extent_y = 0;

		src1 += 4;
		int h = 0;

		while (lines > 0)
		{
			int base, x_offs, x_width, x_pos, draw_this_line = 1;
			int this_extent = 0;

			/* Odd and even lines come from different banks */
			if (h & 1)
			{
				x_width = src1[0] + 1;
				x_offs = src1[1] * scale * 8;
				base = src1[2] | (src1[3] << 8);
			}
			else
			{
				x_width = src2[0] + 1;
				x_offs = src2[1] * scale * 8;
				base = src2[2] | (src2[3] << 8);
			}

			if (draw_this_line)
			{
				base *= 2;

				if (!rotate)
				{
					if (flip_x)
						x_pos = render_x - x_offs - scale * 8;
					else
						x_pos = render_x + x_offs;
				}
				else
					x_pos = x_offs;

				for (int w = 0; w < x_width; w++)
				{
					if (rotate)
						roundupt_drawgfxzoomrotate(
								m_temp_bitmap,cliprect,gfx(0 + (base & 1)),
								base >> 1,
								color,flip_x,flip_y,x_pos,render_y,
								scale,scale,0,write_priority_only);
					else
						roundupt_drawgfxzoomrotate(
								bitmap,cliprect,gfx(0 + (base & 1)),
								base >> 1,
								color,flip_x,flip_y,x_pos,render_y,
								scale,scale,0,write_priority_only);
					base++;

					if (flip_x)
						x_pos -= scale * 8;
					else
						x_pos += scale * 8;

					this_extent += scale * 8;
				}
				if (h & 1)
					src1 += 4;
				else
					src2 += 4;

				if (this_extent > extent_x)
					extent_x = this_extent;
				this_extent = 0;

				if (flip_y)
					render_y -= 8 * scale;
				else
					render_y += 8 * scale;
				extent_y += 8 * scale;

				h++;
				lines -= 8;
			}
			else
			{
				h = 32; // hack
			}
		}

		if (rotate)
		{
			double theta = rotate * ((2.0 * M_PI) / 512.0);

			int incxx = (int)(65536.0 * cos(theta));
			int incxy = (int)(65536.0 * -sin(theta));
			int incyx = (int)(65536.0 * sin(theta));
			int incyy = (int)(65536.0 * cos(theta));

			extent_x = extent_x >> 16;
			extent_y = extent_y >> 16;
			if (extent_x > 2 && extent_y > 2)
				mycopyrozbitmap_core(bitmap, m_temp_bitmap, x/* + (extent_x/2)*/, y /*+ (extent_y/2)*/, extent_x, extent_y, incxx, incxy, incyx, incyy, cliprect, 0);
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
