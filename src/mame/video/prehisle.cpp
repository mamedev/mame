// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Prehistoric Isle video routines

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/prehisle.h"

#include "screen.h"


WRITE16_MEMBER(prehisle_state::fg_vram_w)
{
	COMBINE_DATA(&m_fg_vram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(prehisle_state::tx_vram_w)
{
	COMBINE_DATA(&m_tx_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void prehisle_state::fg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrolly);
	m_fg_tilemap->set_scrolly(0, m_fg_scrolly);
}

void prehisle_state::fg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrollx);
	m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
}

void prehisle_state::bg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrolly);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
}

void prehisle_state::bg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrollx);
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
}


/* tile layout
0  xxxx....  color
0  ....x...  flip x
0  .....xxx  gfx code high bits
1  xxxxxxxx  gfx code low bits
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_bg_tile_info)
{
	int const offs = tile_index * 2;
	int const attr = m_tilemap_rom[offs + 1] + (m_tilemap_rom[offs] << 8);
	int const code = attr & 0x7ff;
	int const color = attr >> 12;
	int const flags = (attr & 0x800) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

/* tile layout
0  xxxx.... ........  color
0  ....x... ........  flip y
0  .....xxx xxxxxxxx  gfx code
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_fg_tile_info)
{
	int const attr = m_fg_vram[tile_index];
	int const code = attr & 0x7ff;
	int const color = attr >> 12;
	int const flags = (attr & 0x800) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(2, code, color, flags);
}

/* tile layout
0  xxxx.... ........  color
0  ....xxxx xxxxxxxx  gfx code
*/
TILE_GET_INFO_MEMBER(prehisle_state::get_tx_tile_info)
{
	int const attr = m_tx_vram[tile_index];
	int const code = attr & 0xfff;
	int const color = attr >> 12;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void prehisle_state::video_start()
{
	// ROM-based background layer
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(prehisle_state::get_bg_tile_info), this),
			TILEMAP_SCAN_COLS,      // scan order
			16, 16,                 // tile size
			1024, 32);              // tilemap size

	// RAM-based foreground layer (overlays most sprites)
	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(prehisle_state::get_fg_tile_info), this),
			TILEMAP_SCAN_COLS,      // scan order
			16, 16,                 // tile size
			256, 32);               // tilemap size
	m_fg_tilemap->set_transparent_pen(15);

	// text layer
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(prehisle_state::get_tx_tile_info), this),
			TILEMAP_SCAN_ROWS,      // scan order
			8, 8,                   // tile size
			32, 32);                // tilemap size
	m_tx_tilemap->set_transparent_pen(15);

	/* register for saving */
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
}

/* sprite layout

0  .......x xxxxxxxx  y, other bits unused?
1  .......x xxxxxxxx  x, other bits unused?
2  x....... ........  flip y
2  .x...... ........  flip x
2  ..x..... ........  ?
2  ...xxxxx xxxxxxxx  gfx code
3  xxxx.... ........  color+priority, other bits unknown
*/
void prehisle_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const spriteram16 = m_spriteram;

	for (int offs = 1024 - 4; offs >= 0; offs -= 4)
	{
		uint16_t const attr = spriteram16[offs + 2];
		uint16_t const code = attr & 0x1fff;
		uint16_t const color = spriteram16[offs + 3] >> 12;
		uint32_t const priority = (color < 0x4) ? 0 : GFX_PMASK_1; // correct?
		bool flipx = attr & 0x4000;
		bool flipy = attr & 0x8000;
		int16_t sx = spriteram16[offs + 1] & 0x1ff;
		int16_t sy = spriteram16[offs] & 0x1ff;

		// coordinates are 9-bit signed
		if (sx & 0x100) sx = -0x100 + (sx & 0xff);
		if (sy & 0x100) sy = -0x100 + (sy & 0xff);

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(3)->prio_transpen(
			bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			screen.priority(), priority,
			15); // transparent pen
	}
}

uint32_t prehisle_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	draw_sprites(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0);
	return 0;
}
