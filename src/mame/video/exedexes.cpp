// license:BSD-3-Clause
// copyright-holders:Richard Davies
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/exedexes.h"
#include "screen.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Exed Exes has three 256x4 palette PROMs (one per gun), three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles)
  and one 256x4 sprite palette bank selector PROM.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void exedexes_state::exedexes_palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		const int r = pal4bit(color_prom[i + 0x000]);
		const int g = pal4bit(color_prom[i + 0x100]);
		const int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0xc0-0xcf
	for (int i = 0; i < 0x100; i++)
	{
		const u8 ctabentry = color_prom[i] | 0xc0;
		palette.set_pen_indirect(i, ctabentry);
	}

	// 32x32 tiles use colors 0-0x0f
	for (int i = 0x100; i < 0x200; i++)
	{
		const u8 ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}

	// 16x16 tiles use colors 0x40-0x4f
	for (int i = 0x200; i < 0x300; i++)
	{
		const u8 ctabentry = color_prom[i] | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites use colors 0x80-0xbf in four banks
	for (int i = 0x300; i < 0x400; i++)
	{
		const u8 ctabentry = color_prom[i] | (color_prom[i + 0x100] << 4) | 0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void exedexes_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void exedexes_state::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void exedexes_state::c804_w(u8 data)
{
	/* bits 0 and 1 are coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	machine().bookkeeping().coin_lockout_w(0, data & 0x04);
	machine().bookkeeping().coin_lockout_w(1, data & 0x08);

	/* bit 7 is text enable */
	m_chon = data & 0x80;

	/* other bits seem to be unused */
}

void exedexes_state::gfxctrl_w(u8 data)
{
	/* bit 4 is bg enable */
	m_sc2on = data & 0x10;

	/* bit 5 is fg enable */
	m_sc1on = data & 0x20;

	/* bit 6 is sprite enable */
	m_objon = data & 0x40;

	/* other bits seem to be unused */
}


TILE_GET_INFO_MEMBER(exedexes_state::get_bg_tile_info)
{
	const u8 attr = m_tilerom[tile_index];
	const u8 code = attr & 0x3f;
	const u8 color = m_tilerom[tile_index + (8 * 8)];
	const int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(exedexes_state::get_fg_tile_info)
{
	const u8 code = m_tilerom[tile_index];

	SET_TILE_INFO_MEMBER(2, code, 0, 0);
}

TILE_GET_INFO_MEMBER(exedexes_state::get_tx_tile_info)
{
	const u32 code = m_videoram[tile_index] + 2 * (m_colorram[tile_index] & 0x80);
	const u8 color = m_colorram[tile_index] & 0x3f;

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(exedexes_state::bg_tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return ((col * 32 & 0xe0) >> 5) + ((row * 32 & 0xe0) >> 2) + ((col * 32 & 0x3f00) >> 1) + 0x4000;
}

TILEMAP_MAPPER_MEMBER(exedexes_state::fg_tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return ((col * 16 & 0xf0) >> 4) + (row * 16 & 0xf0) + (col * 16 & 0x700) + ((row * 16 & 0x700) << 3);
}

void exedexes_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(exedexes_state::bg_tilemap_scan)), 32, 32, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(exedexes_state::fg_tilemap_scan)), 16, 16, 128, 128);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exedexes_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0xcf);
}

void exedexes_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 *buffered_spriteram = m_spriteram->buffer();

	if (!m_objon)
		return;

	for (int offs = 0; offs < m_spriteram->bytes(); offs += 32)
	{
		u32 primask = 0;
		if (buffered_spriteram[offs + 1] & 0x40)
			primask |= GFX_PMASK_2;

		const u32 code = buffered_spriteram[offs];
		const u32 color = buffered_spriteram[offs + 1] & 0x0f;
		const bool flipx = buffered_spriteram[offs + 1] & 0x10;
		const bool flipy = buffered_spriteram[offs + 1] & 0x20;
		const int sx = buffered_spriteram[offs + 3] - ((buffered_spriteram[offs + 1] & 0x80) << 1);
		const int sy = buffered_spriteram[offs + 2];

		m_gfxdecode->gfx(3)->prio_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,screen.priority(),primask,0);
	}
}

u32 exedexes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	if (m_sc2on)
	{
		m_bg_tilemap->set_scrollx(0, ((m_bg_scroll[1]) << 8) + m_bg_scroll[0]);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	}
	else
		bitmap.fill(0, cliprect);

	if (m_sc1on)
	{
		m_fg_tilemap->set_scrollx(0, ((m_nbg_yscroll[1]) << 8) + m_nbg_yscroll[0]);
		m_fg_tilemap->set_scrolly(0, ((m_nbg_xscroll[1]) << 8) + m_nbg_xscroll[0]);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	}

	draw_sprites(screen, bitmap, cliprect);

	if (m_chon)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
