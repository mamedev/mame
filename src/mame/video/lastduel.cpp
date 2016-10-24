// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/lastduel.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

void lastduel_state::ld_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_scroll2[2 * tile_index] & 0x1fff;
	int color = m_scroll2[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(2,
			tile,color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

void lastduel_state::ld_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_scroll1[2 * tile_index] & 0x1fff;
	int color = m_scroll1[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x80) >> 7;
}

void lastduel_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_scroll2[tile_index] & 0x1fff;
	int color = m_scroll2[tile_index + 0x0800];
	SET_TILE_INFO_MEMBER(2,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

void lastduel_state::get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_scroll1[tile_index] & 0x1fff;
	int color = m_scroll1[tile_index + 0x0800];
	SET_TILE_INFO_MEMBER(3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x10) >> 4;
}

void lastduel_state::get_fix_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_vram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			tile & 0x7ff,
			tile>>12,
			(tile & 0x800) ? TILE_FLIPY : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void lastduel_state::video_start_lastduel()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::ld_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::ld_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::get_fix_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transmask(0, 0xffff, 0x0001);
	m_fg_tilemap->set_transmask(1, 0xf07f, 0x0f81);
	m_tx_tilemap->set_transparent_pen(3);

	m_sprite_flipy_mask = 0x40;
	m_sprite_pri_mask = 0x00;
	m_tilemap_priority = 0;
}

void lastduel_state::video_start_madgear()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,16,16,64,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::get_fg_tile_info),this),TILEMAP_SCAN_COLS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(lastduel_state::get_fix_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_fg_tilemap->set_transmask(0, 0xffff, 0x8000);
	m_fg_tilemap->set_transmask(1, 0x80ff, 0xff00);
	m_tx_tilemap->set_transparent_pen(3);
	m_bg_tilemap->set_transparent_pen(15);

	m_sprite_flipy_mask = 0x80;
	m_sprite_pri_mask = 0x10;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void lastduel_state::lastduel_flip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(data & 0x01);

		machine().bookkeeping().coin_lockout_w(0, ~data & 0x10);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x20);
		machine().bookkeeping().coin_counter_w(0, data & 0x40);
		machine().bookkeeping().coin_counter_w(1, data & 0x80);
	}
}

void lastduel_state::lastduel_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);
	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrolly(0, data); break;
		case 1: m_fg_tilemap->set_scrollx(0, data); break;
		case 2: m_bg_tilemap->set_scrolly(0, data); break;
		case 3: m_bg_tilemap->set_scrollx(0, data); break;
		case 7: m_tilemap_priority = data; break;
		default:
			logerror("Unmapped video write %d %04x\n", offset, data);
			break;
	}
}

void lastduel_state::lastduel_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll1[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void lastduel_state::lastduel_scroll2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void lastduel_state::lastduel_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void lastduel_state::madgear_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll1[offset]);
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void lastduel_state::madgear_scroll2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void lastduel_state::lastduel_palette_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int red, green, blue, bright;
	data = COMBINE_DATA(&m_paletteram[offset]);

	// Brightness parameter interpreted same way as CPS1
	bright = 0x10 + (data & 0x0f);

	red   = ((data >> 12) & 0x0f) * bright * 0x11 / 0x1f;
	green = ((data >> 8)  & 0x0f) * bright * 0x11 / 0x1f;
	blue  = ((data >> 4)  & 0x0f) * bright * 0x11 / 0x1f;

	m_palette->set_pen_color (offset, rgb_t(red, green, blue));
}

/***************************************************************************

  Display refresh

***************************************************************************/

void lastduel_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	uint16_t *buffered_spriteram16 = m_spriteram->buffer();
	int offs;

	if (!m_sprite_pri_mask)
		if (pri == 1)
			return; /* only low priority sprites in lastduel */

	for (offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		int attr, sy, sx, flipx, flipy, code, color;

		attr = buffered_spriteram16[offs + 1];
		if (m_sprite_pri_mask)   /* only madgear seems to have this */
		{
			if (pri == 1 && (attr & m_sprite_pri_mask))
				continue;
			if (pri == 0 && !(attr & m_sprite_pri_mask))
				continue;
		}

		code = buffered_spriteram16[offs];
		sx = buffered_spriteram16[offs + 3] & 0x1ff;
		sy = buffered_spriteram16[offs + 2] & 0x1ff;
		if (sy > 0x100)
			sy -= 0x200;

		flipx = attr & 0x20;
		flipy = attr & m_sprite_flipy_mask;  /* 0x40 for lastduel, 0x80 for madgear */
		color = attr & 0x0f;

		if (flip_screen())
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,15);
	}
}

uint32_t lastduel_state::screen_update_lastduel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	draw_sprites(bitmap, cliprect, 1);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t lastduel_state::screen_update_madgear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_tilemap_priority)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, 1);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		draw_sprites(bitmap, cliprect, 1);
	}
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
