// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina,David Haywood
/* Free Kick Video Hardware */

#include "emu.h"
#include "includes/freekick.h"


TILE_GET_INFO_MEMBER(freekick_state::get_freek_tile_info)
{
	int tileno, palno;

	tileno = m_videoram[tile_index] + ((m_videoram[tile_index + 0x400] & 0xe0) << 3);
	palno = m_videoram[tile_index + 0x400] & 0x1f;
	SET_TILE_INFO_MEMBER(0, tileno, palno, 0);
}


void freekick_state::video_start()
{
	m_freek_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(freekick_state::get_freek_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


WRITE8_MEMBER(freekick_state::freek_videoram_w)
{
	m_videoram[offset] = data;
	m_freek_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void freekick_state::gigas_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 2];
		int code = m_spriteram[offs + 0] | ((m_spriteram[offs + 1] & 0x20) << 3);

		int flipx = 0;
		int flipy = 0;
		int color = m_spriteram[offs + 1] & 0x1f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}


void freekick_state::pbillrd_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 2];
		int code = m_spriteram[offs + 0];

		int flipx = 0;//m_spriteram[offs + 0] & 0x80; //?? unused ?
		int flipy = 0;//m_spriteram[offs + 0] & 0x40;
		int color = m_spriteram[offs + 1] & 0x0f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}



void freekick_state::freekick_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xpos = m_spriteram[offs + 3];
		int ypos = m_spriteram[offs + 0];
		int code = m_spriteram[offs + 1] + ((m_spriteram[offs + 2] & 0x20) << 3);

		int flipx = m_spriteram[offs + 2] & 0x80;    //?? unused ?
		int flipy = m_spriteram[offs + 2] & 0x40;
		int color = m_spriteram[offs + 2] & 0x1f;

		if (flip_screen_x())
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,248-ypos,0);
	}
}

UINT32 freekick_state::screen_update_gigas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_freek_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	gigas_draw_sprites(bitmap, cliprect);
	return 0;
}

UINT32 freekick_state::screen_update_pbillrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_freek_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	pbillrd_draw_sprites(bitmap, cliprect);
	return 0;
}

UINT32 freekick_state::screen_update_freekick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_freek_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	freekick_draw_sprites(bitmap, cliprect);
	return 0;
}
