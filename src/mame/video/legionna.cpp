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
#include "includes/legionna.h"
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
	unsigned newbank = (data & 0x4000) >> 2;
	if (m_back_gfx_bank != newbank)
	{
		m_back_gfx_bank = newbank;
		m_background_layer->mark_all_dirty();
	}
}

/*xxx- --- ---- ---- banking*/
void legionna_state::denjinmk_setgfxbank(u16 data)
{
	unsigned newbank = (data & 0x2000) >> 1;//???
	if (m_fore_gfx_bank != newbank)
	{
		m_fore_gfx_bank = newbank;
		m_foreground_layer->mark_all_dirty();
	}

	newbank = (data & 0x4000) >> 2;
	if (m_back_gfx_bank != newbank)
	{
		m_back_gfx_bank = newbank;
		m_background_layer->mark_all_dirty();
	}

	newbank = (data & 0x8000) >> 3;//???
	if (m_mid_gfx_bank != newbank)
	{
		m_mid_gfx_bank = newbank;
		m_midground_layer->mark_all_dirty();
	}
}

void legionna_state::videowrite_cb_w(offs_t offset, u16 data)
{
	//  AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(background_w) AM_SHARE("back_data")
	//  AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(foreground_w) AM_SHARE("fore_data")
	//  AM_RANGE(0x102000, 0x1027ff) AM_RAM // _WRITE(midground_w) AM_SHARE("mid_data")
	//  AM_RANGE(0x102800, 0x1037ff) AM_RAM // _WRITE(text_w) AM_SHARE("textram")

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
	SET_TILE_INFO_MEMBER(2, (tile & 0xfff) | m_back_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_mid_tile_info_split)
{
	const u16 tile = m_mid_data[tile_index];
	SET_TILE_INFO_MEMBER(4, (tile & 0xfff) | m_mid_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_mid_tile_info_share_bgrom)
{
	const u16 tile = m_mid_data[tile_index];
	SET_TILE_INFO_MEMBER(2, (tile & 0xfff) | 0x1000, ((tile >> 12) & 0xf) | 0x10, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_fore_tile_info)
{
	const u16 tile = m_fore_data[tile_index];
	SET_TILE_INFO_MEMBER(3, (tile & 0xfff) | m_fore_gfx_bank, (tile >> 12) & 0xf, 0);
}

TILE_GET_INFO_MEMBER(legionna_state::get_text_tile_info)
{
	const u16 tile = m_textram[tile_index];
	SET_TILE_INFO_MEMBER(1, tile & 0xfff, (tile >> 12) & 0xf, 0);
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

	m_sprite_xoffs = 0;
	m_sprite_yoffs = 0;

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

void legionna_state::common_video_start(bool split, bool has_extended_banking, bool has_extended_priority)
{
	common_video_allocate_ptr();

	m_background_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(legionna_state::get_back_tile_info),this),                TILEMAP_SCAN_ROWS,16,16,32,32);
	if (split)
	{
		m_midground_layer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(legionna_state::get_mid_tile_info_split),this),       TILEMAP_SCAN_ROWS,16,16,32,32);
	}
	else
	{
		m_midground_layer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(legionna_state::get_mid_tile_info_share_bgrom),this), TILEMAP_SCAN_ROWS,16,16,32,32);
	}
	m_foreground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(legionna_state::get_fore_tile_info),this),                TILEMAP_SCAN_ROWS,16,16,32,32);
	m_text_layer =       &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(legionna_state::get_text_tile_info),this),                TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_has_extended_banking = has_extended_banking;
	m_has_extended_priority = has_extended_priority;

	m_background_layer->set_transparent_pen(15);
	m_midground_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);
}

VIDEO_START_MEMBER(legionna_state,legionna)
{
	common_video_start(false, false, false);

	m_sprite_pri_mask[0] = 0x0000;
	m_sprite_pri_mask[1] = 0xfff0;
	m_sprite_pri_mask[2] = 0xfffc;
	m_sprite_pri_mask[3] = 0xfffe;
}

VIDEO_START_MEMBER(legionna_state,heatbrl)
{
	common_video_start(true, false, false);

	m_sprite_pri_mask[0] = 0xfff0;
	m_sprite_pri_mask[1] = 0xfffc;
	m_sprite_pri_mask[2] = 0xfffe;
	// TODO: not shown?
	m_sprite_pri_mask[3] = 0xffff;
}

VIDEO_START_MEMBER(legionna_state,godzilla)
{
	common_video_start(false, true, false);

	m_sprite_pri_mask[0] = 0xfff0;
	m_sprite_pri_mask[1] = 0xfffc;
	m_sprite_pri_mask[2] = 0xfffe;
	// TODO: not shown?
	m_sprite_pri_mask[3] = 0xffff;
}

VIDEO_START_MEMBER(legionna_state,denjinmk)
{
	common_video_start(true, true, false);

	m_sprite_pri_mask[0] = 0xfff0; // normal sprites
	m_sprite_pri_mask[1] = 0xfffc; // luna park horse rides
	m_sprite_pri_mask[2] = 0xfffe; // door at the end of sewers part in level 1
	m_sprite_pri_mask[3] = 0x0000; // briefing guy in pre-stage and portraits before a boss fight

	m_text_layer->set_transparent_pen(7);//?
}

VIDEO_START_MEMBER(legionna_state,cupsoc)
{
	common_video_start(false, false, false);
	
	m_sprite_pri_mask[0] = 0xfff0; // title screen "Seibu Cup Soccer" elements
	m_sprite_pri_mask[1] = 0xfffc; // ?
	m_sprite_pri_mask[2] = 0xfffe; // ?
	m_sprite_pri_mask[3] = 0x0000; // 1P logo and radar dots (latter confirmed to stay above text layer)
}

VIDEO_START_MEMBER(legionna_state,grainbow)
{
	common_video_start(false, false, true);
	m_sprite_xoffs = m_sprite_yoffs = 16;
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

void legionna_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x400; offs += 4)
	{
		const u16 data = m_spriteram[offs];
		if (!(data & 0x8000)) continue;

		u8 cur_pri = 0;
		int pri_mask = 0;

		if (m_has_extended_priority)
		{
			// SD Gundam uses this arrangement, with bit 14 seemingly unused
			// side effect of using the COP sprite DMA?
			cur_pri = (m_spriteram[offs+1] & 0x8000) >> 14;

			if (data & 0x0040)
				cur_pri |= 1;
		}
		else
			cur_pri = (m_spriteram[offs+1] & 0xc000) >> 14;

		pri_mask = m_sprite_pri_mask[cur_pri];

		u32 sprite = m_spriteram[offs+1];

		sprite &= 0x3fff;

		if (m_has_extended_banking)
		{
			if (data & 0x0040)
			{
				sprite |= 0x4000;//tile banking,used in Denjin Makai
			}
			if (m_spriteram[offs+3] & 0x8000)
			{
				sprite |= 0x8000;//tile banking?,used in Denjin Makai
			}
		}

		int y = m_spriteram[offs+3];
		int x = m_spriteram[offs+2];

		/* heated barrel hardware seems to need 0x1ff with 0x100 sign bit for sprite wrap,
		   this doesn't work on denjin makai as the visible area is larger */
		if (cliprect.max_x<(320-1))
		{
			x &= 0x1ff;
			y &= 0x1ff;

			if (x & 0x100) x -= 0x200;
			if (y & 0x100) y -= 0x200;
		}
		else
		{
			x &= 0xfff;
			y &= 0xfff;

			if (x & 0x800) x -= 0x1000;
			if (y & 0x800) y -= 0x1000;

		}


		const u32 color =  (data & 0x003f);
		const u8 fx     =  (data & 0x4000) >> 14;
		const u8 fy     =  (data & 0x2000) >> 13;
		const u8 dy     = ((data & 0x0380) >> 7)  + 1;
		const u8 dx     = ((data & 0x1c00) >> 10) + 1;

		if (!fx)
		{
			if (!fy)
			{
				for (int ax = 0; ax < dx; ax++)
					for (int ay = 0; ay < dy; ay++)
					{
						m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,(x+ax*16)+m_sprite_xoffs,y+ay*16+m_sprite_yoffs,
						screen.priority(),pri_mask, 15);
					}
			}
			else
			{
				for (int ax = 0; ax < dx; ax++)
					for (int ay = 0; ay < dy; ay++)
					{
						m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,(x+ax*16)+m_sprite_xoffs,y+(dy-ay-1)*16+m_sprite_yoffs,
						screen.priority(),pri_mask,15);
					}
			}
		}
		else
		{
			if (!fy)
			{
				for (int ax = 0; ax < dx; ax++)
					for (int ay = 0; ay < dy; ay++)
					{
						m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,(x+(dx-ax-1)*16)+m_sprite_xoffs,y+ay*16+m_sprite_yoffs,
						screen.priority(),pri_mask,15);
					}
			}
			else
			{
				for (int ax = 0; ax < dx; ax++)
					for (int ay = 0; ay < dy; ay++)
					{
						m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,(x+(dx-ax-1)*16)+m_sprite_xoffs,y+(dy-ay-1)*16+m_sprite_yoffs,
						screen.priority(),pri_mask, 15);
					}
			}
		}
	}
}

u32 legionna_state::screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Setup the tilemaps */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);    /* wrong color? */

	if (!(m_layer_disable & 0x0001)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (!(m_layer_disable & 0x0002)) m_background_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (!(m_layer_disable & 0x0004)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (!(m_layer_disable & 0x0008)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (!(m_layer_disable & 0x0010))
		draw_sprites(screen,bitmap,cliprect);

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
	if (!(m_layer_disable & 0x0004)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (!(m_layer_disable & 0x0002)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (!(m_layer_disable & 0x0001)) m_background_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (!(m_layer_disable & 0x0008)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (!(m_layer_disable & 0x0010))
		draw_sprites(screen,bitmap,cliprect);

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}


u32 legionna_state::screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	// matches PCB recording for Denjin Makai, settable thru CRTC?
	bitmap.fill(0xff, cliprect);

	if (!(m_layer_disable & 0x0001)) m_background_layer->draw(screen, bitmap, cliprect, 0, 0);
	if (!(m_layer_disable & 0x0002)) m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);
	if (!(m_layer_disable & 0x0004)) m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);
	if (!(m_layer_disable & 0x0008)) m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (!(m_layer_disable & 0x0010))
		draw_sprites(screen,bitmap,cliprect);

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();


	return 0;
}

u32 legionna_state::screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	if (!(m_layer_disable & 1))
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);

	if (!(m_layer_disable & 2))
		m_midground_layer->draw(screen, bitmap, cliprect, 0,1);

	if (!(m_layer_disable & 4))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,2);

	if (!(m_layer_disable & 8))
		m_text_layer->draw(screen, bitmap, cliprect, 0,4);

	if (!(m_layer_disable & 0x0010))
		draw_sprites(screen,bitmap,cliprect);

	//if (machine().input().code_pressed_once(KEYCODE_Z))
	//  if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}
