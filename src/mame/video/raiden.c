// license:???
// copyright-holders:Oliver Bergmann, Bryan McPhail, Randy Mongenel
/*******************************************************************************

    Seibu Raiden hardware

    Functions to emulate the video hardware

*******************************************************************************/

#include "emu.h"
#include "includes/raiden.h"


/******************************************************************************/

WRITE16_MEMBER(raiden_state::raiden_background_w)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden_state::raiden_foreground_w)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden_state::raiden_text_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}


WRITE8_MEMBER(raiden_state::raiden_control_w)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d2: text layer disable
	// d3: sprite layer disable
	// d4: unused
	// d5: unused
	// d6: flipscreen
	// d7: toggles, maybe spriteram bank? (for buffering)
	m_bg_layer_enabled = ~data & 0x01;
	m_fg_layer_enabled = ~data & 0x02;
	m_tx_layer_enabled = ~data & 0x04;
	m_sp_layer_enabled = ~data & 0x08;

	m_flipscreen = data & 0x40;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

WRITE8_MEMBER(raiden_state::raidenb_control_w)
{
	// d1: flipscreen
	// d2: toggles, maybe spriteram bank? (for buffering)
	// d3: text layer disable (i guess raidenb textlayer isn't part of sei_crtc?)
	// other bits: unused
	m_tx_layer_enabled = ~data & 0x08;

	m_flipscreen = data & 0x02;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

WRITE16_MEMBER(raiden_state::raidenb_layer_enable_w)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d4: sprite layer disable
	// other bits: unused? (d2-d3 always set, d5-d7 always clear)
	m_bg_layer_enabled = ~data & 0x01;
	m_fg_layer_enabled = ~data & 0x02;
	m_sp_layer_enabled = ~data & 0x10;
}


/******************************************************************************/

void raiden_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask)
{
	if (!m_sp_layer_enabled)
		return;

	UINT16 *sprites = m_spriteram->buffer();
	gfx_element *gfx = m_gfxdecode->gfx(3);

	for (int offs = 0x1000/2-4; offs >= 0; offs -= 4)
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

		if (!(sprites[offs + 0] & 0x8000))
			continue;

		int priority = sprites[offs + 2] >> 14 & 0x3;
		if ((priority & pri_mask) == 0)
			continue;

		int flipy = sprites[offs + 0] & 0x4000;
		int flipx = sprites[offs + 0] & 0x2000;
		int color = (sprites[offs + 0] & 0xf00) >> 8;
		int code = sprites[offs + 1] & 0xfff;

		int y = sprites[offs + 0] & 0xff;
		int x = sprites[offs + 2] & 0xff;
		if (sprites[offs + 2] & 0x100)
			x = -(0x100 - x);

		if (m_flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipy = !flipy;
			flipx = !flipx;
		}

			gfx->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 15);
	}
}

UINT32 raiden_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *scrollregs)
{
	// set tilemaps scroll
	m_bg_layer->set_scrollx(0, scrollregs[0]);
	m_bg_layer->set_scrolly(0, scrollregs[1]);
	m_fg_layer->set_scrollx(0, scrollregs[2]);
	m_fg_layer->set_scrolly(0, scrollregs[3]);

	bitmap.fill(m_palette->black_pen(), cliprect);

	// back layer
	if (m_bg_layer_enabled)
		m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);

	// draw sprites underneath foreground
	draw_sprites(bitmap, cliprect, 1);

	// fore layer
	if (m_fg_layer_enabled)
		m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);

	// rest of sprites
	draw_sprites(bitmap, cliprect, 2);

	// text layer
	if (m_tx_layer_enabled)
		m_tx_layer->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

UINT32 raiden_state::screen_update_raiden(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set up scrollregs
	// scroll_ram is only 8 bits wide. 4 bytes per scroll, skip uneven ones
	// 00-03: 28 *0 ** ae  -  bg layer scroll y
	// 08-0b: 28 *0 ** b9  -  bg layer scroll x
	// 10-13: 28 *0 ** ae  -  fg layer scroll y
	// 18-1b: 28 *0 ** b9  -  fg layer scroll x
	UINT16 scrollregs[4];
	scrollregs[0] = ((m_scroll_ram[0x09] & 0xf0) << 4) | ((m_scroll_ram[0x0a] & 0x7f) << 1) | ((m_scroll_ram[0x0a] & 0x80) >> 7);
	scrollregs[1] = ((m_scroll_ram[0x01] & 0xf0) << 4) | ((m_scroll_ram[0x02] & 0x7f) << 1) | ((m_scroll_ram[0x02] & 0x80) >> 7);
	scrollregs[2] = ((m_scroll_ram[0x19] & 0xf0) << 4) | ((m_scroll_ram[0x1a] & 0x7f) << 1) | ((m_scroll_ram[0x1a] & 0x80) >> 7);
	scrollregs[3] = ((m_scroll_ram[0x11] & 0xf0) << 4) | ((m_scroll_ram[0x12] & 0x7f) << 1) | ((m_scroll_ram[0x12] & 0x80) >> 7);

	return screen_update_common(screen, bitmap, cliprect, scrollregs);
}

UINT32 raiden_state::screen_update_raidenb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update_common(screen, bitmap, cliprect, m_raidenb_scroll_ram);
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(raiden_state::get_back_tile_info)
{
	int tiledata = m_back_data[tile_index];
	int tile = tiledata & 0x0fff;
	int color = tiledata >> 12;

	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_fore_tile_info)
{
	int tiledata = m_fore_data[tile_index];
	int tile = tiledata & 0x0fff;
	int color = tiledata >> 12;

	SET_TILE_INFO_MEMBER(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_text_tile_info)
{
	int tiledata = m_videoram[tile_index];
	int tile = (tiledata & 0xff) | ((tiledata >> 6) & 0x300);
	int color = (tiledata >> 8) & 0x0f;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

void raiden_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_back_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);
	m_fg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_fore_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);
	m_tx_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS,8, 8, 32,32);

	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_bg_layer_enabled));
	save_item(NAME(m_fg_layer_enabled));
	save_item(NAME(m_tx_layer_enabled));
	save_item(NAME(m_sp_layer_enabled));
	save_item(NAME(m_flipscreen));
}

VIDEO_START_MEMBER(raiden_state,raidenb)
{
	m_bg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_back_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);
	m_fg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_fore_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);
	m_tx_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden_state::get_text_tile_info),this),TILEMAP_SCAN_COLS,8, 8, 32,32);

	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_bg_layer_enabled));
	save_item(NAME(m_fg_layer_enabled));
	save_item(NAME(m_tx_layer_enabled));
	save_item(NAME(m_sp_layer_enabled));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_raidenb_scroll_ram));
}
