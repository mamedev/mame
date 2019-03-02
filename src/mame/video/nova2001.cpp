// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Frank Palazzolo, Alex Pasadyn, David Haywood, Phil Stroffolino, Uki
#include "emu.h"
#include "includes/nova2001.h"


/*************************************
 *
 *  Palette handling
 *
 *************************************/

void nova2001_state::nova2001_palette(palette_device &palette) const
{
	u8 const *const color_prom = memregion("proms")->base();

	/*
	  Color #1 is used for palette animation.
	  To handle this, color entries 0-15 are based on
	  the primary 16 colors, while color entries 16-31
	  are based on the secondary set.

	  The only difference between 0-15 and 16-31 is that
	  color #1 changes each time
	*/
	for (int i = 0; i < 0x200; ++i)
	{
		int entry;
		if ((i & 0xf) == 1)
			entry = ((i & 0xf0) >> 4) | ((i & 0x100) >> 4);
		else
			entry = ((i & 0x0f) >> 0) | ((i & 0x100) >> 4);

		palette.set_pen_color(i, BBGGRRII(color_prom[entry]));
	}
}

rgb_t nova2001_state::BBGGRRII(u32 raw)
{
	u8 const i = raw & 3;
	u8 const r = ((raw >> 0) & 0x0c) | i;
	u8 const g = ((raw >> 2) & 0x0c) | i;
	u8 const b = ((raw >> 4) & 0x0c) | i;

	return rgb_t(r | (r << 4), g | (g << 4), b | (b << 4));
}

WRITE8_MEMBER(nova2001_state::paletteram_w)
{
	m_palette->write8(space, offset, data);

	// expand the sprite palette to full length
	if (offset < 16)
	{
		m_palette->write8(space, 0x200 + offset * 16 + 1, data);

		if (offset != 1)
		{
			for (int i = 0; i < 16; i++)
			{
				m_palette->write8(space, 0x200 + offset + i * 16, data);
			}
		}
	}
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

TILE_GET_INFO_MEMBER(nova2001_state::nova2001_get_bg_tile_info)
{
	int const code  = m_bg_videoram[tile_index];
	int const color = m_bg_videoram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(nova2001_state::nova2001_get_fg_tile_info)
{
	int const attr  = m_fg_videoram[tile_index + 0x400];
	int const code  = m_fg_videoram[tile_index];
	int const color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(1, code, color, 0);

	tileinfo.category = (attr & 0x10) >> 4;
}

TILE_GET_INFO_MEMBER(nova2001_state::ninjakun_get_bg_tile_info)
{
	int const attr  = m_bg_videoram[tile_index + 0x400];
	int const code  = m_bg_videoram[tile_index] + ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(nova2001_state::ninjakun_get_fg_tile_info)
{
	int const attr  = m_fg_videoram[tile_index + 0x400];
	int const code  = m_fg_videoram[tile_index] + ((attr & 0x20) << 3);
	int const color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(1, code, color, 0);

	tileinfo.category = (attr & 0x10) >> 4;
}

TILE_GET_INFO_MEMBER(nova2001_state::pkunwar_get_bg_tile_info)
{
	int const attr  = m_bg_videoram[tile_index + 0x400];
	int const code  = m_bg_videoram[tile_index] + ((attr & 0x07) << 8);
	int const color = (attr & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, code, color, 0);

	tileinfo.category = (attr & 0x08) >> 3;
}

TILE_GET_INFO_MEMBER(nova2001_state::raiders5_get_bg_tile_info)
{
	int const attr  = m_bg_videoram[tile_index + 0x400];
	int const code  = m_bg_videoram[tile_index] + ((attr & 0x01) << 8);
	int const color = (attr & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(nova2001_state::raiders5_get_fg_tile_info)
{
	int const code  =  m_fg_videoram[tile_index];
	int const color = (m_fg_videoram[tile_index + 0x400] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(nova2001_state,nova2001)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::nova2001_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::nova2001_get_fg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(0, -7);
}

VIDEO_START_MEMBER(nova2001_state,pkunwar)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::pkunwar_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(nova2001_state,ninjakun)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::ninjakun_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::ninjakun_get_fg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(7, 0);
}

VIDEO_START_MEMBER(nova2001_state,raiders5)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::raiders5_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(nova2001_state::raiders5_get_fg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(7, 0);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(nova2001_state::fg_videoram_w)
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
	int const x = m_bg_tilemap->scrollx(0) >> 3;
	int const y = m_bg_tilemap->scrolly(0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

READ8_MEMBER(nova2001_state::ninjakun_bg_videoram_r)
{
	int const x = m_bg_tilemap->scrollx(0) >> 3;
	int const y = m_bg_tilemap->scrolly(0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	return m_bg_videoram[offset];
}

WRITE8_MEMBER(nova2001_state::scroll_x_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(nova2001_state::scroll_y_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(nova2001_state::nova2001_flipscreen_w)
{
	// inverted
	flip_screen_set(~data & 1);
}

WRITE8_MEMBER(nova2001_state::pkunwar_flipscreen_w)
{
	flip_screen_set(data & 1);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void nova2001_state::nova2001_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0; offs < 0x800; offs += 32)
	{
		int const attr = m_spriteram[offs+3];
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = m_spriteram[offs+1] - ((attr & 0x40) << 2);  // high bit shown in schematics, not used by game
		int sy = m_spriteram[offs+2];
		int const tile = m_spriteram[offs+0];
		int const color = attr & 0x0f;

		if (attr & 0x80)    // disable bit shown in schematics, not used by game
		{
			continue;
		}

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

void nova2001_state::pkunwar_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0; offs < 0x800; offs += 32)
	{
		int const attr = m_spriteram[offs+3];
		int flipx = m_spriteram[offs+0] & 0x01;
		int flipy = m_spriteram[offs+0] & 0x02;
		int sx = m_spriteram[offs+1];
		int sy = m_spriteram[offs+2];
		int const tile = ((m_spriteram[offs+0] & 0xfc) >> 2) + ((attr & 0x07) << 6);
		int const color = (attr & 0xf0) >> 4;

		if (attr & 0x08)    // deducted by comparison, not used by game
		{
			continue;
		}

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			tile,
			color,
			flipx, flipy,
			sx, sy, 0);

		// there's no X MSB, so draw with wraparound (fixes title screen)
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			tile,
			color,
			flipx, flipy,
			sx - 256, sy, 0);
	}
}



u32 nova2001_state::screen_update_nova2001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	nova2001_draw_sprites(bitmap, cliprect);

	// according to the schematics, fg category 0 should be drawn behind sprites,
	// but it doesn't look right that way
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}

u32 nova2001_state::screen_update_pkunwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	pkunwar_draw_sprites(bitmap, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}

u32 nova2001_state::screen_update_ninjakun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	nova2001_draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

u32 nova2001_state::screen_update_raiders5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	pkunwar_draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
