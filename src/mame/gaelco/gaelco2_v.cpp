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
                 i   | xxxx---- -------- | line i extra vertical step (6)
                 i   | ----xx-- -------- | not used?
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
                  0  | -x------ -------- | tilemap #1 vertical step direction (=0 down, =1 up) (6)
                  0  | --xx---- -------- | not used?
                  0  | ----xxx- -------- | tilemap #1 video RAM bank? (4, 5)
                  0  | -------x -------- | not used?
                  0  | -------- x------- | tilemap #1 over sprites (GAE1 PRI)
                  0  | -------- -x------ | not used?
                  0  | -------- --xx---- | visible area size? (=0,480x240;=1,384x240;=2,320x240)
                  0  | -------- ----xxxx | not used?
                  1  | x------- -------- | tilemap #2 linescroll enable
                  1  | -x------ -------- | tilemap #2 vertical step direction (=0 down, =1 up) (6)
                  1  | --xx---- -------- | not used?
                  1  | ----xxx- -------- | tilemap #2 video RAM bank? (4, 5)
                  1  | -------x xx------ | not used?
                  1  | -------- --x----- | tilemaps x flip (GAE1 FLIP, not emulated)
                  1  | -------- ---x---- | sprite bank select
                  1  | -------- ----xxxx | not used?
        0x018008-0x018009   Clear video int?

Notes:
    (1) See sprite format in the sprite section
    (2) x scroll register is not taken into account when doing line scroll
    (3) See devices/sound/gaelco.cpp for the sound register layout
    (4) tilemaps use the memory [0x2000*bank .. 0x2000*bank + 0x1fff]
    (5) See tile format in the tilemap section
    (6) The GAE1 reloads the tilemap line to draw from the y scroll register
        during vblank, and steps it once per scanline. In linescroll mode the
        extra step from the entry of the current tilemap line is added, which
        Touch & Go uses to squash part of its background vertically.

Multi monitor notes:
    Some games have two RGB outputs to allow two or four simultaneous
    players linking two cabinets (World Rally 2, Touch & Go).

    In 2 monitors mode, the hardware maps one tilemap to a monitor and the
    other tilemap to the other monitor. The game palette is splitted, using
    the first half for one monitor and the second half for the other monitor.
    The sprite RAM has one bit that selects the sprite's target monitor. The
    sound is splitted too, right channel for cabinet 1 and the left channel
    for the other cabinet.

    According to the GAE1 schematics, a high level on its CTR pin makes the
    chip drive the tilemap #2 pixels out through the pins that normally read
    the sprite pixels back from the external frame buffer. The dual monitor
    boards presumably tie CTR high: the chip's own pixel output then never
    sees a sprite, and both games keep PRI set so it shows tilemap #1 over
    tilemap #2. The sprites are merged outside the chip, so PRI doesn't apply
    to the dual monitor screens.

***************************************************************************/

#include "emu.h"
#include "gaelco2.h"

#include "screen.h"

#include <algorithm>


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

template <unsigned Layer>
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

template <unsigned Layer>
TILE_GET_INFO_MEMBER(gaelco2_dual_state::get_tile_info_dual)
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
	if ((offset == 0) && !m_dual_monitor && ((old ^ data) & mem_mask & 0x0080))
	{
		for (screen_device &screen : screen_device_enumerator(*this))
			screen.update_partial(screen.vpos());
	}
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
static constexpr u8 ADJUST_COLOR(s16 c) { return std::clamp<s16>(c, 0, 255); }

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

void gaelco2_state::video_start()
{
	m_videoram = m_spriteram->live();

	// create tilemaps
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	// set tilemap properties
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

	m_dual_monitor = false;
}

void gaelco2_dual_state::video_start()
{
	m_videoram = m_spriteram->live();

	// create tilemaps
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_dual_state::get_tile_info_dual<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaelco2_dual_state::get_tile_info_dual<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	// set tilemap properties
	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);

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
	const int spr_x_adjust = (screen.visible_area().right() - 320 + 1) - (511 - 320 - 1) - ((m_vregs[0] >> 4) & 0x01) + m_global_spritexoff;

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
				const int ey = yflip ? (ysize - 1 - y) : y;
				for (int x = 0; x < xsize; x++)
				{
					// for each x,y of the sprite, fetch the sprite data
					const u16 data5 = buffered_spriteram16[((data4 / 2) + (y * xsize + x)) & 0x7fff];
					const u32 number = ((data & 0x1ff) << 10) + (data5 & 0x0fff);
					const u32 color = ((data >> 9) & 0x7f) + ((data5 >> 12) & 0x0f);
					const bool color_effect = m_dual_monitor ? ((color & 0x3f) == 0x3f) : (color == 0x7f);

					const int ex = xflip ? (xsize - 1 - x) : x;

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
							if (!cliprect.containsy(ypos))
								continue;

							const int gfx_py = yflip ? (gfx->height() - 1 - py) : py;
							u8 const *const gfx_row = &gfx_src[gfx->rowbytes() * gfx_py];
							u16 *const srcy = &bitmap.pix(ypos);

							for (int px = 0; px < gfx->width(); px++)
							{
								// get current pixel
								const int xpos = (((sx + ex * 16 + px) & 0x3ff) + spr_x_adjust) & 0x3ff;
								if (!cliprect.containsx(xpos))
									continue;

								const int gfx_px = xflip ? (gfx->width() - 1 - px) : px;

								// get asociated pen for the current sprite pixel
								const u8 gfx_pen = gfx_row[gfx_px];

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

void gaelco2_state::draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, unsigned layer)
{
	// dual monitor intro scenes align better with 0x13, but its test screen is definitely 0x14
	const int xoff = layer ? 0x10 : 0x14;

	const u16 *const linescroll = &m_videoram[(0x2000 / 2) + (layer * 0x200)];
	const bool linescroll_en = BIT(m_vregs[layer], 15);
	const bool step_up = BIT(m_vregs[layer], 14);
	const u16 scrollx = m_videoram[(0x2802 / 2) + (layer * 2)];

	// tilemap line for the first scanline, stepped once from the y scroll register
	int line = m_videoram[(0x2800 / 2) + (layer * 2)] + (step_up ? -1 : 1);

	// without linescroll every step is one line down, so the layer can be drawn in one go
	if (!linescroll_en && !step_up)
	{
		m_pant[layer]->set_scrollx(0, (scrollx + xoff) & 0x3ff);
		m_pant[layer]->set_scrolly(0, line & 0x1ff);
		m_pant[layer]->draw(screen, bitmap, cliprect, 0, 0);
		return;
	}

	for (int y = 0; y <= cliprect.max_y; y++)
	{
		line &= 0x1ff;
		const u16 entry = linescroll_en ? linescroll[line] : scrollx;
		if (y >= cliprect.min_y)
		{
			const rectangle clip(cliprect.min_x, cliprect.max_x, y, y);
			m_pant[layer]->set_scrollx(0, (entry + xoff) & 0x3ff);
			m_pant[layer]->set_scrolly(0, (line - y) & 0x1ff);
			m_pant[layer]->draw(screen, bitmap, clip, 0, 0);
		}

		const int step = 1 + (linescroll_en ? BIT(entry, 12, 4) : 0);
		line += step_up ? -step : step;
	}
}

u32 gaelco2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_tilemap(screen, bitmap, cliprect, 1);
	if (BIT(m_vregs[0], 7))
	{
		draw_sprites(screen, bitmap, cliprect, 0);
		draw_tilemap(screen, bitmap, cliprect, 0);
	}
	else
	{
		draw_tilemap(screen, bitmap, cliprect, 0);
		draw_sprites(screen, bitmap, cliprect, 0);
	}
	return 0;
}

u32 gaelco2_dual_state::dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index)
{
	bitmap.fill(0, cliprect);

	draw_tilemap(screen, bitmap, cliprect, index);
	draw_sprites(screen, bitmap, cliprect, 0x8000 * index);

	return 0;
}

u32 gaelco2_dual_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return dual_update(screen, bitmap, cliprect, 0);
}

u32 gaelco2_dual_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return dual_update(screen, bitmap, cliprect, 1);
}
