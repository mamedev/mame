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

TILE_GET_INFO_MEMBER(lastduel_state::ld_get_bg_tile_info)
{
	int const tile = m_vram[1][2 * tile_index] & 0x1fff;
	int const color = m_vram[1][2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(2,
			tile,color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

TILE_GET_INFO_MEMBER(lastduel_state::ld_get_fg_tile_info)
{
	int const tile = m_vram[0][2 * tile_index] & 0x1fff;
	int const color = m_vram[0][2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x80) >> 7;
}

TILE_GET_INFO_MEMBER(lastduel_state::get_bg_tile_info)
{
	int const tile = m_vram[1][tile_index] & 0x1fff;
	int const color = m_vram[1][tile_index + 0x0800];
	SET_TILE_INFO_MEMBER(2,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

TILE_GET_INFO_MEMBER(lastduel_state::get_fg_tile_info)
{
	int const tile = m_vram[0][tile_index] & 0x1fff;
	int const color = m_vram[0][tile_index + 0x0800];
	SET_TILE_INFO_MEMBER(3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x10) >> 4;
}

TILE_GET_INFO_MEMBER(lastduel_state::get_fix_info)
{
	int const tile = m_txram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			tile & 0x7ff,
			tile>>12,
			(tile & 0x800) ? TILE_FLIPY : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(lastduel_state,lastduel)
{
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::ld_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::ld_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::get_fix_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transmask(0, 0xffff, 0x0001);
	m_tilemap[0]->set_transmask(1, 0xf07f, 0x0f81);
	m_tx_tilemap->set_transparent_pen(3);

	m_sprite_flipy_mask = 0x40;
	m_sprite_pri_mask = 0x00;
	m_tilemap_priority = 0;
}

VIDEO_START_MEMBER(lastduel_state,madgear)
{
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastduel_state::get_fix_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transmask(0, 0xffff, 0x8000);
	m_tilemap[0]->set_transmask(1, 0x80ff, 0xff00);
	m_tx_tilemap->set_transparent_pen(3);
	m_tilemap[1]->set_transparent_pen(15);

	m_sprite_flipy_mask = 0x80;
	m_sprite_pri_mask = 0x10;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(lastduel_state::flip_w)
{
	flip_screen_set(data & 0x01);

	machine().bookkeeping().coin_lockout_w(0, ~data & 0x10);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x20);
	machine().bookkeeping().coin_counter_w(0, data & 0x40);
	machine().bookkeeping().coin_counter_w(1, data & 0x80);
}

WRITE16_MEMBER(lastduel_state::vctrl_w)
{
	data = COMBINE_DATA(&m_vctrl[offset]);
	switch (offset)
	{
		case 0: m_tilemap[0]->set_scrolly(0, data); break;
		case 1: m_tilemap[0]->set_scrollx(0, data); break;
		case 2: m_tilemap[1]->set_scrolly(0, data); break;
		case 3: m_tilemap[1]->set_scrollx(0, data); break;
		case 7: m_tilemap_priority = data; break;
		default:
			logerror("Unmapped video write %d %04x\n", offset, data);
			break;
	}
}

WRITE16_MEMBER(lastduel_state::txram_w)
{
	COMBINE_DATA(&m_txram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

rgb_t lastduel_state::lastduel_RRRRGGGGBBBBIIII(uint32_t raw)
{
	// Brightness parameter interpreted same way as CPS1
	int const bright = 0x10 + (raw & 0x0f);

	int const red   = ((raw >> 12) & 0x0f) * bright * 0x11 / 0x1f;
	int const green = ((raw >> 8)  & 0x0f) * bright * 0x11 / 0x1f;
	int const blue  = ((raw >> 4)  & 0x0f) * bright * 0x11 / 0x1f;

	return rgb_t(red, green, blue);
}

/***************************************************************************

  Display refresh

***************************************************************************/

void lastduel_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	const uint16_t *buffered_spriteram16 = m_spriteram->buffer();
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
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	draw_sprites(bitmap, cliprect, 1);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t lastduel_state::screen_update_madgear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_tilemap_priority)
	{
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, 1);
	}
	else
	{
		m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
		draw_sprites(bitmap, cliprect, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		draw_sprites(bitmap, cliprect, 1);
	}
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
