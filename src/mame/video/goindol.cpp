// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************
  Goindol

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/goindol.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(goindol_state::get_fg_tile_info)
{
	int code = m_fg_videoram[2 * tile_index + 1];
	int attr = m_fg_videoram[2 * tile_index];
	SET_TILE_INFO_MEMBER(0,
			code | ((attr & 0x7) << 8) | (m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}

TILE_GET_INFO_MEMBER(goindol_state::get_bg_tile_info)
{
	int code = m_bg_videoram[2 * tile_index + 1];
	int attr = m_bg_videoram[2 * tile_index];
	SET_TILE_INFO_MEMBER(1,
			code | ((attr & 0x7) << 8) | (m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void goindol_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goindol_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goindol_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(goindol_state::goindol_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(goindol_state::goindol_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void goindol_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, UINT8 *sprite_ram )
{
	int offs, sx, sy, tile, palette;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		sx = sprite_ram[offs];
		sy = 240 - sprite_ram[offs + 1];

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		if ((sprite_ram[offs + 1] >> 3) && (sx < 248))
		{
			tile = ((sprite_ram[offs + 3]) + ((sprite_ram[offs + 2] & 7) << 8));
			tile += tile;
			palette = sprite_ram[offs + 2] >> 3;


						m_gfxdecode->gfx(gfxbank)->transpen(bitmap,cliprect,
						tile,
						palette,
						flip_screen(),flip_screen(),
						sx,sy, 0);

						m_gfxdecode->gfx(gfxbank)->transpen(bitmap,cliprect,
						tile+1,
						palette,
						flip_screen(),flip_screen(),
						sx,sy + (flip_screen() ? -8 : 8), 0);
		}
	}
}

UINT32 goindol_state::screen_update_goindol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->set_scrollx(0, *m_fg_scrollx);
	m_fg_tilemap->set_scrolly(0, *m_fg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1, m_spriteram);
	draw_sprites(bitmap, cliprect, 0, m_spriteram2);
	return 0;
}
