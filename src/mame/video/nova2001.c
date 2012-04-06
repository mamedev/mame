#include "emu.h"
#include "includes/nova2001.h"


/*************************************
 *
 *  Palette handling
 *
 *************************************/

PALETTE_INIT( nova2001 )
{
	int i;

	/* Color #1 is used for palette animation.          */

	/* To handle this, color entries 0-15 are based on  */
	/* the primary 16 colors, while color entries 16-31 */
	/* are based on the secondary set.                  */

	/* The only difference between 0-15 and 16-31 is that */
	/* color #1 changes each time */

	for (i = 0; i < 0x200; ++i)
	{
		int entry;
		int intensity,r,g,b;

		if ((i & 0xf) == 1)
		{
			entry = ((i & 0xf0) >> 4) | ((i & 0x100) >> 4);
		}
		else
		{
			entry = ((i & 0x0f) >> 0) | ((i & 0x100) >> 4);
		}

		intensity = (color_prom[entry] >> 0) & 0x03;
		/* red component */
		r = (((color_prom[entry] >> 0) & 0x0c) | intensity) * 0x11;
		/* green component */
		g = (((color_prom[entry] >> 2) & 0x0c) | intensity) * 0x11;
		/* blue component */
		b = (((color_prom[entry] >> 4) & 0x0c) | intensity) * 0x11;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_MEMBER(nova2001_state::ninjakun_paletteram_w)
{
	int i;

	paletteram_BBGGRRII_w(space,offset,data);

	// expand the sprite palette to full length
	if (offset < 16)
	{
		paletteram_BBGGRRII_w(space, 0x200 + offset * 16 + 1, data);

		if (offset != 1)
		{
			for (i = 0; i < 16; i++)
			{
				paletteram_BBGGRRII_w(space, 0x200 + offset + i * 16, data);
			}
		}
	}
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( nova2001_get_bg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int code = state->m_bg_videoram[tile_index];
	int color = state->m_bg_videoram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( nova2001_get_fg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int attr = state->m_fg_videoram[tile_index + 0x400];
	int code = state->m_fg_videoram[tile_index];
	int color = attr & 0x0f;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo.category = (attr & 0x10) >> 4;
}

static TILE_GET_INFO( ninjakun_get_bg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int attr  = state->m_bg_videoram[tile_index+0x400];
	int code = state->m_bg_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( ninjakun_get_fg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int attr = state->m_fg_videoram[tile_index+0x400];
	int code = state->m_fg_videoram[tile_index] + ((attr & 0x20) << 3);
	int color = attr & 0x0f;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo.category = (attr & 0x10) >> 4;
}

static TILE_GET_INFO( pkunwar_get_bg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int attr = state->m_bg_videoram[tile_index + 0x400];
	int code = state->m_bg_videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0xf0) >> 4;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo.category = (attr & 0x08) >> 3;
}

static TILE_GET_INFO( raiders5_get_bg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int attr = state->m_bg_videoram[tile_index+0x400];
	int code = state->m_bg_videoram[tile_index] + ((attr & 0x01) << 8);
	int color = (attr & 0xf0) >> 4;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( raiders5_get_fg_tile_info )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	int code = state->m_fg_videoram[tile_index];
	int color = (state->m_fg_videoram[tile_index + 0x400] & 0xf0) >> 4;

	SET_TILE_INFO(1, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( nova2001 )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	state->m_bg_tilemap = tilemap_create(machine, nova2001_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, nova2001_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(0, -7);
}

VIDEO_START( pkunwar )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	state->m_bg_tilemap = tilemap_create(machine, pkunwar_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START( ninjakun )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	state->m_bg_tilemap = tilemap_create(machine, ninjakun_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, ninjakun_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(7, 0);
}

VIDEO_START( raiders5 )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	state->m_bg_tilemap = tilemap_create(machine, raiders5_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, raiders5_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(7, 0);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(nova2001_state::nova2001_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(nova2001_state::nova2001_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(nova2001_state::ninjakun_bg_videoram_w)
{
	int x = m_bg_tilemap->scrollx(0) >> 3;
	int y = m_bg_tilemap->scrolly(0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

READ8_MEMBER(nova2001_state::ninjakun_bg_videoram_r)
{
	int x = m_bg_tilemap->scrollx(0) >> 3;
	int y = m_bg_tilemap->scrolly(0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	return m_bg_videoram[offset];
}

WRITE8_MEMBER(nova2001_state::nova2001_scroll_x_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(nova2001_state::nova2001_scroll_y_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(nova2001_state::nova2001_flipscreen_w)
{
	// inverted
	flip_screen_set(machine(), ~data & 1);
}

WRITE8_MEMBER(nova2001_state::pkunwar_flipscreen_w)
{
	flip_screen_set(machine(), data & 1);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void nova2001_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	UINT8 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[0];
	int offs;

	for (offs = 0; offs < 0x800; offs += 32)
	{
		int attr = spriteram[offs+3];
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[offs+1] - ((attr & 0x40) << 2);	// high bit shown in schematics, not used by game
		int sy = spriteram[offs+2];
		int tile = spriteram[offs+0];
		int color = attr & 0x0f;

		if (attr & 0x80)	// disable bit shown in schematics, not used by game
		{
			continue;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

static void pkunwar_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	nova2001_state *state = machine.driver_data<nova2001_state>();
	UINT8 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[0];
	int offs;

	for (offs = 0; offs < 0x800; offs += 32)
	{
		int attr = spriteram[offs+3];
		int flipx = spriteram[offs+0] & 0x01;
		int flipy = spriteram[offs+0] & 0x02;
		int sx = spriteram[offs+1];
		int sy = spriteram[offs+2];
		int tile = ((spriteram[offs+0] & 0xfc) >> 2) + ((attr & 0x07) << 6);
		int color = (attr & 0xf0) >> 4;

		if (attr & 0x08)	// deducted by comparison, not used by game
		{
			continue;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);

		// there's no X MSB, so draw with wraparound (fixes title screen)
		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx - 256, sy, 0);
	}
}



SCREEN_UPDATE_IND16( nova2001 )
{
	nova2001_state *state = screen.machine().driver_data<nova2001_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	nova2001_draw_sprites(screen.machine(), bitmap, cliprect);

	// according to the schematics, fg category 0 should be drawn behind sprites,
	// but it doesn't look right that way
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 1, 0);

	return 0;
}

SCREEN_UPDATE_IND16( pkunwar )
{
	nova2001_state *state = screen.machine().driver_data<nova2001_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	pkunwar_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, 1, 0);

	return 0;
}

SCREEN_UPDATE_IND16( ninjakun )
{
	nova2001_state *state = screen.machine().driver_data<nova2001_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_fg_tilemap->draw(bitmap, cliprect, 1, 0);

	nova2001_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

SCREEN_UPDATE_IND16( raiders5 )
{
	nova2001_state *state = screen.machine().driver_data<nova2001_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	pkunwar_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
