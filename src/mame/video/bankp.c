// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bankp.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Bank Panic has a 32x8 palette PROM (I'm not sure whether the second 16
  bytes are used - they contain the same colors as the first 16 with only
  one different) and two 256x4 lookup table PROMs (one for charset #1, one
  for charset #2 - only the first 128 nibbles seem to be used).

  I don't know for sure how the palette PROM is connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(bankp_state, bankp)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r,g,b));

		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */

	/* charset #1 lookup table */
	for (i = 0; i < m_gfxdecode->gfx(0)->colors() * m_gfxdecode->gfx(0)->granularity(); i++)
		palette.set_pen_indirect(m_gfxdecode->gfx(0)->colorbase() + i, *color_prom++ & 0x0f);

	color_prom += 128;  /* skip the bottom half of the PROM - seems to be not used */

	/* charset #2 lookup table */
	for (i = 0; i < m_gfxdecode->gfx(1)->colors() * m_gfxdecode->gfx(1)->granularity(); i++)
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + i, *color_prom++ & 0x0f);

	/* the bottom half of the PROM seems to be not used */
}

WRITE8_MEMBER(bankp_state::bankp_scroll_w)
{
	m_scroll_x = data;
}

WRITE8_MEMBER(bankp_state::bankp_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bankp_state::bankp_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bankp_state::bankp_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bankp_state::bankp_colorram2_w)
{
	m_colorram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bankp_state::bankp_out_w)
{
	/* bits 0-1 are playfield priority */
	/* TODO: understand how this works */
	m_priority = data & 0x03;

	/* bits 2-3 unknown (2 is used) */

	/* bit 4 controls NMI */
	m_nmi_mask = (data & 0x10) >> 4;

	/* bit 5 controls screen flip */
	flip_screen_set(data & 0x20);

	/* bits 6-7 unknown */
}

TILE_GET_INFO_MEMBER(bankp_state::get_bg_tile_info)
{
	int code = m_videoram2[tile_index] + 256 * (m_colorram2[tile_index] & 0x07);
	int color = m_colorram2[tile_index] >> 4;
	int flags = (m_colorram2[tile_index] & 0x08) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
	tileinfo.group = color;
}

TILE_GET_INFO_MEMBER(bankp_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index] + 256 * ((m_colorram[tile_index] & 3) >> 0);
	int color = m_colorram[tile_index] >> 3;
	int flags = (m_colorram[tile_index] & 0x04) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
	tileinfo.group = color;
}

void bankp_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bankp_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bankp_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(1), 0);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0);

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_priority));
}

UINT32 bankp_state::screen_update_bankp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
	{
		m_fg_tilemap->set_scrollx(0, -m_scroll_x);
		m_bg_tilemap->set_scrollx(0, 0);
	}
	else
	{
		m_fg_tilemap->set_scrollx(0, m_scroll_x);
		m_bg_tilemap->set_scrollx(0, 0);
	}


	// only one bit matters?
	switch (m_priority)
	{
	case 0: // combat hawk uses this
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;
	case 1:
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;
	case 2:
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;
	case 3:
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0); // just a guess
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;
	}
	return 0;
}
