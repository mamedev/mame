// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
// thanks-to:Enrique Sanchez
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/yiear.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Yie Ar Kung-Fu has one 32x8 palette PROM, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(yiear_state, yiear)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
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

		palette.set_pen_color(i, rgb_t(r,g,b));
		color_prom++;
	}
}

WRITE8_MEMBER(yiear_state::yiear_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(yiear_state::yiear_control_w)
{
	/* bit 0 flips screen */
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	/* bit 1 is NMI enable */
	m_yiear_nmi_enable = data & 0x02;

	/* bit 2 is IRQ enable */
	m_yiear_irq_enable = data & 0x04;

	/* bits 3 and 4 are coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);
}

TILE_GET_INFO_MEMBER(yiear_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int attr = m_videoram[offs];
	int code = m_videoram[offs + 1] | ((attr & 0x10) << 4);
//  int color = (attr & 0xf0) >> 4;
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, 0, flags);
}

void yiear_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(yiear_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void yiear_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = m_spriteram.bytes() - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram[offs];
		int code = spriteram_2[offs + 1] + 256 * (attr & 0x01);
		int color = 0;
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sy = 240 - spriteram[offs + 1];
		int sx = spriteram_2[offs];

		if (flip_screen())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		if (offs < 0x26)
		{
			sy++;   /* fix title screen & garbage at the bottom of the screen */
		}


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

UINT32 yiear_state::screen_update_yiear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
