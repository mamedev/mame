// license:BSD-3-Clause
// copyright-holders:Paul Hampson, Nicola Salmoria
#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/spdodgeb.h"


PALETTE_INIT_MEMBER(spdodgeb_state, spdodgeb)
{
	const UINT8 *color_prom = memregion("proms")->base();

	for (int i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(spdodgeb_state::background_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

TILE_GET_INFO_MEMBER(spdodgeb_state::get_bg_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	UINT8 attr = m_videoram[tile_index + 0x800];
	SET_TILE_INFO_MEMBER(0,
			code + ((attr & 0x1f) << 8),
			((attr & 0xe0) >> 5) + 8 * m_tile_palbank,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void spdodgeb_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spdodgeb_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(spdodgeb_state::background_scan),this),8,8,64,32);

	membank("mainbank")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_tile_palbank));
	save_item(NAME(m_sprite_palbank));
	save_item(NAME(m_lastscroll));
}


/***************************************************************************

  Memory handlers

***************************************************************************/


TIMER_DEVICE_CALLBACK_MEMBER(spdodgeb_state::interrupt)
{
	int scanline = param;

	if (scanline == 256)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_screen->update_partial(256);
	}
	else if ((scanline % 8) == 0)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		m_screen->update_partial(scanline+16); /* TODO: pretty off ... */
	}
}

WRITE8_MEMBER(spdodgeb_state::scrollx_lo_w)
{
	m_lastscroll = (m_lastscroll & 0x100) | data;
}

WRITE8_MEMBER(spdodgeb_state::ctrl_w)
{
	/* bit 0 = flip screen */
	flip_screen_set(data & 0x01);

	/* bit 1 = ROM bank switch */
	membank("mainbank")->set_entry((~data & 0x02) >> 1);

	/* bit 2 = scroll high bit */
	m_lastscroll = (m_lastscroll & 0x0ff) | ((data & 0x04) << 6);

	/* bit 3 = to mcu?? */

	/* bits 4-7 = palette bank select */
	if (m_tile_palbank != ((data & 0x30) >> 4))
	{
		m_tile_palbank = ((data & 0x30) >> 4);
		m_bg_tilemap->mark_all_dirty();
	}
	m_sprite_palbank = (data & 0xc0) >> 6;
}

WRITE8_MEMBER(spdodgeb_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

#define DRAW_SPRITE( order, sx, sy ) gfx->transpen(bitmap,\
					cliprect, \
					(which+order),color+ 8 * m_sprite_palbank,flipx,flipy,sx,sy,0);

void spdodgeb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

/*  240-SY   Z|F|CLR|WCH WHICH    SX
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (int i = 0;i < m_spriteram.bytes();i += 4)
	{
		int attr = m_spriteram[i+1];
		int which = m_spriteram[i+2]+((attr & 0x07)<<8);
		int sx = m_spriteram[i+3];
		int sy = 240 - m_spriteram[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;
		int cy;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		if (sx < -8) sx += 256; else if (sx > 248) sx -= 256;

		switch (size)
		{
			case 0: /* normal */
			if (sy < -8) sy += 256; else if (sy > 248) sy -= 256;
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			if (flip_screen()) { if (sy > 240) sy -= 256; } else { if (sy < 0) sy += 256; }
			cy = sy + dy;
			which &= ~1;
			DRAW_SPRITE(0,sx,cy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE


UINT32 spdodgeb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,m_lastscroll+5);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}
