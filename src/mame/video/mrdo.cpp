// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  mrdo.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mrdo.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mr. Do! has two 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROMs are connected to the RGB output this way:

  U2:
  bit 7 -- unused
        -- unused
        -- 100 ohm resistor  -diode- BLUE
        --  75 ohm resistor  -diode- BLUE
        -- 100 ohm resistor  -diode- GREEN
        --  75 ohm resistor  -diode- GREEN
        -- 100 ohm resistor  -diode- RED
  bit 0 --  75 ohm resistor  -diode- RED

  T2:
  bit 7 -- unused
        -- unused
        -- 150 ohm resistor  -diode- BLUE
        -- 120 ohm resistor  -diode- BLUE
        -- 150 ohm resistor  -diode- GREEN
        -- 120 ohm resistor  -diode- GREEN
        -- 150 ohm resistor  -diode- RED
  bit 0 -- 120 ohm resistor  -diode- RED

  200 ohm pulldown on all three components

***************************************************************************/

void mrdo_state::palette_init(palette_device &palette) const
{
	constexpr int R1 = 150;
	constexpr int R2 = 120;
	constexpr int R3 = 100;
	constexpr int R4 = 75;
	constexpr int pull = 220;
	constexpr float potadjust = 0.7f;   /* diode voltage drop */

	float pot[16];
	int weight[16];
	for (int i = 0x0f; i >= 0; i--)
	{
		float par = 0;

		if (i & 1) par += 1.0f / float(R1);
		if (i & 2) par += 1.0f / float(R2);
		if (i & 4) par += 1.0f / float(R3);
		if (i & 8) par += 1.0f / float(R4);
		if (par)
		{
			par = 1 / par;
			pot[i] = pull/(pull+par) - potadjust;
		}
		else
			pot[i] = 0;

		weight[i] = 0xff * pot[i] / pot[0x0f];
		if (weight[i] < 0)
			weight[i] = 0;
	}

	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		int bits0, bits2;

		int const a1 = ((i >> 3) & 0x1c) + (i & 0x03) + 0x20;
		int const a2 = ((i >> 0) & 0x1c) + (i & 0x03);

		// red component
		bits0 = (color_prom[a1] >> 0) & 0x03;
		bits2 = (color_prom[a2] >> 0) & 0x03;
		int const r = weight[bits0 + (bits2 << 2)];

		// green component
		bits0 = (color_prom[a1] >> 2) & 0x03;
		bits2 = (color_prom[a2] >> 2) & 0x03;
		int const g = weight[bits0 + (bits2 << 2)];

		// blue component
		bits0 = (color_prom[a1] >> 4) & 0x03;
		bits2 = (color_prom[a2] >> 4) & 0x03;
		int const b = weight[bits0 + (bits2 << 2)];

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x40;

	// characters
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	// sprites
	for (int i = 0; i < 0x40; i++)
	{
		uint8_t ctabentry = color_prom[i & 0x1f];

		if (i & 0x20)
			ctabentry >>= 4;    // high 4 bits are for sprite color n + 8
		else
			ctabentry &= 0x0f;  // low 4 bits are for sprite color n

		palette.set_pen_indirect(i + 0x100, ctabentry + ((ctabentry & 0x0c) << 3));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mrdo_state::get_bg_tile_info)
{
	uint8_t attr = m_bgvideoram[tile_index];
	tileinfo.set(1,
			m_bgvideoram[tile_index + 0x400] + ((attr & 0x80) << 1),
			attr & 0x3f,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(mrdo_state::get_fg_tile_info)
{
	uint8_t attr = m_fgvideoram[tile_index];
	tileinfo.set(0,
			m_fgvideoram[tile_index+0x400] + ((attr & 0x80) << 1),
			attr & 0x3f,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mrdo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mrdo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mrdo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_flipscreen = 0;

	save_item(NAME(m_flipscreen));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void mrdo_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void mrdo_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);

	protection_w(data);
}

void mrdo_state::scrollx_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, data);
}

void mrdo_state::scrolly_w(uint8_t data)
{
	/* This is NOT affected by flipscreen (so stop it happening) */
	if (m_flipscreen)
		m_bg_tilemap->set_scrolly(0,((256 - data) & 0xff));
	else
		m_bg_tilemap->set_scrolly(0, data);
}


void mrdo_state::flipscreen_w(uint8_t data)
{
	/* bits 1-3 control the playfield priority, but they are not used by */
	/* Mr. Do! so we don't emulate them */
	m_flipscreen = data & 0x01;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void mrdo_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (m_spriteram[offs + 1] != 0)
		{
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					m_spriteram[offs], m_spriteram[offs + 2] & 0x0f,
					m_spriteram[offs + 2] & 0x10, m_spriteram[offs + 2] & 0x20,
					m_spriteram[offs + 3], 256 - m_spriteram[offs + 1], 0);
		}
	}
}

uint32_t mrdo_state::screen_update_mrdo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
