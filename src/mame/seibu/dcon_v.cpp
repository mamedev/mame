// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    D-Con video hardware.

***************************************************************************/

#include "emu.h"
#include "dcon.h"
#include "screen.h"


/******************************************************************************/

void dcon_state::gfxbank_w(uint16_t data)
{
	if (data&1)
		m_gfx_bank_select=0x1000;
	else
		m_gfx_bank_select=0;
}

void dcon_state::background_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

void dcon_state::foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

void dcon_state::midground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mid_data[offset]);
	m_midground_layer->mark_tile_dirty(offset);
}

void dcon_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(dcon_state::get_back_tile_info)
{
	int tile=m_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	tileinfo.set(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_fore_tile_info)
{
	int tile=m_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	tileinfo.set(2,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_mid_tile_info)
{
	int tile=m_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	tileinfo.set(3,
			tile|m_gfx_bank_select,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_text_tile_info)
{
	int tile = m_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	tileinfo.set(0,
			tile,
			color,
			0);
}

void dcon_state::video_start()
{
	m_background_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_back_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_foreground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_fore_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_midground_layer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_mid_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_text_layer =       &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_text_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

	m_midground_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	m_gfx_bank_select = 0;

	save_item(NAME(m_gfx_bank_select));
	save_item(NAME(m_last_gfx_bank));
	save_item(NAME(m_scroll_ram));
	save_item(NAME(m_layer_en));
}

uint32_t dcon_state::pri_cb(uint8_t pri, uint8_t ext)
{
	switch(pri)
	{
		case 0: return 0xf0; // above foreground layer
		case 1: return 0xfc; // above midground layer
		case 2: return 0xfe; // above background layer
		case 3:
		default: return 0; // above text layer
	}
}

uint32_t dcon_state::screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	/* Setup the tilemaps */
	m_background_layer->set_scrollx(0, m_scroll_ram[0] );
	m_background_layer->set_scrolly(0, m_scroll_ram[1] );
	m_midground_layer->set_scrollx(0, m_scroll_ram[2] );
	m_midground_layer->set_scrolly(0, m_scroll_ram[3] );
	m_foreground_layer->set_scrollx(0, m_scroll_ram[4] );
	m_foreground_layer->set_scrolly(0, m_scroll_ram[5] );

	if (BIT(~m_layer_en, 0))
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	if (BIT(~m_layer_en, 1))
		m_midground_layer->draw(screen, bitmap, cliprect, 0,1);

	if (BIT(~m_layer_en, 2))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,2);

	if (BIT(~m_layer_en, 3))
		m_text_layer->draw(screen, bitmap, cliprect, 0,4);

	if (BIT(~m_layer_en, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	return 0;
}

uint32_t dcon_state::screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	/* Gfx banking */
	if (m_last_gfx_bank!=m_gfx_bank_select)
	{
		m_midground_layer->mark_all_dirty();
		m_last_gfx_bank=m_gfx_bank_select;
	}

	/* Setup the tilemaps */
	m_background_layer->set_scrollx(0, m_scroll_ram[0]+128 );
	m_background_layer->set_scrolly(0, m_scroll_ram[1] );
	m_midground_layer->set_scrollx(0, m_scroll_ram[2]+128 );
	m_midground_layer->set_scrolly(0, m_scroll_ram[3] );
	m_foreground_layer->set_scrollx(0, m_scroll_ram[4]+128 );
	m_foreground_layer->set_scrolly(0, m_scroll_ram[5] );
	m_text_layer->set_scrollx(0, /*m_scroll_ram[6] + */ 128 );
	m_text_layer->set_scrolly(0, /*m_scroll_ram[7] + */ 0 );

	if (BIT(~m_layer_en, 0))
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	if (BIT(~m_layer_en, 1))
		m_midground_layer->draw(screen, bitmap, cliprect, 0,1);

	if (BIT(~m_layer_en, 2))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,2);

	if (BIT(~m_layer_en, 3))
		m_text_layer->draw(screen, bitmap, cliprect, 0,4);

	if (BIT(~m_layer_en, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	return 0;
}
