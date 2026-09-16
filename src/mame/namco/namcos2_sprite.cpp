// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria

/*
    Namco System 2 Sprites - found on Namco System 2 video board (standard type)

    based on namcoic.txt this probably consists of the following
    C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
    C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
    C135 - Checks is object is displayed on Current output line.
    C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

    C146 (80 pin QFP), after furrtek's reverse engineered RTL (tested on Cosmo Gang the Video):
    - CH0-CH31 carry four 8bpp pixels at a time, pixel k of colour plane g on CH(4g+k)
    - on a 12M rising edge with SL = 100 (SL2 = OBEN, SL1 = 6M, SL0 = /1H) the four pixels are
      loaded into eight 4-bit shift registers, one per colour plane, in reverse order when FLIP
      is high. Four transparency flags are loaded with them, a pixel being transparent when all
      eight of its planes are set.
    - any other 12M rising edge shifts, with 1s coming in, so the chip idles at colour 0xff.
      Pixel 3 of the group is the first one out (pixel 0 with FLIP), TRA flags transparent ones.
    - 1V (line parity) selects the line buffer being built: the pixels go out on LDA when it is
      high and on LDB when it is low, and the other bus is held at 0xff. That is how the buffer
      being scanned out gets cleared for its next turn.
    - LDA and LDB are tri-stated by /LOE0 and /LOE1, nothing else is read back.
    - no reset
    For the boards using obj_layout the four pixels the C146 sees are simply the big-endian
    32-bit word formed by the four object ROMs.

    The C106 zoom counter steps twice for each pixel out of the C146 (see draw_line).
    The one line delay of the real line buffers (a line is built while the previous one is
    displayed) is not reproduced; sprite RAM is sampled when the line is drawn.

    Metal Hawk requires a different draw function, so might use a different chip unless the hookup
    is just scrambled (needs checking). Its object ROMs hold one byte per pixel and the rotated
    sprites can't be a single four ROM load, so its pixels are passed to the C146 from the decoded
    graphics like everyone else's.

    "Shadow" sprites are used in the baseball games to remap the background tile pen, and they're
    also used a lot in valkyrie.

    Device used by the following drivers:
    namco/namcos2.cpp (all games EXCEPT Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours,
    Suzuka 8 Hours 2 which use the newer Namco NB1 style sprites, see shared/namco_c355spr.cpp).
*/

#include "emu.h"
#include "namcos2_sprite.h"

#include <algorithm>


namespace {

// four decoded pixels as they arrive on CH0-CH31: the leftmost one goes on the
// k = 3 lines, since that is the one the C146 sends out first
u32 c146_ch(const u8 *src)
{
	u32 ch = 0;
	for (int i = 0; i < 4; i++)
	{
		// spread the eight planes onto every fourth line
		u32 p = src[i];
		p = (p | (p << 12)) & 0x000f000f;
		p = (p | (p << 6)) & 0x03030303;
		p = (p | (p << 3)) & 0x11111111;
		ch |= p << (3 - i);
	}
	return ch;
}

} // anonymous namespace


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

	for (auto &buf : m_linebuf)
		std::fill(std::begin(buf), std::end(buf), 0xffff);

	save_item(NAME(m_c146.m_sr_col));
	save_item(NAME(m_c146.m_sr_tra));
}

/**************************************************************************************/

void namcos2_sprite_device::c146::load(u32 ch, bool flip)
{
	// OPA: all eight planes of the pixel set
	u8 opa = 0;
	for (int k = 0; k < 4; k++)
	{
		if (((ch >> k) & 0x11111111) == 0x11111111)
			opa |= 1 << k;
	}

	if (flip)
	{
		// the four pixels in reverse order, i.e. every nibble mirrored
		ch = ((ch & 0x55555555) << 1) | ((ch >> 1) & 0x55555555);
		ch = ((ch & 0x33333333) << 2) | ((ch >> 2) & 0x33333333);
		m_sr_tra = bitswap<4>(opa, 0, 1, 2, 3);
	}
	else
	{
		m_sr_tra = opa;
	}
	m_sr_col = ch;
}

void namcos2_sprite_device::c146::shift()
{
	m_sr_tra = ((m_sr_tra << 1) | 1) & 0x0f;
	m_sr_col = (m_sr_col << 1) | 0x11111111;
}

u8 namcos2_sprite_device::c146::out() const
{
	// SR_OUT: bit 3 of each register, gathered back into a colour
	u32 c = (m_sr_col >> 3) & 0x11111111;
	c = (c | (c >> 3)) & 0x03030303;
	c = (c | (c >> 6)) & 0x000f000f;
	c = (c | (c >> 12)) & 0x000000ff;
	return c;
}

/**************************************************************************************/

void namcos2_sprite_device::add_sprite(
		gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy,
		int sizex, int sizey, u32 prival, u8 srcx, u8 srcy, u8 size)
{
	sprite_entry &spr = m_sprite[m_sprite_count++];

	spr.gfx = gfx;
	spr.code = code % gfx->elements();
	spr.pri = (prival & 0xf) << 12;
	spr.pal = gfx->granularity() * (color % gfx->colors());
	spr.sx = sx & m_xmask;
	spr.sy = sy;
	spr.sizex = sizex;
	spr.sizey = sizey;
	spr.lutbank = (flipy ? 0x1000 : 0) | sizey;
	spr.srcx = srcx;
	spr.srcy = srcy;
	spr.size = size;
	spr.flipx = flipx;
}

void namcos2_sprite_device::draw_line(int y, const rectangle &cliprect)
{
	// 1V high: this line is built in line buffer A and the pixels leave the C146 on LDA
	const int v1 = BIT(y, 0);
	u16 *const lb = m_linebuf[v1 ? 0 : 1];

	for (int i = 0; i < m_sprite_count; i++)
	{
		const sprite_entry &spr = m_sprite[i];

		// is the object on this line?
		const int row = (y - spr.sy) & 0x1ff;
		if (row > spr.sizey)
			continue;

		int dy = m_scalelut_region[spr.lutbank | (row << 6)];
		if (dy > 0x1f)
			continue;
		if (spr.size < 32)
			dy >>= 1;

		const u8 *const src = spr.gfx->get_data(spr.code) + ((spr.srcy + dy) * spr.gfx->rowbytes()) + spr.srcx;
		int xx = spr.sx;
		int siz = 0;

		for (int n = 0; n < spr.size; n += 4)
		{
			// the groups of a flipped object are fetched backwards, the C146
			// turns the four pixels round inside each of them
			m_c146.load(c146_ch(&src[spr.flipx ? (spr.size - 4 - n) : n]), spr.flipx);

			for (int p = 0; p < 4; p++)
			{
				if (p)
					m_c146.shift();

				const u8 c = v1 ? m_c146.lda(v1) : m_c146.ldb(v1);
				const bool tra = m_c146.tra();

				// the zoom counter steps twice for every pixel out of the C146
				for (int step = 0; step < 2; step++)
				{
					if (!tra)
						lb[xx] = spr.pri | ((spr.pal + c) & 0xfff);

					siz += 1 + spr.sizex;
					if (siz >= 0x40)
					{
						xx = (xx + (siz >> 6)) & m_xmask;
						siz &= 0x3f;
					}
				}
			}
		}
	}

	// scan-out; while this buffer is read the next line is built in the other
	// one, so the C146 holds this bus at 0xff and the buffer comes back empty
	u16 *const dest = &m_renderbitmap.pix(y);
	std::copy(&lb[cliprect.min_x], &lb[cliprect.max_x + 1], &dest[cliprect.min_x]);
	std::fill_n(lb, std::size(m_linebuf[0]), 0xffff);
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
	get_sprites(control);
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		draw_line(y, cliprect);
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

void namcos2_sprite_device::get_sprites(int control)
{
	gfx_element *const sgfx = gfx(0);

	// cells are picked per entry, see add_sprite
	sgfx->set_source_clip(0, 32, 0, 32);

	m_sprite_count = 0;
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
			{
				add_sprite(sgfx, sprn, color, flipx, flipy, xpos, ypos, sizex, sizey, prival,
						BIT(word1, 0) ? 16 : 0, BIT(word1, 1) ? 16 : 0, 16);
			}
			else
			{
				add_sprite(sgfx, sprn, color, flipx, flipy, xpos, ypos, sizex, sizey, prival,
						0, 0, 32);
			}
		}
	}
} /* get_sprites */

void namcos2_sprite_metalhawk_device::get_sprites(int control)
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

	// cells are picked per entry, see add_sprite
	gfx(0)->set_source_clip(0, 32, 0, 32);
	gfx(1)->set_source_clip(0, 32, 0, 32);

	m_sprite_count = 0;
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
				add_sprite(sgfx, sprn, color, flipx, flipy, sx, sy, sizex, sizey, prival,
						0, 0, 32);
			}
			else
			{
				add_sprite(sgfx, sprn, color, flipx, flipy, sx, sy, sizex, sizey, prival,
						BIT(tile, 0) ? 16 : 0, BIT(tile, 1) ? 16 : 0, 16);
			}
		}
	}
} /* get_sprites_metalhawk */
