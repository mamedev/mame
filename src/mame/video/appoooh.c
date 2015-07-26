// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/appoooh.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Palette information of appoooh is not known.

  The palette decoder of Bank Panic was used for this driver.
  Because these hardware is similar.

***************************************************************************/

PALETTE_INIT_MEMBER(appoooh_state,appoooh)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		UINT8 pen;
		int bit0, bit1, bit2, r, g, b;

		if (i < 0x100)
			/* charset #1 */
			pen = (color_prom[0x020 + (i - 0x000)] & 0x0f) | 0x00;
		else
			/* charset #2 */
			pen = (color_prom[0x120 + (i - 0x100)] & 0x0f) | 0x10;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

PALETTE_INIT_MEMBER(appoooh_state,robowres)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 pen = color_prom[0x020 + i] & 0x0f;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(appoooh_state::get_fg_tile_info)
{
	int code = m_fg_videoram[tile_index] + 256 * ((m_fg_colorram[tile_index] >> 5) & 7);

	SET_TILE_INFO_MEMBER(0,
			code,
			m_fg_colorram[tile_index] & 0x0f,
			(m_fg_colorram[tile_index] & 0x10 ) ? TILEMAP_FLIPX : 0
	);
}

TILE_GET_INFO_MEMBER(appoooh_state::get_bg_tile_info)
{
	int code = m_bg_videoram[tile_index] + 256 * ((m_bg_colorram[tile_index] >> 5) & 7);

	SET_TILE_INFO_MEMBER(1,
			code,
			m_bg_colorram[tile_index] & 0x0f,
			(m_bg_colorram[tile_index] & 0x10 ) ? TILEMAP_FLIPX : 0
	);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(appoooh_state,appoooh)
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(appoooh_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(appoooh_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldy(8, 8);
	m_bg_tilemap->set_scrolldy(8, 8);

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_priority));
}

WRITE8_MEMBER(appoooh_state::scroll_w)
{
	m_scroll_x = data;
}


WRITE8_MEMBER(appoooh_state::fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(appoooh_state::fg_colorram_w)
{
	m_fg_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(appoooh_state::bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(appoooh_state::bg_colorram_w)
{
	m_bg_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(appoooh_state::out_w)
{
	/* bit 0 controls NMI */
	m_nmi_mask = data & 1;

	/* bit 1 flip screen */
	flip_screen_set(data & 0x02);

	/* bits 2-3 unknown */

	/* bits 4-5 are playfield/sprite priority */
	/* TODO: understand how this works, currently the only thing I do is draw */
	/* the front layer behind sprites when priority == 0, and invert the sprite */
	/* order when priority == 1 */
	m_priority = (data & 0x30) >> 4;

	/* bit 6 ROM bank select */
	{
		membank("bank1")->set_entry((data&0x40) ? 1 : 0);
	}

	/* bit 7 unknown (used) */
}

void appoooh_state::appoooh_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, UINT8 *sprite )
{
	int offs;
	int flipy = flip_screen();

	for (offs = 0x20 - 4; offs >= 0; offs -= 4)
	{
		int sy    = 240 - sprite[offs + 0];
		int code  = (sprite[offs + 1] >> 2) + ((sprite[offs + 2] >> 5) & 0x07) * 0x40;
		int color = sprite[offs + 2] & 0x0f;    /* TODO: bit 4 toggles continuously, what is it? */
		int sx    = sprite[offs + 3];
		int flipx = sprite[offs + 1] & 0x01;

		if(sx >= 248)
			sx -= 256;

		if (flipy)
		{
			sx = 239 - sx;
			sy = 239 - sy;
			flipx = !flipx;
		}

				gfx->transpen(dest_bmp,cliprect,
				code,
				color,
				flipx,flipy,
				sx, sy, 0);
	}
}

void appoooh_state::robowres_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, UINT8 *sprite )
{
	int offs;
	int flipy = flip_screen();

	for (offs = 0x20 - 4; offs >= 0; offs -= 4)
	{
		int sy    = 240 - sprite[offs + 0];
		int code  = 0x200 + (sprite[offs + 1] >> 2) + ((sprite[offs + 2] >> 5) & 0x07) * 0x40;
		int color = sprite[offs + 2] & 0x0f;    /* TODO: bit 4 toggles continuously, what is it? */
		int sx    = sprite[offs + 3];
		int flipx = sprite[offs + 1] & 0x01;

		if(sx >= 248)
			sx -= 256;

		if (flipy)
		{
			sx = 239 - sx;
			sy = 239 - sy;
			flipx = !flipx;
		}

				gfx->transpen(dest_bmp,cliprect,
				code,
				color,
				flipx,flipy,
				sx, sy, 0);
	}
}


UINT32 appoooh_state::screen_update_appoooh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_priority == 0)    /* fg behind sprites */
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw sprites */
	if (m_priority == 1)
	{
		/* sprite set #1 */
		appoooh_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), m_spriteram);
		/* sprite set #2 */
		appoooh_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(3), m_spriteram_2);
	}
	else
	{
		/* sprite set #2 */
		appoooh_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(3), m_spriteram_2);
		/* sprite set #1 */
		appoooh_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), m_spriteram);
	}

	if (m_priority != 0)    /* fg in front of sprites */
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

UINT32 appoooh_state::screen_update_robowres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_priority == 0)    /* fg behind sprites */
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw sprites */
	if (m_priority == 1)
	{
		/* sprite set #1 */
		robowres_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), m_spriteram);
		/* sprite set #2 */
		robowres_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(3), m_spriteram_2);
	}
	else
	{
		/* sprite set #2 */
		robowres_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(3), m_spriteram_2);
		/* sprite set #1 */
		robowres_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), m_spriteram);
	}

	if (m_priority != 0)    /* fg in front of sprites */
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
