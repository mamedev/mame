// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Seibu Raiden hardware

    Functions to emulate the video hardware

*******************************************************************************/

#include "emu.h"
#include "includes/raiden.h"
#include "screen.h"


/******************************************************************************/

void raiden_state::bgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

void raiden_state::fgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void raiden_state::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}


void raiden_state::raiden_control_w(u8 data)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d2: text layer disable
	// d3: sprite layer disable
	// d4: unused
	// d5: unused
	// d6: flipscreen
	// d7: toggles, maybe spriteram bank? (for buffering)
	m_bg_layer_enabled = BIT(~data, 0);
	m_fg_layer_enabled = BIT(~data, 1);
	m_tx_layer_enabled = BIT(~data, 2);
	m_sp_layer_enabled = BIT(~data, 3);

	m_flipscreen = BIT(data, 6);
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

void raidenb_state::raidenb_control_w(u8 data)
{
	// d1: flipscreen
	// d2: toggles, maybe spriteram bank? (for buffering)
	// d3: text layer disable (i guess raidenb textlayer isn't part of sei_crtc?)
	// other bits: unused
	m_tx_layer_enabled = BIT(~data, 3);

	m_flipscreen = BIT(data, 1);
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

void raidenb_state::raidenb_layer_enable_w(u16 data)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d4: sprite layer disable
	// other bits: unused? (d2-d3 always set, d5-d7 always clear)
	m_bg_layer_enabled = BIT(~data, 0);
	m_fg_layer_enabled = BIT(~data, 1);
	m_sp_layer_enabled = BIT(~data, 4);
}


/******************************************************************************/

void raiden_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap)
{
	if (!m_sp_layer_enabled)
		return;

	u16 *sprites = m_spriteram->buffer();
	const u32 size = m_spriteram->bytes() / 2;
	gfx_element *gfx = m_gfxdecode->gfx(3);

	for (int offs = 0; offs < size; offs += 4)
	{
		/*
		    Word #0
		    x------- --------  active
		    -x------ --------  flipy
		    --x----- --------  flipx
		    ---x---- --------  unused
		    ----xxxx --------  color
		    -------- xxxxxxxx  y

		    Word #1
		    x------- --------  ? (set when groundboss explodes)
		    -xxx---- --------  unused
		    ----xxxx xxxxxxxx  code

		    Word #2
		    xx------ --------  priority
		    --xxxxx- --------  unused
		    -------x xxxxxxxx  x (signed)

		    Word #3 unused
		*/

		if (!BIT(sprites[offs + 0], 15))
			continue;

		const u8 priority = BIT(sprites[offs + 2], 14, 2);
		if (priority == 0)
			continue;

		u32 pri_mask = GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1;
		switch (priority)
		{
		case 1: // draw sprites underneath foreground
			pri_mask = GFX_PMASK_4 | GFX_PMASK_2;
			break;
		case 2:
		case 3: // rest of sprites, draw underneath text
		default:
			pri_mask = GFX_PMASK_4;
			break;
		}

		bool flipy = BIT(sprites[offs + 0], 14);
		bool flipx = BIT(sprites[offs + 0], 13);
		const u32 color = BIT(sprites[offs + 0], 8, 4);
		const u32 code = BIT(sprites[offs + 1], 0, 12);

		s32 y = BIT(sprites[offs + 0], 0, 8);
		s32 x = BIT(sprites[offs + 2], 0, 9);
		if (BIT(x, 8)) // sign bit
			x |= ~0x1ff;

		if (m_flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipy = !flipy;
			flipx = !flipx;
		}

		gfx->prio_transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, primap, pri_mask, 15);
	}
}

u32 raiden_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *scrollregs)
{
	// set tilemaps scroll
	m_bg_layer->set_scrollx(0, scrollregs[0]);
	m_bg_layer->set_scrolly(0, scrollregs[1]);
	m_fg_layer->set_scrollx(0, scrollregs[2]);
	m_fg_layer->set_scrolly(0, scrollregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	// back layer
	if (m_bg_layer_enabled)
		m_bg_layer->draw(screen, bitmap, cliprect, 0, 1);

	// fore layer
	if (m_fg_layer_enabled)
		m_fg_layer->draw(screen, bitmap, cliprect, 0, 2);

	// text layer
	if (m_tx_layer_enabled)
		m_tx_layer->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(bitmap, cliprect, screen.priority());

	return 0;
}

u32 raiden_state::screen_update_raiden(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set up scrollregs
	// scroll_ram is only 8 bits wide. 4 bytes per scroll, skip uneven ones
	// 00-03: 28 *0 ** ae  -  bg layer scroll y
	// 08-0b: 28 *0 ** b9  -  bg layer scroll x
	// 10-13: 28 *0 ** ae  -  fg layer scroll y
	// 18-1b: 28 *0 ** b9  -  fg layer scroll x
	u16 scrollregs[4];
	scrollregs[0] = ((m_scroll_ram[0x09] & 0xf0) << 4) | ((m_scroll_ram[0x0a] & 0x7f) << 1) | ((m_scroll_ram[0x0a] & 0x80) >> 7);
	scrollregs[1] = ((m_scroll_ram[0x01] & 0xf0) << 4) | ((m_scroll_ram[0x02] & 0x7f) << 1) | ((m_scroll_ram[0x02] & 0x80) >> 7);
	scrollregs[2] = ((m_scroll_ram[0x19] & 0xf0) << 4) | ((m_scroll_ram[0x1a] & 0x7f) << 1) | ((m_scroll_ram[0x1a] & 0x80) >> 7);
	scrollregs[3] = ((m_scroll_ram[0x11] & 0xf0) << 4) | ((m_scroll_ram[0x12] & 0x7f) << 1) | ((m_scroll_ram[0x12] & 0x80) >> 7);

	return screen_update_common(screen, bitmap, cliprect, scrollregs);
}

u32 raidenb_state::screen_update_raidenb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update_common(screen, bitmap, cliprect, m_raidenb_scroll_ram);
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(raiden_state::get_back_tile_info)
{
	const u16 tiledata = m_bgram[tile_index];
	const u32 tile = tiledata & 0x0fff;
	const u32 color = tiledata >> 12;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_fore_tile_info)
{
	const u16 tiledata = m_fgram[tile_index];
	const u32 tile = tiledata & 0x0fff;
	const u32 color = tiledata >> 12;

	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_text_tile_info)
{
	const u16 tiledata = m_textram[tile_index];
	const u32 tile = (tiledata & 0xff) | ((tiledata >> 6) & 0x300);
	const u32 color = (tiledata >> 8) & 0x0f;

	tileinfo.set(0, tile, color, 0);
}

void raiden_state::common_video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_back_tile_info)), TILEMAP_SCAN_COLS, 16,16, 32,32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_fore_tile_info)), TILEMAP_SCAN_COLS, 16,16, 32,32);
	m_fg_layer->set_transparent_pen(15);

	save_item(NAME(m_bg_layer_enabled));
	save_item(NAME(m_fg_layer_enabled));
	save_item(NAME(m_tx_layer_enabled));
	save_item(NAME(m_sp_layer_enabled));
	save_item(NAME(m_flipscreen));
}

void raiden_state::video_start()
{
	common_video_start();

	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32,32);
	m_tx_layer->set_transparent_pen(15);
}

void raidenb_state::video_start()
{
	common_video_start();

	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raidenb_state::get_text_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32,32);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_raidenb_scroll_ram));
}
