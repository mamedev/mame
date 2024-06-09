// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
/***************************************************************************

  Gaelco Type CG-1V/GAE1 Video Hardware

  Functions to emulate the video hardware of the machine

  CG-1V/GAE1 (Gaelco custom GFX & Sound chip):
    The CG-1V works with 16x16, 5 bpp gfx.
    It can handle:
        * 2 1024x512 tilemaps with linescroll.
        * 2 banks of 512 sprites (sprites can be grouped up to 16x16).
    Sprites can make the background darker or brighter.

    Memory map:
    ===========
        0x000000-0x000fff   Sprite bank #1  (1)
        0x001000-0x001fff   Sprite bank #2  (1)
        0x002000-0x0023ff   Linescroll tilemap #1 (2)
        0x002400-0x0027ff   Linescroll tilemap #2 (2)
            Linescroll entries are like this:
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                 i   | xxxxxx-- -------- | not used?
                 i   | ------xx xxxxxxxx | line i x scroll register

        0x002800-0x002807   Scroll registers
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                  0  | xxxxxxx- -------- | not used?
                  0  | -------x xxxxxxxx | tilemap #1 y scroll register
                  1  | xxxxxx-- -------- | not used?
                  1  | ------xx xxxxxxxx | tilemap #1 x scroll register
                  2  | xxxxxxx- -------- | not used?
                  2  | -------x xxxxxxxx | tilemap #2 y scroll register
                  3  | xxxxxx-- -------- | not used?
                  3  | ------xx xxxxxxxx | tilemap #2 x scroll register
        0x002890-0x0028ff   Sound registers (3)
        0x000000-0x00ffff   Video RAM
        0x010000-0x011fff   Palette (xRRRRRGGGGGBBBBB)
        0x018004-0x018007   Video Registers
                Word | Bit(s)            | Description
                -----+-FEDCBA98-76543210-+--------------------------
                  0  | x------- -------- | tilemap #1 linescroll enable
                  0  | -xxx---- -------- | not used?
                  0  | ----xxx- -------- | tilemap #1 video RAM bank? (4, 5)
                  0  | -------x -------- | not used?
                  0  | -------- x------- | unknown
                  0  | -------- -x------ | not used?
                  0  | -------- --xx---- | visible area size? (=0,480x240;=1,384x240;=2,320x240)
                  0  | -------- ----xxxx | not used?
                  1  | x------- -------- | tilemap #2 linescroll enable
                  1  | -xxx---- -------- | not used?
                  1  | ----xxx- -------- | tilemap #2 video RAM bank? (4, 5)
                  1  | -------x xxx----- | not used?
                  1  | -------- ---x---- | sprite bank select
                  1  | -------- ----xxxx | not used?
        0x018008-0x018009   Clear video int?

Notes:
    (1) See sprite format in the sprite section
    (2) x scroll register is not taken into account when doing line scroll
    (3) See devices/sound/gaelco.cpp for the sound register layout
    (4) tilemaps use the memory [0x2000*bank .. 0x2000*bank + 0x1fff]
    (5) See tile format in the tilemap section

Multi monitor notes:
    Some games have two RGB outputs to allow two or four simultaneous
    players linking two cabinets (World Rally 2, Touch & Go).

    In 2 monitors mode, the hardware maps one tilemap to a monitor and the
    other tilemap to the other monitor. The game palette is splitted, using
    the first half for one monitor and the second half for the other monitor.
    The sprite RAM has one bit that selects the sprite's target monitor. The
    sound is splitted too, right channel for cabinet 1 and the left channel
    for the other cabinet.

***************************************************************************/

#include "emu.h"
#include "gaelco2.h"
#include "screen.h"


/***************************************************************************

    Callbacks for the TileMap code (single monitor games)

    Tile format
    -----------

    Screen 0 & 1: (1024*512, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -----xxx | code (bits 18-16)
      0  | -------- --xxx--- | not used?
      0  | -------- -x------ | flip y
      0  | -------- x------- | flip x
      0  | -------x -------- | not used?
      0  | xxxxxxx- -------- | color
      1  | xxxxxxxx xxxxxxxx | code (bits 15-0)

***************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info)
{
	const u16 data = m_videoram[(((m_vregs[Layer] >> 9) & 0x07) * 0x2000 / 2) + (tile_index << 1)];
	const u16 data2 = m_videoram[(((m_vregs[Layer] >> 9) & 0x07) * 0x2000 / 2) + ((tile_index << 1) + 1)];
	const u32 code = ((data & 0x07) << 16) | (data2 & 0xffff);

	tileinfo.set(0, code, ((data >> 9) & 0x7f), TILE_FLIPXY((data >> 6) & 0x03));
}


/***************************************************************************

    Callbacks for the TileMap code (dual monitor games)

    Tile format
    -----------

    Screen 0 & 1: (1024*512, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -----xxx | code (bits 18-16)
      0  | -------- --xxx--- | not used?
      0  | -------- -x------ | flip y
      0  | -------- x------- | flip x
      0  | -------x -------- | not used?
      0  | -xxxxxx- -------- | color
      0  | x------- -------- | unknown
      1  | xxxxxxxx xxxxxxxx | code (bits 15-0)

***************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(gaelco2_state::get_tile_info_dual)
{
	const u16 data = m_videoram[(((m_vregs[Layer] >> 9) & 0x07) * 0x2000 / 2) + (tile_index << 1)];
	const u16 data2 = m_videoram[(((m_vregs[Layer] >> 9) & 0x07) * 0x2000 / 2) + ((tile_index << 1) + 1)];
	const u32 code = ((data & 0x07) << 16) | (data2 & 0xffff);

	tileinfo.set(0, code, (Layer * 0x40) + ((data >> 9) & 0x3f), TILE_FLIPXY((data >> 6) & 0x03));
}


/***************************************************************************

    Memory Handlers

***************************************************************************/

void gaelco2_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u32 pant0_start = ((m_vregs[0] >> 9) & 0x07) * 0x1000;
	const u32 pant0_end = pant0_start + 0x1000;
	const u32 pant1_start = ((m_vregs[1] >> 9) & 0x07) * 0x1000;
	const u32 pant1_end = pant1_start + 0x1000;

	COMBINE_DATA(&m_videoram[offset]);

	// tilemap 0 writes
	if ((offset >= pant0_start) && (offset < pant0_end))
		m_pant[0]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);

	// tilemap 1 writes
	if ((offset >= pant1_start) && (offset < pant1_end))
		m_pant[1]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);

}

void gaelco2_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 old = m_vregs[offset];
	data = COMBINE_DATA(&m_vregs[offset]);
	switch (offset)
	{
		case 0:
		case 1:
			if (((old ^ data) >> 9) & 7) // tilemap ram offset
			{
				m_pant[offset]->mark_all_dirty();
			}
			break;
	}
}

/***************************************************************************

    Palette (paletteram_xRRRRRGGGGGBBBBB_word_w)

    The game's palette uses colors 0-4095, but we need 15 aditional palettes
    to handle shadows and highlights properly. After a color write to the
    game's palette we update the other palettes with a darker/brighter color.

    Sprites use last palette entry for shadows and highlights
    (in order to make some pixels darker or brighter).

    The sprite's pens define the color adjustment:

    0x00 -> Transparent
    0x01-0x07 -> Shadow level (0x01 = min, 0x07 = max)
    0x08-0x0f -> Highlight level (0x08 = max, 0x0f = min)
    0x10-0x1f -> not used?

***************************************************************************/

static constexpr u8 RGB_CHG = 0x08;
static inline const u8 ADJUST_COLOR(s16 c) { return (c < 0) ? 0 : ((c > 255) ? 255 : c); }

// table used for color adjustment
static const s8 pen_color_adjust[16] = {
	+RGB_CHG * 0, -RGB_CHG * 1, -RGB_CHG * 2, -RGB_CHG * 3, -RGB_CHG * 4, -RGB_CHG * 5, -RGB_CHG * 6, -RGB_CHG * 7,
	+RGB_CHG * 8, +RGB_CHG * 7, +RGB_CHG * 6, +RGB_CHG * 5, +RGB_CHG * 4, +RGB_CHG * 3, +RGB_CHG * 2, +RGB_CHG * 1
};


void gaelco2_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	const u16 color = m_paletteram[offset];

	// extract RGB components
	const u8 r = pal5bit(color >>  10);
	const u8 g = pal5bit(color >>   5);
	const u8 b = pal5bit(color & 0x1f);

	// update game palette
	m_palette->set_pen_color(4096*0 + offset, rgb_t(r, g, b));

	// update shadow/highlight palettes
	for (int i = 1; i < 16; i++)
	{
		const u8 auxr = ADJUST_COLOR(r + pen_color_adjust[i]);
		const u8 auxg = ADJUST_COLOR(g + pen_color_adjust[i]);
		const u8 auxb = ADJUST_COLOR(b + pen_color_adjust[i]);

		m_palette->set_pen_color(4096*i + offset, rgb_t(auxr, auxg, auxb));
	}
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaelco2_state,gaelco2)
{
	m_videoram = m_spriteram->live();

	// create tilemaps
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	// set tilemap properties
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

	m_pant[0]->set_scroll_rows(512);
	m_pant[0]->set_scroll_cols(1);
	m_pant[1]->set_scroll_rows(512);
	m_pant[1]->set_scroll_cols(1);

	m_dual_monitor = false;
}

VIDEO_START_MEMBER(gaelco2_state,gaelco2_dual)
{
	m_videoram = m_spriteram->live();

	// create tilemaps
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info_dual<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info_dual<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	// set tilemap properties
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

	m_pant[0]->set_scroll_rows(512);
	m_pant[0]->set_scroll_cols(1);
	m_pant[1]->set_scroll_rows(512);
	m_pant[1]->set_scroll_cols(1);

	m_dual_monitor = true;
}

/***************************************************************************

    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------x xxxxxxxx | sprite bank (sprite number bits 18-10)
      0  | xxxxxxx- -------- | sprite color (bits 6-0)
      1  | -------x xxxxxxxx | y position
      1  | ------x- -------- | sprite enable
      1  | -----x-- -------- | flipy
      1  | ----x--- -------- | flipx
      1  | xxxx---- -------- | sprite y size
      2  | ------xx xxxxxxxx | x position
      2  | ----xx-- -------- | not used?
      2  | xxxx---- -------- | sprite x size
      3  | xxxxxxxx xxxxxxxx | pointer to more sprite data

      Each sprite entry has a pointer to more sprite data.
      The length of data depends on the sprite size (xsize*ysize).
      Each entry has the following format:

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | ----xxxx xxxxxxxx | sprite number offset (sprite number bits 11-0)
      0  | xxxx---- -------- | sprite color offset (bits 3-0)

      In dual monitor games, the configuration is the same, but MSB of
      word 0 is used to select target monitor for the sprite, and the
      palette is splitted for each monitor.

      Last sprite color entry is used for shadows/highlights

***************************************************************************/

void gaelco2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask)
{
	u16 const *const buffered_spriteram16 = m_spriteram->buffer();
	gfx_element *gfx = m_gfxdecode->gfx(0);

	// get sprite ram start and end offsets
	const u32 start_offset = (m_vregs[1] & 0x10) * 0x100;
	const u32 end_offset = start_offset + 0x1000;

	// sprite offset is based on the visible area - this seems very kludgy
	const int spr_x_adjust = (screen.visible_area().max_x - 320 + 1) - (511 - 320 - 1) - ((m_vregs[0] >> 4) & 0x01) + m_global_spritexoff;

	for (int j = start_offset; j < end_offset; j += 8)
	{
		const u16 data = buffered_spriteram16[(j / 2) + 0];
		const u16 data2 = buffered_spriteram16[(j / 2) + 1];
		const u16 data3 = buffered_spriteram16[(j / 2) + 2];
		const u16 data4 = buffered_spriteram16[(j / 2) + 3];

		const int sx = data3 & 0x3ff;
		const int sy = data2 & 0x1ff;

		const bool xflip = BIT(data2, 11);
		const bool yflip = BIT(data2, 10);

		const int xsize = ((data3 >> 12) & 0x0f) + 1;
		const int ysize = ((data2 >> 12) & 0x0f) + 1;

		if (m_dual_monitor && ((data & 0x8000) != mask)) continue;

		// if it's enabled, draw it
		if (BIT(data2, 9))
		{
			for (int y = 0; y < ysize; y++)
			{
				for (int x = 0; x < xsize; x++)
				{
					// for each x,y of the sprite, fetch the sprite data
					const u16 data5 = buffered_spriteram16[((data4 / 2) + (y * xsize + x)) & 0x7fff];
					const u32 number = ((data & 0x1ff) << 10) + (data5 & 0x0fff);
					const u32 color = ((data >> 9) & 0x7f) + ((data5 >> 12) & 0x0f);
					const bool color_effect = m_dual_monitor ? ((color & 0x3f) == 0x3f) : (color == 0x7f);

					const int ex = xflip ? (xsize - 1 - x) : x;
					const int ey = yflip ? (ysize - 1 - y) : y;

					if (!color_effect)
					{ // normal sprite, pen 0 transparent
							gfx->transpen(bitmap,cliprect, number,
									color, xflip, yflip,
									((sx + ex * 16) & 0x3ff) + spr_x_adjust,
									((sy + ey * 16) & 0x1ff), 0);
					}
					else
					{ // last palette entry is reserved for shadows and highlights

						// get a pointer to the current sprite's gfx data
						u8 const *const gfx_src = gfx->get_data(number % gfx->elements());

						for (int py = 0; py < gfx->height(); py++)
						{
							// get a pointer to the current line in the screen bitmap
							const int ypos = ((sy + ey * 16 + py) & 0x1ff);

							if ((ypos < cliprect.min_y) || (ypos > cliprect.max_y)) continue;

							const int gfx_py = yflip ? (gfx->height() - 1 - py) : py;
							u16 *const srcy = &bitmap.pix(ypos);

							for (int px = 0; px < gfx->width(); px++)
							{
								// get current pixel
								const int xpos = (((sx + ex * 16 + px) & 0x3ff) + spr_x_adjust) & 0x3ff;
								if ((xpos < cliprect.min_x) || (xpos > cliprect.max_x))
									continue;

								const int gfx_px = xflip ? (gfx->width() - 1 - px) : px;

								// get asociated pen for the current sprite pixel
								const u8 gfx_pen = gfx_src[gfx->rowbytes() * gfx_py + gfx_px];

								if ((gfx_pen == 0) || (gfx_pen >= 16)) continue;

								u16 *const pixel = srcy + xpos;
								const u16 src_color = *pixel;

								// make background color darker or brighter
								*pixel = (src_color & 0xfff) | (0x1000 * gfx_pen);
							}
						}
					}
				}
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

u32 gaelco2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int xoff0 = 0x14;
	const int xoff1 = xoff0 - 4;
	const int yoff0 = 0x01;
	const int yoff1 = 0x01;

	// read scroll values
	const int scroll0x = m_videoram[0x2802 / 2] + xoff0;
	const int scroll1x = m_videoram[0x2806 / 2] + xoff1;
	const int scroll0y = m_videoram[0x2800 / 2] + yoff0;
	const int scroll1y = m_videoram[0x2804 / 2] + yoff1;

	// set y scroll registers
	m_pant[0]->set_scrolly(0, scroll0y & 0x1ff);
	m_pant[1]->set_scrolly(0, scroll1y & 0x1ff);

	// set x linescroll registers
	for (int i = 0; i < 512; i++)
	{
		m_pant[0]->set_scrollx(i & 0x1ff, (BIT(m_vregs[0], 15) ? (m_videoram[(0x2000 / 2) + i] + xoff0) : scroll0x) & 0x3ff);
		m_pant[1]->set_scrollx(i & 0x1ff, (BIT(m_vregs[1], 15) ? (m_videoram[(0x2400 / 2) + i] + xoff1) : scroll1x) & 0x3ff);
	}

	// draw screen
	bitmap.fill(0, cliprect);

	m_pant[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(screen, bitmap, cliprect, 0);
	return 0;
}

	/*
	The Y-scroll value in rowscroll mode has a very unusual implementation
	the scroll value seems come from a 0x1000 bit being set in one of the
	512 rowscroll table entries.

	Only the bit representing the largest value is used?

	The ordering of the bits does not appear to be immediatley obvious.

	This is used in Touch & Go for the 'Super Spike' but we only scroll
	32 possible values, the entries used for the other scroll values
	are unknown unless a pattern can be derived, or figured out from
	the game code.

	_______________________________________ entry number
	                    1111 1111 1111 1111
	0123 4567 89ab cdef 0123 4567 89ab cdef
	_______________________________________ offset into linescroll ram
	0000 0000 0000 1111 1111 1111 1111 1111
	6677 7888 8999 0001 1112 2233 3444 4555
	5814 7036 9258 1470 3692 5814 7036 9258
	_______________________________________
	                                         scr = expected scroll value if bit 0x1000 at offset is set

	                                         scr (entry from above)
	--x- ---- ---- ---- ---- ---- ---- ----  32  (02)
	---- ---- ---- --x- ---- ---- ---- ----  31  (0e)
	---- ---- --x- ---- ---- ---- ---- ----  30  (0a)
	---- ---- ---- ---- ---- ---- ---x ----  29  (1b)
	---- ---- ---- ---- ---- -x-- ---- ----  28  (15)
	---- ---- ---- ---- ---x ---- ---- ----  27  (13)
	---- x--- ---- ---- ---- ---- ---- ----  26  (04)
	---- ---- ---- ---- ---- ---- -x-- ----  25  (19)
	---- ---- ---- x--- ---- ---- ---- ----  24  (0c)
	---- ---- ---- ---- ---- ---- ---- ---x  23  (1f)
	---- ---- x--- ---- ---- ---- ---- ----  22  (08)
	---- ---- ---- ---- ---- ---- ---- -x--  21  (1d)
	---- ---- ---- ---- -x-- ---- ---- ----  20  (11)
	---- --x- ---- ---- ---- ---- ---- ----  19  (06)
	---- ---- ---- ---- ---- ---x ---- ----  18  (17)
	---- ---- ---- ---- x--- ---- ---- ----  17  (10)
	-x-- ---- ---- ---- ---- ---- ---- ----  16  (01)
	---- ---- ---- -x-- ---- ---- ---- ----  15  (0d)
	---- ---- ---- ---- ---- --x- ---- ----  14  (16)
	---- ---- -x-- ---- ---- ---- ---- ----  13  (09)
	---- ---- ---- ---- ---- ---- --x- ----  12  (1a)
	---- -x-- ---- ---- ---- ---- ---- ----  11  (05)
	---- ---- ---- ---- ---- ---- ---- --x-  10  (1e)
	x--- ---- ---- ---- ---- ---- ---- ----  9   (00)
	---- ---- ---- ---- --x- ---- ---- ----  8   (12)
	---- ---- ---x ---- ---- ---- ---- ----  7   (0b)
	---- ---- ---- ---- ---- x--- ---- ----  6   (14)
	---x ---- ---- ---- ---- ---- ---- ----  5   (03)
	---- ---- ---- ---- ---- ---- ---- x---  4   (1c)
	---- ---- ---- ---- ---- ---- x--- ----  3   (18)
	---- ---x ---- ---- ---- ---- ---- ----  2   (07)
	---- ---- ---- ---x ---- ---- ---- ----  1   (0f)

	================================================== sorted list for reference

	x--- ---- ---- ---- ---- ---- ---- ----  9
	-x-- ---- ---- ---- ---- ---- ---- ----  16
	--x- ---- ---- ---- ---- ---- ---- ----  32
	---x ---- ---- ---- ---- ---- ---- ----  5

	---- x--- ---- ---- ---- ---- ---- ----  26
	---- -x-- ---- ---- ---- ---- ---- ----  11
	---- --x- ---- ---- ---- ---- ---- ----  19
	---- ---x ---- ---- ---- ---- ---- ----  2

	---- ---- x--- ---- ---- ---- ---- ----  22
	---- ---- -x-- ---- ---- ---- ---- ----  13
	---- ---- --x- ---- ---- ---- ---- ----  30
	---- ---- ---x ---- ---- ---- ---- ----  7

	---- ---- ---- x--- ---- ---- ---- ----  24
	---- ---- ---- -x-- ---- ---- ---- ----  15
	---- ---- ---- --x- ---- ---- ---- ----  31
	---- ---- ---- ---x ---- ---- ---- ----  1

	---- ---- ---- ---- x--- ---- ---- ----  17
	---- ---- ---- ---- -x-- ---- ---- ----  20
	---- ---- ---- ---- --x- ---- ---- ----  8
	---- ---- ---- ---- ---x ---- ---- ----  27

	---- ---- ---- ---- ---- x--- ---- ----  6
	---- ---- ---- ---- ---- -x-- ---- ----  28
	---- ---- ---- ---- ---- --x- ---- ----  14
	---- ---- ---- ---- ---- ---x ---- ----  18

	---- ---- ---- ---- ---- ---- x--- ----  3
	---- ---- ---- ---- ---- ---- -x-- ----  25
	---- ---- ---- ---- ---- ---- --x- ----  12
	---- ---- ---- ---- ---- ---- ---x ----  29

	---- ---- ---- ---- ---- ---- ---- x---  4
	---- ---- ---- ---- ---- ---- ---- -x--  21
	---- ---- ---- ---- ---- ---- ---- --x-  10
	---- ---- ---- ---- ---- ---- ---- ---x  23

*/

int gaelco2_state::get_rowscrollmode_yscroll(bool first_screen)
{
	const u16 base = first_screen ? 0x2000 / 2 : 0x2400 / 2;

	const u8 checkoffsets[32] = {
		0x02, 0x0e, 0x0a, 0x1b, 0x15, 0x13, 0x04, 0x19,
		0x0c, 0x1f, 0x08, 0x1d, 0x11, 0x06, 0x17, 0x10,
		0x01, 0x0d, 0x16, 0x09, 0x1a, 0x05, 0x1e, 0x00,
		0x12, 0x0b, 0x14, 0x03, 0x1c, 0x18, 0x07, 0x0f };

	int usescroll = 0;
	for (int i = 31; i >= 0; i--)
	{
		const int checkoffset = (0x80 / 2) + ((checkoffsets[i] * 3) + 1);

		if (BIT(m_videoram[base + checkoffset], 12))
		{
			usescroll = 31 - i;
		}
	}

	return usescroll;
}

u32 gaelco2_state::dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index)
{
	const int xoff0 = 0x14; // intro scenes align better with 0x13, but test screen is definitely 0x14
	const int xoff1 = xoff0 - 4;
	const int yoff0 = 0x01;
	const int yoff1 = 0x01;

	// read scroll values
	const int scroll0x = m_videoram[0x2802 / 2] + xoff0;
	const int scroll1x = m_videoram[0x2806 / 2] + xoff1;
	int scroll0y = m_videoram[0x2800 / 2] + yoff0;
	int scroll1y = m_videoram[0x2804 / 2] + yoff1;

	if (BIT(m_vregs[0], 15))
	{
		scroll0y += get_rowscrollmode_yscroll(true);
	}

	if (BIT(m_vregs[1], 15))
	{
		scroll1y += get_rowscrollmode_yscroll(false);
	}

	// set y scroll registers
	m_pant[0]->set_scrolly(0, scroll0y & 0x1ff);
	m_pant[1]->set_scrolly(0, scroll1y & 0x1ff);

	// set x linescroll registers
	for (int i = 0; i < 512; i++)
	{
		m_pant[0]->set_scrollx(i & 0x1ff, (BIT(m_vregs[0], 15) ? (m_videoram[(0x2000 / 2) + i] + xoff0) : scroll0x) & 0x3ff);
		m_pant[1]->set_scrollx(i & 0x1ff, (BIT(m_vregs[1], 15) ? (m_videoram[(0x2400 / 2) + i] + xoff1) : scroll1x) & 0x3ff);
	}

	// draw screen
	bitmap.fill(0, cliprect);

	m_pant[index]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(screen, bitmap, cliprect, 0x8000 * index);

	return 0;
}

u32 gaelco2_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return dual_update(screen, bitmap, cliprect, 0); }
u32 gaelco2_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return dual_update(screen, bitmap, cliprect, 1); }
