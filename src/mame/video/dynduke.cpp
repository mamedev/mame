// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "includes/dynduke.h"



/******************************************************************************/

WRITE16_MEMBER(dynduke_state::background_w)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dynduke_state::foreground_w)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dynduke_state::text_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_bg_tile_info)
{
	int tile=m_back_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO_MEMBER(1,
			tile+m_back_bankbase,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_fg_tile_info)
{
	int tile=m_fore_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO_MEMBER(2,
			tile+m_fore_bankbase,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_tx_tile_info)
{
	int tile=m_videoram[tile_index];
	int color=(tile >> 8) & 0x0f;

	tile = (tile & 0xff) | ((tile & 0xc000) >> 6);

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void dynduke_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dynduke_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,      16,16,32,32);
	m_fg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dynduke_state::get_fg_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);
	m_tx_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dynduke_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);

	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_back_bankbase));
	save_item(NAME(m_fore_bankbase));
	save_item(NAME(m_back_enable));
	save_item(NAME(m_fore_enable));
	save_item(NAME(m_sprite_enable));
	save_item(NAME(m_txt_enable));
	save_item(NAME(m_old_back));
	save_item(NAME(m_old_fore));
}

WRITE16_MEMBER(dynduke_state::gfxbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data&0x01) m_back_bankbase=0x1000; else m_back_bankbase=0;
		if (data&0x10) m_fore_bankbase=0x1000; else m_fore_bankbase=0;

		if (m_back_bankbase!=m_old_back)
			m_bg_layer->mark_all_dirty();
		if (m_fore_bankbase!=m_old_fore)
			m_fg_layer->mark_all_dirty();

		m_old_back=m_back_bankbase;
		m_old_fore=m_fore_bankbase;
	}
}


WRITE16_MEMBER(dynduke_state::control_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// bit 0x80 toggles, maybe sprite buffering?
		// bit 0x40 is flipscreen
		// bit 0x20 not used?
		// bit 0x10 not used?
		// bit 0x08 is set on the title screen (sprite disable?)
		// bit 0x04 unused? txt disable?
		// bit 0x02 is used on the map screen (fore disable?)
		// bit 0x01 set when inserting coin.. bg disable?

		if (data&0x1) m_back_enable = 0; else m_back_enable = 1;
		if (data&0x2) m_fore_enable=0; else m_fore_enable=1;
		if (data&0x4) m_txt_enable = 0; else m_txt_enable = 1;
		if (data&0x8) m_sprite_enable=0; else m_sprite_enable=1;

		flip_screen_set(data & 0x40);
	}
}

void dynduke_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	UINT16 *buffered_spriteram16 = m_spriteram->buffer();
	int offs,fx,fy,x,y,color,sprite;

	if (!m_sprite_enable) return;

	for (offs = 0x800-4;offs >= 0;offs -= 4)
	{
		/* Don't draw empty sprite table entries */
		if ((buffered_spriteram16[offs+3] >> 8)!=0xf) continue;
		if (((buffered_spriteram16[offs+2]>>13)&3)!=pri) continue;

		fx= buffered_spriteram16[offs+0]&0x2000;
		fy= buffered_spriteram16[offs+0]&0x4000;
		y = buffered_spriteram16[offs+0] & 0xff;
		x = buffered_spriteram16[offs+2] & 0xff;

		if (buffered_spriteram16[offs+2]&0x100) x=0-(0x100-x);

		color = (buffered_spriteram16[offs+0]>>8)&0x1f;
		sprite = buffered_spriteram16[offs+1];
		sprite &= 0x3fff;

		if (flip_screen()) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
				sprite,
				color,fx,fy,x,y,15);
	}
}

void dynduke_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	/* The transparency / palette handling on the background layer is very strange */
	bitmap_ind16 &bm = m_bg_layer->pixmap();
	int scrolly, scrollx;
	int x,y;

	/* if we're disabled, don't draw */
	if (!m_back_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return;
	}

	scrolly = ((m_scroll_ram[0x01]&0x30)<<4)+((m_scroll_ram[0x02]&0x7f)<<1)+((m_scroll_ram[0x02]&0x80)>>7);
	scrollx = ((m_scroll_ram[0x09]&0x30)<<4)+((m_scroll_ram[0x0a]&0x7f)<<1)+((m_scroll_ram[0x0a]&0x80)>>7);

	for (y=0;y<256;y++)
	{
		int realy = (y + scrolly) & 0x1ff;
		UINT16 *src = &bm.pix16(realy);
		UINT16 *dst = &bitmap.pix16(y);


		for (x=0;x<256;x++)
		{
			int realx = (x + scrollx) & 0x1ff;
			UINT16 srcdat = src[realx];

			/* 0x01 - data bits
			   0x02
			   0x04
			   0x08
			   0x10 - extra colour bit? (first boss)
			   0x20 - priority over sprites
			   the old driver also had 'bg_palbase' but I don't see what it's for?
			*/

			if ((srcdat & 0x20) == pri)
			{
				if (srcdat & 0x10) srcdat += 0x400;
				//if (srcdat & 0x10) srcdat += machine().rand()&0x1f;

				srcdat = (srcdat & 0x000f) | ((srcdat & 0xffc0) >> 2);
				dst[x] = srcdat;
			}


		}
	}
}

UINT32 dynduke_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Setup the tilemaps */
	m_fg_layer->set_scrolly(0, ((m_scroll_ram[0x11]&0x30)<<4)+((m_scroll_ram[0x12]&0x7f)<<1)+((m_scroll_ram[0x12]&0x80)>>7) );
	m_fg_layer->set_scrollx(0, ((m_scroll_ram[0x19]&0x30)<<4)+((m_scroll_ram[0x1a]&0x7f)<<1)+((m_scroll_ram[0x1a]&0x80)>>7) );
	m_fg_layer->enable(m_fore_enable);
	m_tx_layer->enable(m_txt_enable);


	draw_background(bitmap, cliprect,0x00);
	draw_sprites(bitmap,cliprect,0); // Untested: does anything use it? Could be behind background
	draw_sprites(bitmap,cliprect,1);
	draw_background(bitmap, cliprect,0x20);

	draw_sprites(bitmap,cliprect,2);
	m_fg_layer->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect,3);
	m_tx_layer->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
