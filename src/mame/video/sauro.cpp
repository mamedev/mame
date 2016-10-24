// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  sauro.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/sauro.h"

/* General */

void sauro_state::videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::sauro_videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::sauro_colorram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::scroll_bg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bg_tilemap->set_scrollx(0, data);
}

void sauro_state::get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x07) << 8);
	int color = ((m_colorram[tile_index] >> 4) & 0x0f) | m_palette_bank;
	int flags = m_colorram[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void sauro_state::get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_videoram2[tile_index] + ((m_colorram2[tile_index] & 0x07) << 8);
	int color = ((m_colorram2[tile_index] >> 4) & 0x0f) | m_palette_bank;
	int flags = m_colorram2[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

/* Sauro */

static const int scroll2_map[8] = {2, 1, 4, 3, 6, 5, 0, 7};
static const int scroll2_map_flip[8] = {0, 7, 2, 1, 4, 3, 6, 5};

void sauro_state::sauro_palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_palette_bank = (data & 0x03) << 4;
	machine().tilemap().mark_all_dirty();
}

void sauro_state::sauro_scroll_fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	const int *map = (flip_screen() ? scroll2_map_flip : scroll2_map);
	int scroll = (data & 0xf8) | map[data & 7];

	m_fg_tilemap->set_scrollx(0, scroll);
}

void sauro_state::video_start_sauro()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sauro_state::get_tile_info_bg),this), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sauro_state::get_tile_info_fg),this), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_palette_bank = 0;

	save_item(NAME(m_palette_bank));
}

void sauro_state::sauro_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram;
	int offs,code,sx,sy,color,flipx;
	int flipy = flip_screen();

	for (offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		sy = spriteram[offs];
		if (sy == 0xf8) continue;

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x03) << 8);
		sx = spriteram[offs+2];
		sy = 236 - sy;
		color = ((spriteram[offs+3] >> 4) & 0x0f) | m_palette_bank;

		// I'm not really sure how this bit works
		if (spriteram[offs+3] & 0x08)
		{
			if (sx > 0xc0)
			{
				// Sign extend
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  // The &0xff is not 100% percent correct
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				color,
				flipx, flipy,
				sx,sy,0);
	}
}

uint32_t sauro_state::screen_update_sauro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	sauro_draw_sprites(bitmap, cliprect);
	return 0;
}

/* Tricky Doc */

void sauro_state::video_start_trckydoc()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sauro_state::get_tile_info_bg),this), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);
}

void sauro_state::trckydoc_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram;
	int offs,code,sy,color,flipx,sx;
	int flipy = flip_screen();

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		sy = spriteram[offs];

		if(spriteram[offs+3] & 0x08)
		{
			/* needed by the elevator cable (2nd stage), balls bouncing (3rd stage) and maybe other things */
			sy += 6;
		}

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x01) << 8);

		sx = spriteram[offs+2]-2;
		color = (spriteram[offs+3] >> 4) & 0x0f;

		sy = 236 - sy;

		/* similar to sauro but different bit is used .. */
		if (spriteram[offs+3] & 0x02)
		{
			if (sx > 0xc0)
			{
				/* Sign extend */
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  /* The &0xff is not 100% percent correct */
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx, flipy,
				sx,sy,0);
	}
}

uint32_t sauro_state::screen_update_trckydoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	trckydoc_draw_sprites(bitmap, cliprect);
	return 0;
}
