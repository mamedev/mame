// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/trackfld.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Track 'n Field has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void trackfld_state::trackfld_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// sprites
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// characters
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(trackfld_state::trackfld_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(trackfld_state::trackfld_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE_LINE_MEMBER(trackfld_state::flipscreen_w)
{
	flip_screen_set(state);
	machine().tilemap().mark_all_dirty();
}

WRITE8_MEMBER(trackfld_state::atlantol_gfxbank_w)
{
	if (data & 1)
	{
		/* male / female sprites switch */
		if ((m_old_gfx_bank == 1 && (data & 1) == 1) || (m_old_gfx_bank == 0 && (data & 1) == 1))
			m_sprite_bank2 = 0x200;
		else
			m_sprite_bank2 = 0;

		m_sprite_bank1 = 0;
		m_old_gfx_bank = data & 1;
	}
	else
	{
		/* male / female sprites switch */
		if ((m_old_gfx_bank == 0 && (data & 1) == 0) || (m_old_gfx_bank == 1 && (data & 1) == 0))
			m_sprite_bank2 = 0;
		else
			m_sprite_bank2 = 0x200;

		m_sprite_bank1 = 0;
		m_old_gfx_bank = data & 1;
	}

	if ((data & 3) == 3)
	{
		if (m_sprite_bank2)
			m_sprite_bank1 = 0x500;
		else
			m_sprite_bank1 = 0x300;
	}
	else if ((data & 3) == 2)
	{
		if (m_sprite_bank2)
			m_sprite_bank1 = 0x300;
		else
			m_sprite_bank1 = 0x100;
	}

	if (m_bg_bank != (data & 0x8))
	{
		m_bg_bank = data & 0x8;
		m_bg_tilemap->mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(trackfld_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 4 * (attr & 0xc0);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	if (m_bg_bank)
		code |= 0x400;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

VIDEO_START_MEMBER(trackfld_state,trackfld)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(trackfld_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_scroll_rows(32);
	m_sprites_gfx_banked = 0;
}


VIDEO_START_MEMBER(trackfld_state,atlantol)
{
	VIDEO_START_CALL_MEMBER( trackfld );
	m_sprites_gfx_banked = 1;
}



void trackfld_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = m_spriteram;
	uint8_t *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = m_spriteram.bytes() - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram_2[offs];
		int code = spriteram[offs + 1];
		int color = attr & 0x0f;
		if (!m_sprites_gfx_banked)
			if (attr&1) code|=0x100; // extra tile# bit for the yiear conversion, trackfld doesn't have this many sprites so it will just get masked
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs] - 1;
		int sy = 240 - spriteram_2[offs + 1];

		if (flip_screen())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustement must be done AFTER handling flip screen, thus */
		/* proving that this is a hardware related "feature" */
		sy += 1;

		// to fix the title screen in yieartf it would have to be like this, the same as yiear.c, this should be verified on the hw
		//
		//if (offs < 0x26)
		//{
		//  sy++;   /* fix title screen & garbage at the bottom of the screen */
		//}



			m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
			code + m_sprite_bank1 + m_sprite_bank2, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));

		/* redraw with wraparound */

			m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
			code + m_sprite_bank1 + m_sprite_bank2, color,
			flipx, flipy,
			sx - 256, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
	}
}



uint32_t trackfld_state::screen_update_trackfld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row, scrollx;

	for (row = 0; row < 32; row++)
	{
		scrollx = m_scroll[row] + 256 * (m_scroll2[row] & 0x01);
		if (flip_screen()) scrollx = -scrollx;
		m_bg_tilemap->set_scrollx(row, scrollx);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
