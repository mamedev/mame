// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "fastfred.h"



/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void fastfred_state::fastfred_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 1000, 470, 220, 100 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i | 0x000], 0);
		bit1 = BIT(color_prom[i | 0x000], 1);
		bit2 = BIT(color_prom[i | 0x000], 2);
		bit3 = BIT(color_prom[i | 0x000], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i | 0x100], 0);
		bit1 = BIT(color_prom[i | 0x100], 1);
		bit2 = BIT(color_prom[i | 0x100], 2);
		bit3 = BIT(color_prom[i | 0x100], 3);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i | 0x200], 0);
		bit1 = BIT(color_prom[i | 0x200], 1);
		bit2 = BIT(color_prom[i | 0x200], 2);
		bit3 = BIT(color_prom[i | 0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// characters and sprites use the same palette
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(fastfred_state::get_tile_info)
{
	uint8_t x = tile_index & 0x1f;

	uint16_t code = m_charbank | m_videoram[tile_index];
	uint8_t color = m_colorbank | (m_attributesram[2 * x + 1] & 0x07);

	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(fastfred_state,fastfred)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fastfred_state::get_tile_info)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void fastfred_state::fastfred_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void fastfred_state::fastfred_attributes_w(offs_t offset, uint8_t data)
{
	if (m_attributesram[offset] != data)
	{
		if (offset & 0x01)
		{
			/* color change */
			int i;

			for (i = offset / 2; i < 0x0400; i += 32)
				m_bg_tilemap->mark_tile_dirty(i);
		}
		else
		{
			/* coloumn scroll */
			m_bg_tilemap->set_scrolly(offset / 2, data);
		}

		m_attributesram[offset] = data;
	}
}


WRITE_LINE_MEMBER(fastfred_state::charbank1_w)
{
	uint16_t new_data = (m_charbank & 0x0200) | (state << 8);

	if (new_data != m_charbank)
	{
		m_bg_tilemap->mark_all_dirty();

		m_charbank = new_data;
	}
}

WRITE_LINE_MEMBER(fastfred_state::charbank2_w)
{
	uint16_t new_data = (m_charbank & 0x0100) | (state << 9);

	if (new_data != m_charbank)
	{
		m_bg_tilemap->mark_all_dirty();

		m_charbank = new_data;
	}
}


WRITE_LINE_MEMBER(fastfred_state::colorbank1_w)
{
	uint8_t new_data = (m_colorbank & 0x10) | (state << 3);

	if (new_data != m_colorbank)
	{
		m_bg_tilemap->mark_all_dirty();

		m_colorbank = new_data;
	}
}

WRITE_LINE_MEMBER(fastfred_state::colorbank2_w)
{
	uint8_t new_data = (m_colorbank & 0x08) | (state << 4);

	if (new_data != m_colorbank)
	{
		m_bg_tilemap->mark_all_dirty();

		m_colorbank = new_data;
	}
}



WRITE_LINE_MEMBER(fastfred_state::flip_screen_x_w)
{
	flip_screen_x_set(state);

	m_bg_tilemap->set_flip((flip_screen_x() ? TILEMAP_FLIPX : 0) | (flip_screen_y() ? TILEMAP_FLIPY : 0));
}

WRITE_LINE_MEMBER(fastfred_state::flip_screen_y_w)
{
	flip_screen_y_set(state);

	m_bg_tilemap->set_flip((flip_screen_x() ? TILEMAP_FLIPX : 0) | (flip_screen_y() ? TILEMAP_FLIPY : 0));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void fastfred_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle spritevisiblearea(2*8, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-1, 2*8, 30*8-1);
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		uint8_t code,sx,sy;
		int flipx,flipy;

		sx = m_spriteram[offs + 3];
		sy = 240 - m_spriteram[offs];

		if (m_hardware_type == 3)
		{
			// Imago
			code  = (m_spriteram[offs + 1]) & 0x3f;
			flipx = 0;
			flipy = 0;
		}
		else if (m_hardware_type == 2)
		{
			// Boggy 84
			code  =  m_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy =  m_spriteram[offs + 1] & 0x80;
		}
		else if (m_hardware_type == 1)
		{
			// Fly-Boy/Fast Freddie/Red Robin
			code  =  m_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy = ~m_spriteram[offs + 1] & 0x80;
		}
		else
		{
			// Jump Coaster
			code  = (m_spriteram[offs + 1] & 0x3f) | 0x40;
			flipx = ~m_spriteram[offs + 1] & 0x40;
			flipy =  m_spriteram[offs + 1] & 0x80;
		}


		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,flip_screen_x() ? spritevisibleareaflipx : spritevisiblearea,
				code,
				m_colorbank | (m_spriteram[offs + 2] & 0x07),
				flipx,flipy,
				sx,sy,0);
	}
}


uint32_t fastfred_state::screen_update_fastfred(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(*m_background_color, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect);

	return 0;
}


TILE_GET_INFO_MEMBER(fastfred_state::imago_get_tile_info_bg)
{
	uint8_t x = tile_index & 0x1f;

	uint16_t code = m_charbank * 0x100 + m_videoram[tile_index];
	uint8_t color = m_colorbank | (m_attributesram[2 * x + 1] & 0x07);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(fastfred_state::imago_get_tile_info_fg)
{
	int code = m_imago_fg_videoram[tile_index];
	tileinfo.set(2, code, 2, 0);
}

TILE_GET_INFO_MEMBER(fastfred_state::imago_get_tile_info_web)
{
	tileinfo.set(3, tile_index & 0x1ff, 0, 0);
}

void fastfred_state::imago_fg_videoram_w(offs_t offset, uint8_t data)
{
	m_imago_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE_LINE_MEMBER(fastfred_state::imago_charbank_w)
{
	if (m_charbank != state)
	{
		m_charbank = state;
		m_bg_tilemap->mark_all_dirty();
	}
}

VIDEO_START_MEMBER(fastfred_state,imago)
{
	m_web_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fastfred_state::imago_get_tile_info_web)),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_bg_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fastfred_state::imago_get_tile_info_bg)), TILEMAP_SCAN_ROWS,8,8,32,32);
	m_fg_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fastfred_state::imago_get_tile_info_fg)), TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_flipscreen_x = 0;
	m_flipscreen_y = 0;

	/* the game has a galaxian starfield */
	galaxold_init_stars(256);
	m_stars_on = 1;
	m_stars_scrollpos = 0;

	/* web colors */
	m_palette->set_pen_color(256+64+0,rgb_t(0x50,0x00,0x00));
	m_palette->set_pen_color(256+64+1,rgb_t(0x00,0x00,0x00));

	save_item(NAME(m_imago_sprites));
	save_item(NAME(m_imago_sprites_address));
	save_item(NAME(m_imago_sprites_bank));

	// galaxold starfield related save states. Something's still missing here.
	save_item(NAME(m_stars_on));
	save_item(NAME(m_stars_blink_state));
	save_item(NAME(m_timer_adjusted));
	save_item(NAME(m_stars_scrollpos));

	for (int i = 0; i < STAR_COUNT; i++)
	{
		save_item(NAME(m_stars[i].x), i);
		save_item(NAME(m_stars[i].y), i);
		save_item(NAME(m_stars[i].color), i);
	}
}

uint32_t fastfred_state::screen_update_imago(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_web_tilemap->draw(screen, bitmap, cliprect, 0,0);
	galaxold_draw_stars(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
