// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/flstory.h"

TILE_GET_INFO_MEMBER(flstory_state::get_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	int flags = TILE_FLIPYX((attr & 0x18) >> 3);
	tileinfo.category = (attr & 0x20) >> 5;
	tileinfo.group = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			attr & 0x0f,
			flags);
}

TILE_GET_INFO_MEMBER(flstory_state::victnine_get_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = ((attr & 0x38) << 5) + code;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			attr & 0x07,
			flags);
}

TILE_GET_INFO_MEMBER(flstory_state::get_rumba_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	int col = (attr & 0x0f);

	tileinfo.category = (attr & 0x20) >> 5;
	tileinfo.group = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			col,
			0);
}

VIDEO_START_MEMBER(flstory_state,flstory)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flstory_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram.resize(m_palette->entries());
	m_paletteram_ext.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext, ENDIANNESS_LITTLE, 1);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_paletteram_ext));
}

VIDEO_START_MEMBER(flstory_state,rumba)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flstory_state::get_rumba_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram.resize(m_palette->entries());
	m_paletteram_ext.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext, ENDIANNESS_LITTLE, 1);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_paletteram_ext));
}

VIDEO_START_MEMBER(flstory_state,victnine)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flstory_state::victnine_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram.resize(m_palette->entries());
	m_paletteram_ext.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext, ENDIANNESS_LITTLE, 1);

	save_item(NAME(m_paletteram));
	save_item(NAME(m_paletteram_ext));
}

WRITE8_MEMBER(flstory_state::flstory_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(flstory_state::flstory_palette_w)
{
	if (offset & 0x100)
		m_palette->write_ext(space, (offset & 0xff) + (m_palette_bank << 8), data);
	else
		m_palette->write(space, (offset & 0xff) + (m_palette_bank << 8), data);
}

READ8_MEMBER(flstory_state::flstory_palette_r)
{
	if (offset & 0x100)
		return m_paletteram_ext[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_paletteram[(offset & 0xff) + (m_palette_bank << 8)];
}

WRITE8_MEMBER(flstory_state::flstory_gfxctrl_w)
{
	m_gfxctrl = data;

	flip_screen_set(~data & 0x01);
	if (m_char_bank != ((data & 0x10) >> 4))
	{
		m_char_bank = (data & 0x10) >> 4;
		m_bg_tilemap->mark_all_dirty();
	}
	m_palette_bank = (data & 0x20) >> 5;

//popmessage("%04x: gfxctrl = %02x\n", space.device().safe_pc(), data);

}

READ8_MEMBER(flstory_state::victnine_gfxctrl_r)
{
	return m_gfxctrl;
}

WRITE8_MEMBER(flstory_state::victnine_gfxctrl_w)
{
	m_gfxctrl = data;

	m_palette_bank = (data & 0x20) >> 5;

	if (data & 0x04)
		flip_screen_set(data & 0x01);

//popmessage("%04x: gfxctrl = %02x\n", space.device().safe_pc(), data);

}

WRITE8_MEMBER(flstory_state::flstory_scrlram_w)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}


void flstory_state::flstory_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int flip = flip_screen();

	for (int i = 0; i < 0x20; i++)
	{
		int pr = m_spriteram[m_spriteram.bytes() - 1 - i];
		int offs = (pr & 0x1f) * 4;

		if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x30) << 4);
			sx = m_spriteram[offs + 3];
			sy = m_spriteram[offs + 0];

			flipx = ((m_spriteram[offs + 1] & 0x40) >> 6);
			flipy = ((m_spriteram[offs + 1] & 0x80) >> 7);

			if (flip)
			{
				sx = (240 - sx) & 0xff ;
				sy = sy - 1 ;
				flipx = !flipx;
				flipy = !flipy;
			}
			else
				sy = 240 - sy - 1 ;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					m_spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,
						m_spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

UINT32 flstory_state::screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);
	flstory_draw_sprites(bitmap, cliprect, 0x00);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	flstory_draw_sprites(bitmap, cliprect, 0x80);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

void flstory_state::victnine_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int flip = flip_screen();

	for (int i = 0; i < 0x20; i++)
	{
		int pr = m_spriteram[m_spriteram.bytes() - 1 - i];
		int offs = (pr & 0x1f) * 4;

		//if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x20) << 3);
			sx = m_spriteram[offs + 3];
			sy = m_spriteram[offs + 0];

			flipx = ((m_spriteram[offs + 1] & 0x40) >> 6);
			flipy = ((m_spriteram[offs + 1] & 0x80) >> 7);

			if (flip)
			{
				sx = (240 - sx + 1) & 0xff ;
				sy = sy + 1 ;
				flipx = !flipx;
				flipy = !flipy;
			}
			else
				sy = 240 - sy + 1 ;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					m_spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,
						m_spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

UINT32 flstory_state::screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	victnine_draw_sprites(bitmap, cliprect);
	return 0;
}

UINT32 flstory_state::screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);
	victnine_draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	victnine_draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
