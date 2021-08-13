// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  ddribble.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ddribble.h"


void ddribble_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0x10; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	// sprite #2 uses pens 0x00-0x0f
	for (int i = 0x0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}
}


void ddribble_state::K005885_0_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x03:  // char bank selection for set 1
			if ((data & 0x03) != m_charbank[0])
			{
				m_charbank[0] = data & 0x03;
				m_fg_tilemap->mark_all_dirty();
			}
			break;
		case 0x04:  // IRQ control, flipscreen
			m_int_enable[0] = data & 0x02;
			break;
	}
	m_vregs[0][offset] = data;
}

void ddribble_state::K005885_1_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x03:  // char bank selection for set 2
			if ((data & 0x03) != m_charbank[1])
			{
				m_charbank[1] = data & 0x03;
				m_bg_tilemap->mark_all_dirty();
			}
			break;
		case 0x04:  // IRQ control, flipscreen
			m_int_enable[1] = data & 0x02;
			break;
	}
	m_vregs[1][offset] = data;
}

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(ddribble_state::tilemap_scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);    // skip 0x400
}

TILE_GET_INFO_MEMBER(ddribble_state::get_fg_tile_info)
{
	uint8_t attr = m_fg_videoram[tile_index];
	int num = m_fg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + ((m_charbank[0] & 2) << 10);
	tileinfo.set(0,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(ddribble_state::get_bg_tile_info)
{
	uint8_t attr = m_bg_videoram[tile_index];
	int num = m_bg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + (m_charbank[1] << 11);
	tileinfo.set(1,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void ddribble_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddribble_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(ddribble_state::tilemap_scan)), 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddribble_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(ddribble_state::tilemap_scan)), 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

void ddribble_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xbff);
}

void ddribble_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xbff);
}

/***************************************************************************

    Double Dribble sprites

Each sprite has 5 bytes:
byte #0:    sprite number
byte #1:
    bits 0..2:  sprite bank #
    bit 3:      not used?
    bits 4..7:  sprite color
byte #2:    y position
byte #3:    x position
byte #4:    attributes
    bit 0:      x position (high bit)
    bit 1:      ???
    bits 2..4:  sprite size
    bit 5:      flip x
    bit 6:      flip y
    bit 7:      unused?

***************************************************************************/

void ddribble_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen)
{
	gfx_element *gfx = m_gfxdecode->gfx(gfxset);
	const uint8_t *finish = source + lenght;

	while (source < finish)
	{
		int number = source[0] | ((source[1] & 0x07) << 8); // sprite number
		int attr = source[4];                               // attributes
		int sx = source[3] | ((attr & 0x01) << 8);          // vertical position
		int sy = source[2];                                 // horizontal position
		int flipx = attr & 0x20;                            // flip x
		int flipy = attr & 0x40;                            // flip y
		int color = (source[1] & 0xf0) >> 4;                // color
		int width, height;

		if (flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 240 - sx;
			sy = 240 - sy;

			if ((attr & 0x1c) == 0x10)
			{   // ???. needed for some sprites in flipped mode
				sx -= 0x10;
				sy -= 0x10;
			}
		}

		switch (attr & 0x1c)
		{
			case 0x10:  // 32x32
				width = height = 2; number &= (~3); break;
			case 0x08:  // 16x32
				width = 1; height = 2; number &= (~2); break;
			case 0x04:  // 32x16
				width = 2; height = 1; number &= (~1); break;
			// the hardware allows more sprite sizes, but ddribble doesn't use them
			default:    // 16x16
				width = height = 1; break;
		}

		{
			static const int x_offset[2] = { 0x00, 0x01 };
			static const int y_offset[2] = { 0x00, 0x02 };

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int ex = flipx ? (width - 1 - x) : x;
					int ey = flipy ? (height - 1 - y) : y;


						gfx->transpen(bitmap, cliprect,
						(number) + x_offset[ex] + y_offset[ey],
						color,
						flipx, flipy,
						sx + x * 16, sy + y * 16, 0);
				}
			}
		}
		source += 5;
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t ddribble_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->set_flip((m_vregs[0][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_bg_tilemap->set_flip((m_vregs[1][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	// set scroll registers
	m_fg_tilemap->set_scrollx(0, m_vregs[0][1] | ((m_vregs[0][2] & 0x01) << 8));
	m_bg_tilemap->set_scrollx(0, m_vregs[1][1] | ((m_vregs[1][2] & 0x01) << 8));
	m_fg_tilemap->set_scrolly(0, m_vregs[0][0]);
	m_bg_tilemap->set_scrolly(0, m_vregs[1][0]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_spriteram[0], 0x07d, 2, m_vregs[0][4] & 0x08);
	draw_sprites(bitmap, cliprect, m_spriteram[1], 0x140, 3, m_vregs[1][4] & 0x08);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
