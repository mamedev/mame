// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    D-Con video hardware.

***************************************************************************/

#include "emu.h"
#include "includes/dcon.h"


/******************************************************************************/

WRITE16_MEMBER(dcon_state::gfxbank_w)
{
	if (data&1)
		m_gfx_bank_select=0x1000;
	else
		m_gfx_bank_select=0;
}

WRITE16_MEMBER(dcon_state::background_w)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::foreground_w)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::midground_w)
{
	COMBINE_DATA(&m_mid_data[offset]);
	m_midground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::text_w)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(dcon_state::get_back_tile_info)
{
	int tile=m_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_fore_tile_info)
{
	int tile=m_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO_MEMBER(2,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_mid_tile_info)
{
	int tile=m_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO_MEMBER(3,
			tile|m_gfx_bank_select,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_text_tile_info)
{
	int tile = m_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void dcon_state::video_start()
{
	m_background_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dcon_state::get_back_tile_info),this),TILEMAP_SCAN_ROWS,     16,16,32,32);
	m_foreground_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dcon_state::get_fore_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_midground_layer =  &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dcon_state::get_mid_tile_info),this), TILEMAP_SCAN_ROWS,16,16,32,32);
	m_text_layer =       &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dcon_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS,  8,8,64,32);

	m_midground_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	m_gfx_bank_select = 0;

	save_item(NAME(m_gfx_bank_select));
	save_item(NAME(m_last_gfx_bank));
	save_item(NAME(m_scroll_ram));
	save_item(NAME(m_layer_en));
}

void dcon_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay,inc,pri_mask = 0;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];

		switch((sprite>>14) & 3)
		{
		case 0: pri_mask = 0xf0; // above foreground layer
			break;
		case 1: pri_mask = 0xfc; // above midground layer
			break;
		case 2: pri_mask = 0xfe; // above background layer
			break;
		case 3: pri_mask = 0; // above text layer
			break;
		}

		sprite &= 0x3fff;

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		else y&=0x1ff;

		color = spriteram16[offs+0]&0x3f;
		fx = spriteram16[offs+0]&0x4000;
		fy = spriteram16[offs+0]&0x2000;
		dy=((spriteram16[offs+0]&0x0380)>>7)+1;
		dx=((spriteram16[offs+0]&0x1c00)>>10)+1;

		inc = 0;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx && !fy)
				{
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 + 512,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 - 512,
						screen.priority(),pri_mask,15);
				}
				else if (fx && !fy)
				{
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 + 512,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 - 512,
						screen.priority(),pri_mask,15);
				}
				else if (!fx && fy)
				{
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 + 512,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 - 512,
						screen.priority(),pri_mask,15);
				}
				else
				{
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 + 512,
						screen.priority(),pri_mask,15);

					// wrap around y
					m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 - 512,
						screen.priority(),pri_mask,15);
				}

				inc++;
			}
	}
}

UINT32 dcon_state::screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	/* Setup the tilemaps */
	m_background_layer->set_scrollx(0, m_scroll_ram[0] );
	m_background_layer->set_scrolly(0, m_scroll_ram[1] );
	m_midground_layer->set_scrollx(0, m_scroll_ram[2] );
	m_midground_layer->set_scrolly(0, m_scroll_ram[3] );
	m_foreground_layer->set_scrollx(0, m_scroll_ram[4] );
	m_foreground_layer->set_scrolly(0, m_scroll_ram[5] );

	if (!(m_layer_en & 1))
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	if (!(m_layer_en & 2))
		m_midground_layer->draw(screen, bitmap, cliprect, 0,1);

	if (!(m_layer_en & 4))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,2);

	if (!(m_layer_en & 8))
		m_text_layer->draw(screen, bitmap, cliprect, 0,4);

	if (!(m_layer_en & 0x10))
		draw_sprites(screen, bitmap,cliprect);

	return 0;
}

UINT32 dcon_state::screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	if (!(m_layer_en & 1))
		m_background_layer->draw(screen, bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	if (!(m_layer_en & 2))
		m_midground_layer->draw(screen, bitmap, cliprect, 0,1);

	if (!(m_layer_en & 4))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0,2);

	if (!(m_layer_en & 8))
		m_text_layer->draw(screen, bitmap, cliprect, 0,4);

	if (!(m_layer_en & 0x10))
		draw_sprites(screen, bitmap,cliprect);

	return 0;
}
