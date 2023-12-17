// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina
/***************************************************************************

    Legionnaire / Heated Barrel video hardware (derived from D-Con)

    priority test (used by Legionnaire, front to bottom):
    - OBJ 0
    - TXT
    - OBJ 1
    - BK3
    - OBJ 2
    - MBK
    - OBJ 3
    - LBK
    TODO: Anything else doesn't match this scheme (most notably Denjin Makai),
          guess it's selectable by PROM, CRTC or COP ...

***************************************************************************/

#include "emu.h"
#include "legionna.h"
#include "screen.h"


/******************************************************************************/


void legionna_state::tilemap_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_layer_disable);
}

void legionna_state::tile_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(m_scrollvals + offset);
	data = m_scrollvals[offset];

	tilemap_t *tm = nullptr;
	switch (offset/2)
	{
	case 0: tm = m_background_layer; break;
	case 1: tm = m_midground_layer; break;
	case 2: tm = m_foreground_layer; break;
	}
	if (offset & 1)
		tm->set_scrolly(0, data);
	else
		tm->set_scrollx(0, data);
}

void legionna_state::tile_vreg_1a_w(u16 data)
{
	flip_screen_set(data & 1);
	// TODO: other bits ...
}

void legionna_state::tile_scroll_base_w(offs_t offset, u16 data)
{
	// TODO: specific for Godzilla, needs visible area changes.
	if (offset == 7)
		m_text_layer->set_scrolldy(0x1ef - data,0x1ef - data);

	//printf("%02x %04x\n",offset,data);
}

void legionna_state::heatbrl_setgfxbank(u16 data)
{
	m_back_gfx_bank = (data & 0x4000) >> 2;
	m_background_layer->mark_all_dirty();
}

/*xxx- --- ---- ---- banking*/
void legionna_state::denjinmk_setgfxbank(u16 data)
{
	// this is either 0x0000 or 0xe000, except in two endings (MT #07416)
	m_back_gfx_bank = (data & 0x2000) >> 1; // Makai/Tarukusu endings
	m_mid_gfx_bank = (data & 0x4000) >> 2; //???
	m_fore_gfx_bank  = (data & 0x8000) >> 3; //???

	m_foreground_layer->mark_all_dirty();
	m_background_layer->mark_all_dirty();
	m_midground_layer->mark_all_dirty();
}

void legionna_state::videowrite_cb_w(offs_t offset, u16 data)
{
	//  map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	//  map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::foreground_w)).share("fore_data");
	//  map(0x102000, 0x1027ff).ram(); // .w(FUNC(legionna_state::midground_w)).share("mid_data");
	//  map(0x102800, 0x1037ff).ram(); // .w(FUNC(legionna_state::text_w)).share("textram");

	if (offset < 0x800 / 2)
	{
		background_w(offset, data);
	}
	else if (offset < 0x1000 /2)
	{
		offset -= 0x800 / 2;
		foreground_w(offset, data);
	}
	else if (offset < 0x1800/2)
	{
		offset -= 0x1000 / 2;
		midground_w(offset, data);
	}
	else if (offset < 0x2800/2)
	{
		offset -= 0x1800 / 2;
		text_w(offset, data);
	}
}

// TODO: move to COP device
void legionna_state::grainbow_layer_config_w(offs_t offset, u16 data, u16 mem_mask)
{
	// (0x8000|0x1ff), 0x200, 0x1ff, 0x200 written in sequence at startup
	COMBINE_DATA(&m_layer_config[offset]);
}

void legionna_state::background_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

void legionna_state::midground_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mid_data[offset]);
	m_midground_layer->mark_tile_dirty(offset);
}

void legionna_state::foreground_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

void legionna_state::text_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(legionna_state::get_back_tile_info)
{
	const u16 tile = m_back_data[tile_index];
	tileinfo.set(1, (tile & 0xfff) | m_back_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_mid_tile_info_split)
{
	const u16 tile = m_mid_data[tile_index];
	tileinfo.set(3, (tile & 0xfff) | m_mid_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_mid_tile_info_share_bgrom)
{
	const u16 tile = m_mid_data[tile_index];
	tileinfo.set(1, (tile & 0xfff) | 0x1000, ((tile >> 12) & 0xf) | 0x10, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_fore_tile_info)
{
	const u16 tile = m_fore_data[tile_index];
	tileinfo.set(2, (tile & 0xfff) | m_fore_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_text_tile_info)
{
	const u16 tile = m_textram[tile_index];
	tileinfo.set(0, tile & 0xfff, (tile >> 12) & 0xf, 0);
}

void legionna_state::common_video_allocate_ptr()
{
	m_back_data = make_unique_clear<u16[]>(0x800/2);
	m_fore_data = make_unique_clear<u16[]>(0x800/2);
	m_mid_data = make_unique_clear<u16[]>(0x800/2);
	m_textram = make_unique_clear<u16[]>(0x1000/2);
	m_scrollram16 = std::make_unique<u16[]>(0x60/2);
	m_paletteram = make_unique_clear<u16[]>(0x1000/2);
	m_palette->basemem().set(m_paletteram.get(), 0x1000/2 * sizeof(u16), 16, ENDIANNESS_BIG, 2);

	save_pointer(NAME(m_back_data), 0x800/2);
	save_pointer(NAME(m_fore_data), 0x800/2);
	save_pointer(NAME(m_mid_data), 0x800/2);
	save_pointer(NAME(m_textram), 0x1000/2);
	save_pointer(NAME(m_scrollram16), 0x60/2);
	save_pointer(NAME(m_paletteram), 0x1000/2);
	// saved for debugging
	save_pointer(NAME(m_sprite_pri_mask), 4);

	save_item(NAME(m_back_gfx_bank));
	save_item(NAME(m_mid_gfx_bank));
	save_item(NAME(m_fore_gfx_bank));
	save_item(NAME(m_layer_disable));
}

void legionna_state::common_video_start(bool split)
{
	common_video_allocate_ptr();

	m_background_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(legionna_state::get_back_tile_info)),                TILEMAP_SCAN_ROWS,16,16,32,32);
	if (split)
	{
		m_midground_layer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(legionna_state::get_mid_tile_info_split)),       TILEMAP_SCAN_ROWS,16,16,32,32);
	}
	else
	{
		m_midground_layer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(legionna_state::get_mid_tile_info_share_bgrom)), TILEMAP_SCAN_ROWS,16,16,32,32);
	}
	m_foreground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(legionna_state::get_fore_tile_info)),                TILEMAP_SCAN_ROWS,16,16,32,32);
	m_text_layer =       &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(legionna_state::get_text_tile_info)),                TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_background_layer->set_transparent_pen(15);
	m_midground_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);
}

VIDEO_START_MEMBER(legionna_state,legionna)
{
	common_video_start(false);

	m_sprite_pri_mask[0] = 0x0000;
	m_sprite_pri_mask[1] = 0xfff0;
	m_sprite_pri_mask[2] = 0xfffc;
	m_sprite_pri_mask[3] = 0xfffe;
}

VIDEO_START_MEMBER(legionna_state,heatbrl)
{
	common_video_start(true);

	m_sprite_pri_mask[0] = 0xfff0;
	m_sprite_pri_mask[1] = 0xfffc;
	m_sprite_pri_mask[2] = 0xfffe;
	// TODO: not shown?
	m_sprite_pri_mask[3] = 0xffff;
}

VIDEO_START_MEMBER(legionna_state,godzilla)
{
	common_video_start(false);

	m_sprite_pri_mask[0] = 0xfff0;
	m_sprite_pri_mask[1] = 0xfffc;
	m_sprite_pri_mask[2] = 0xfffe;
	// TODO: not shown?
	m_sprite_pri_mask[3] = 0xffff;
}

VIDEO_START_MEMBER(legionna_state,denjinmk)
{
	common_video_start(true);

	m_sprite_pri_mask[0] = 0xfff0; // normal sprites
	m_sprite_pri_mask[1] = 0xfffc; // luna park horse rides
	m_sprite_pri_mask[2] = 0xfffe; // door at the end of sewers part in level 1
	m_sprite_pri_mask[3] = 0x0000; // briefing guy in pre-stage and portraits before a boss fight

	m_text_layer->set_transparent_pen(7);//?
}

VIDEO_START_MEMBER(legionna_state,cupsoc)
{
	common_video_start(false);

	m_sprite_pri_mask[0] = 0xfff0; // title screen "Seibu Cup Soccer" elements
	m_sprite_pri_mask[1] = 0xfffc; // ?
	m_sprite_pri_mask[2] = 0xfffe; // ?
	m_sprite_pri_mask[3] = 0x0000; // 1P logo and radar dots (latter confirmed to stay above text layer)
}

VIDEO_START_MEMBER(legionna_state,grainbow)
{
	common_video_start(false);

	m_sprite_pri_mask[0] = 0xfff0; //
	m_sprite_pri_mask[1] = 0xfffc; // level 2 and 3
	m_sprite_pri_mask[2] = 0xfffe; // swamp monster mask effect
	m_sprite_pri_mask[3] = 0x0000; // Insert coin

	m_layer_config = std::make_unique<u16[]>(0x8/2);
}

/*************************************************************************

    Legionnaire Spriteram (similar to Dcon)
    ---------------------

    It has "big sprites" created by setting width or height >0. Tile
    numbers are read consecutively.

    +0   x....... ........  Sprite enable
    +0   .x...... ........  Flip x
    +0   ..x..... ........  Flip y ???
    +0   ...xxx.. ........  Width: do this many tiles horizontally
    +0   ......xx x.......  Height: do this many tiles vertically
    +0   ........ .x......  Tile bank,used in Denjin Makai / extra Priority in Grainbow (to external pin?)
    +0   ........ ..xxxxxx  Color bank

    +1   xx...... ........  Priority? (1=high?)
    +1   ..xxxxxx xxxxxxxx  Tile number

    +2   ----xxxx xxxxxxxx  X coordinate (signed)

    +3   b------- --------  more tile banking used by Denjin Makai
    +3   ----xxxx xxxxxxxx  Y coordinate (signed)

*************************************************************************/

u32 legionna_state::pri_cb(u8 pri, u8 ext)
{
	return m_sprite_pri_mask[pri];
}

u32 legionna_state::grainbow_pri_cb(u8 pri, u8 ext)
{
	// SD Gundam uses this arrangement, with bit 14 seemingly unused
	// side effect of using the COP sprite DMA?
	pri = (pri & 2) | (ext & 1);
	return m_sprite_pri_mask[pri];
}

u32 legionna_state::godzilla_tile_cb(u32 code, u8 ext, u8 y)
{
	if (ext)
		code |= 0x4000; //tile banking,used in Denjin Makai
	if (y)
		code |= 0x8000; //tile banking?,used in Denjin Makai
	return code;
}

u32 legionna_state::screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Setup the tilemaps */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);    /* wrong color? */

	if (BIT(~m_layer_disable, 0)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(~m_layer_disable, 1)) m_background_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (BIT(~m_layer_disable, 2)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (BIT(~m_layer_disable, 3)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_disable, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}

u32 legionna_state::screen_update_heatbrl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Setup the tilemaps */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);    /* wrong color? */

	// TODO: priority order is different than anything else?
	if (BIT(~m_layer_disable, 2)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(~m_layer_disable, 1)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (BIT(~m_layer_disable, 0)) m_background_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (BIT(~m_layer_disable, 3)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_disable, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}


u32 legionna_state::screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	// matches PCB recording for Denjin Makai, settable thru CRTC?
	bitmap.fill(0xff, cliprect);

	if (BIT(~m_layer_disable, 0)) m_background_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(~m_layer_disable, 1)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (BIT(~m_layer_disable, 2)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (BIT(~m_layer_disable, 3)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_disable, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();


	return 0;
}

u32 legionna_state::screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	if (BIT(~m_layer_disable, 0)) m_background_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(~m_layer_disable, 1)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (BIT(~m_layer_disable, 2)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (BIT(~m_layer_disable, 3)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_disable, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}
