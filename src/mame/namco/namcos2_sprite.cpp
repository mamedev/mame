// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria

/*
    Namco System 2 Sprites - found on Namco System 2 video board (standard type)

    based on namcoic.txt this probably consists of the following
    C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
    C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
    C135 - Checks is object is displayed on Current output line.
    C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

    Metal Hawk requires a different draw function, so might use a different chip unless the hookup
    is just scrambled (needs checking).

    "Shadow" sprites are used in the baseball games to remap the background tile pen, and they're
    also used a lot in valkyrie.

    Device used by the following drivers:
    namco/namcos2.cpp (all games EXCEPT Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours,
    Suzuka 8 Hours 2 which use the newer Namco NB1 style sprites, see shared/namco_c355spr.cpp).
*/

#include "emu.h"
#include "namcos2_sprite.h"

DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device, "namcos2_sprite", "Namco System 2 Sprites (C106,C134,C135,C146)")
DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE_FINALLAP, namcos2_sprite_finallap_device, "namcos2_sprite_finallap", "Namco System 2 Sprites (C106,C134,C135,C146) (Final Lap)")
DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE_METALHAWK, namcos2_sprite_metalhawk_device, "namcos2_sprite_metalhawk", "Namco System 2 Sprites (C106,C134,C135,C146) (Metal Hawk)")

namcos2_sprite_device::namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE, tag, owner, clock, 0x7ff)
{
}

namcos2_sprite_device::namcos2_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 xmask) :
	device_t(mconfig, type, tag, owner, clock),
	device_gfx_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_spriteram(*this, finder_base::DUMMY_TAG),
	m_scalelut_region(*this, "scalelut"),
	m_pri_cb(*this),
	m_mix_cb(*this),
	m_xmask(xmask)
{
}


namcos2_sprite_metalhawk_device::namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE_METALHAWK, tag, owner, clock, 0x3ff)
{
}

namcos2_sprite_finallap_device::namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE_FINALLAP, tag, owner, clock, 0x7ff)
{
}

void namcos2_sprite_device::device_start()
{
	m_pri_cb.resolve_safe(0);
	m_mix_cb.resolve_safe(false);

	screen().register_screen_bitmap(m_renderbitmap);
	screen().register_screen_bitmap(m_screenbitmap);
}

/**************************************************************************************/

/**************************************************************************************/

void namcos2_sprite_device::draw_single_sprite(
		bitmap_ind16 &bitmap, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color, bool flipx, bool flipy, int sx, int sy,
		int sizex, int sizey, u32 prival)
{
	const u8 *const gfxdata = gfx->get_data(code % gfx->elements());
	const u16 pal = gfx->granularity() * (color % gfx->colors());

	const u8 offsxor = flipx ? (gfx->width() - 1) : 0;
	const u16 lutbank = (flipy ? 0x1000 : 0) | sizey;

	for (int y = 0; y <= sizey; y++)
	{
		const int yy = (sy + y) & 0x1ff;

		if (yy >= clip.min_y && yy <= clip.max_y)
		{
			int dy = m_scalelut_region[lutbank | (y << 6)];
			if (dy > 0x1f)
				continue;
			int xx = sx & m_xmask;
			int siz = 0;
			int offs = 0;

			if (gfx->height() < 32) dy >>= 1;
			const u8 *const src = &gfxdata[dy * gfx->rowbytes()];

			for (int x = gfx->width() << 1; x > 0; x--)
			{
				if (xx >= clip.min_x && xx <= clip.max_x)
				{
					const u8 c = src[(offs >> 1) ^ offsxor];

					if (c != 0xff)
						bitmap.pix(yy, xx) = ((prival & 0xf) << 12) | ((pal + c) & 0xfff);
				}
				offs++;

				siz += 1 + sizex;
				if (siz >= 0x40)
				{
					xx = (xx + (siz >> 6)) & m_xmask;
					siz &= 0x3f;
				}
			}
		}
	}
}

void namcos2_sprite_device::copybitmap(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip)
{
	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		u16 *const src = &m_renderbitmap.pix(y);
		u16 *const dest = &dest_bmp.pix(y);
		u8 *const destpri = &screen.priority().pix(y);
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			if (src[x] != 0xffff)
			{
				const u8 srcpri = (src[x] >> 12) & 0xf;
				const u16 c = src[x] & 0xfff;
				m_mix_cb(dest[x], destpri[x], gfx(0)->colorbase(), c, srcpri);
			}
		}
	}
}

void namcos2_sprite_device::copybitmap(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip)
{
	device_palette_interface &palette = gfx(0)->palette();
	const pen_t *pal = palette.pens();
	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		u16 *const src = &m_renderbitmap.pix(y);
		u16 *const srcrender = &m_screenbitmap.pix(y);
		u32 *const dest = &dest_bmp.pix(y);
		u8 *const destpri = &screen.priority().pix(y);
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			if (src[x] != 0xffff)
			{
				const u8 srcpri = (src[x] >> 12) & 0xf;
				const u16 c = src[x] & 0xfff;
				if (m_mix_cb(srcrender[x], destpri[x], gfx(0)->colorbase(), c, srcpri))
					dest[x] = pal[srcrender[x]];
			}
			else if (srcrender[x] != 0xffff)
				dest[x] = pal[gfx(0)->colorbase() + srcrender[x]];
		}
	}
}

void namcos2_sprite_device::get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32)
{
	sprn = (word1 >> 2) & 0x0fff;
	is_32 = BIT(word0, 9);
}

void namcos2_sprite_finallap_device::get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32)
{
	// Final Lap schematics show an older sprite board with lower capacity
	// and the 32/16 pixel mode select on a different bit
	// this is needed for the title screen to look correct, in addition to various in game sparks effects etc.

	sprn = (word1 >> 2) & 0x07ff;
	is_32 = BIT(word1, 13);
}

template <class BitmapClass>
void namcos2_sprite_device::draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int control)
{
	m_renderbitmap.fill(0xffff, cliprect);
	draw_sprites(cliprect, control);
	copybitmap(screen, bitmap, cliprect);
}

void namcos2_sprite_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int control)
{
	draw_common(screen, bitmap, cliprect, control);
}

void namcos2_sprite_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int control)
{
	draw_common(screen, bitmap, cliprect, control);
}

void namcos2_sprite_device::draw_sprites(const rectangle &cliprect, int control)
{
	gfx_element *const sgfx = gfx(0);

	const int offset = (control & 0x000f) * (128 * 4);
	for (int loop = 0; loop < 128; loop++)
	{
		/****************************************
		* word#0
		*   Sprite Y position           D00-D08
		*   Sprite Size 16/32           D09
		*   Sprite Size Y               D10-D15
		*
		* word#1
		*   Sprite Quadrant             D00-D01
		*   Sprite Number               D02-D13
		*   Sprite flip X               D14
		*   Sprite flip Y               D15
		*
		* word#2
		*   Sprite X position           D00-D10
		*
		* word#3
		*   Sprite priority             D00-D02
		*   Sprite colour index         D04-D07
		*   Sprite Size X               D10-D15
		*/
		const u16 word3   = m_spriteram[offset + (loop * 4) + 3];
		const u16 word0   = m_spriteram[offset + (loop * 4) + 0];
		const u16 word1   = m_spriteram[offset + (loop * 4) + 1];
		const int sizey   = (word0 >> 10) & 0x003f;
		const int sizex = (word3 >> 10) & 0x003f;

		if (sizey && sizex)
		{
			u32 sprn;
			bool is_32;

			get_tilenum_and_size(word0, word1, sprn, is_32);

			const u32 prival = m_pri_cb(word3 & 0xf);
			const u16 offset4 = m_spriteram[offset + (loop * 4) + 2];
			const u32 color  = (word3 >> 4) & 0x000f;
			const int ypos   = (0x1ff - (word0 & 0x01ff)) - 0x50 + 0x02;
			const int xpos   = (offset4 & 0x07ff) - 0x50 + 0x07;
			const bool flipy = BIT(word1, 15);
			const bool flipx = BIT(word1, 14);

			if (!is_32)
				sgfx->set_source_clip(BIT(word1, 0) ? 16 : 0, 16, BIT(word1, 1) ? 16 : 0, 16);
			else
				sgfx->set_source_clip(0, 32, 0, 32);

			draw_single_sprite(
					m_renderbitmap,
					cliprect,
					sgfx,
					sprn, color,
					flipx, flipy,
					xpos, ypos,
					sizex, sizey,
					prival);
		}
	}
} /* draw_sprites */

void namcos2_sprite_metalhawk_device::draw_sprites(const rectangle &cliprect, int control)
{
	/**
	 * word#0
	 *  xxxxxx---------- ysize
	 *  ------x--------- sprite tile size
	 *  -------xxxxxxxxx screeny
	 *
	 * word#1
	 *  --xxxxxxxxxxxxxx tile
	 *
	 * word#2 (unused)
	 *
	 * word#3
	 *  xxxxxx---------- xsize
	 *  ------xxxxxxxxxx screenx
	 *
	 * word#4 (unused)
	 * word#5 (unused)
	 *
	 * word#6 (orientation)
	 *  ---------------x rot90
	 *  --------------x- flipx
	 *  -------------x-- flipy
	 *  ------------x--- tile size
	 *
	 * word#7
	 *  ------------xxxx priority
	 *  --------xxxx---- color
	 *  x--------------- unknown
	 */
	for (int loop = 0; loop < 128; loop++)
	{
		const u16 ypos  = m_spriteram[(loop * 8) + 0];
		const u16 xpos  = m_spriteram[(loop * 8) + 3];
		const int sizey = (ypos >> 10) & 0x003f;
		const int sizex = (xpos >> 10) & 0x003f;

		if (sizey && sizex)
		{
			const u16 attrs = m_spriteram[(loop * 8) + 7];

			const u32 prival = m_pri_cb((attrs & 0xf));
			const u16 tile  = m_spriteram[(loop * 8) + 1];
			const u16 flags = m_spriteram[(loop * 8) + 6];
			const u32 sprn  =  (tile >> 2) & 0x0fff;

			const bool is_bigsprite = BIT(flags, 3);
			const u32 color  = (attrs >> 4) & 0x000f;
			int sx           = (xpos & 0x03ff) - 0x50 + 0x07;
			int sy           = (0x1ff - (ypos & 0x01ff)) - 0x50 + 0x02;
			const bool flipx = BIT(flags, 1);
			const bool flipy = BIT(flags, 2);

			/* swap xy */
			const int rgn = (flags & 0x0001);

			gfx_element *const sgfx = gfx(rgn);

			if (is_bigsprite)
			{
				if (sizex < 0x20)
				{
					sx -= (0x20 - sizex) / 0x8;
				}
				if (sizey < 0x1f)
				{
					sy += (0x20 - (sizey + 1)) / 0xc;
				}
				sgfx->set_source_clip(0, 32, 0, 32);
			}
			else
				sgfx->set_source_clip(BIT(tile, 0) ? 16 : 0, 16, BIT(tile, 1) ? 16 : 0, 16);

			draw_single_sprite(
					m_renderbitmap,
					cliprect,
					sgfx,
					sprn, color,
					flipx, flipy,
					sx, sy,
					sizex, sizey,
					prival);
		}
	}
} /* draw_sprites_metalhawk */
