// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    Irem M90 system.  There is 1 video chip - NANAO GA-25, it produces
    2 tilemaps and sprites.  16 control bytes:

    0:  Playfield 1 X scroll
    2:  Playfield 1 Y scroll
    4:  Playfield 2 X scroll
    6:  Playfield 2 Y scroll
    8:  Bit 0x01 - unknown (set by hasamu)
    10: Playfield 1 control
        Bits0x03 - Playfield 1 VRAM base
        Bit 0x04 - Playfield 1 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 1 disable
        Bit 0x20 - Playfield 1 rowscroll enable
        Bit 0x40 - Playfield 1 y-offset table enable
    12: Playfield 2 control
        Bits0x03 - Playfield 2 VRAM base
        Bit 0x04 - Playfield 2 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 2 disable
        Bit 0x20 - Playfield 2 rowscroll enable
        Bit 0x40 - Playfield 1 y-offset table enable
    14: Bits0x03 - Sprite/Tile Priority (related to sprite color)

    Emulation by Bryan McPhail, mish@tendril.co.uk, thanks to Chris Hardy!

*****************************************************************************/

#include "emu.h"
#include "includes/m90.h"


inline void m90_state::get_tile_info(tile_data &tileinfo,int tile_index,int layer,int page_mask)
{
	int tile,color;
	tile_index = 2*tile_index + ((m_video_control_data[5+layer] & page_mask) * 0x2000);

	tile=m_video_data[tile_index];
	color=m_video_data[tile_index+1];
	SET_TILE_INFO_MEMBER(0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

inline void m90_state::bomblord_get_tile_info(tile_data &tileinfo,int tile_index,int layer)
{
	int tile,color;
	tile_index = 2*tile_index + (layer * 0x2000);

	tile=m_video_data[tile_index];
	color=m_video_data[tile_index+1];
	SET_TILE_INFO_MEMBER(0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

inline void m90_state::dynablsb_get_tile_info(tile_data &tileinfo,int tile_index,int layer)
{
	int tile,color;
	tile_index = 2*tile_index + (layer * 0x2000);

	tile=m_video_data[tile_index];
	color=m_video_data[tile_index+1];
	SET_TILE_INFO_MEMBER(0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

TILE_GET_INFO_MEMBER(m90_state::get_pf1_tile_info){ get_tile_info(tileinfo,tile_index,0,0x3); }
TILE_GET_INFO_MEMBER(m90_state::get_pf1w_tile_info){ get_tile_info(tileinfo,tile_index,0,0x2); }
TILE_GET_INFO_MEMBER(m90_state::get_pf2_tile_info){ get_tile_info(tileinfo,tile_index,1,0x3); }
TILE_GET_INFO_MEMBER(m90_state::get_pf2w_tile_info){ get_tile_info(tileinfo,tile_index,1,0x2); }

TILE_GET_INFO_MEMBER(m90_state::bomblord_get_pf1_tile_info){ bomblord_get_tile_info(tileinfo,tile_index,0); }
TILE_GET_INFO_MEMBER(m90_state::bomblord_get_pf1w_tile_info){ bomblord_get_tile_info(tileinfo,tile_index,0); }
TILE_GET_INFO_MEMBER(m90_state::bomblord_get_pf2_tile_info){ bomblord_get_tile_info(tileinfo,tile_index,2); }
TILE_GET_INFO_MEMBER(m90_state::bomblord_get_pf2w_tile_info){ bomblord_get_tile_info(tileinfo,tile_index,2); }

TILE_GET_INFO_MEMBER(m90_state::dynablsb_get_pf1_tile_info){ dynablsb_get_tile_info(tileinfo,tile_index,0); }
TILE_GET_INFO_MEMBER(m90_state::dynablsb_get_pf1w_tile_info){ dynablsb_get_tile_info(tileinfo,tile_index,0); }
TILE_GET_INFO_MEMBER(m90_state::dynablsb_get_pf2_tile_info){ dynablsb_get_tile_info(tileinfo,tile_index,2); }
TILE_GET_INFO_MEMBER(m90_state::dynablsb_get_pf2w_tile_info){ dynablsb_get_tile_info(tileinfo,tile_index,2); }

void m90_state::video_start()
{
	m_pf1_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::get_pf1_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf1_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::get_pf1w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);
	m_pf2_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::get_pf2_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf2_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::get_pf2w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);

	m_pf1_layer->set_transparent_pen(0);
	m_pf1_wide_layer->set_transparent_pen(0);

	save_item(NAME(m_video_control_data));
	save_item(NAME(m_last_pf1));
	save_item(NAME(m_last_pf2));
}

VIDEO_START_MEMBER(m90_state,bomblord)
{
	m_pf1_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::bomblord_get_pf1_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf1_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::bomblord_get_pf1w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);
	m_pf2_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::bomblord_get_pf2_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf2_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::bomblord_get_pf2w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);

	m_pf2_layer->set_transparent_pen(0);
	m_pf2_wide_layer->set_transparent_pen(0);
	m_pf1_layer->set_transparent_pen(0);
	m_pf1_wide_layer->set_transparent_pen(0);

	save_item(NAME(m_video_control_data));
}

VIDEO_START_MEMBER(m90_state,dynablsb)
{
	m_pf1_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::dynablsb_get_pf1_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf1_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::dynablsb_get_pf1w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);
	m_pf2_layer =      &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::dynablsb_get_pf2_tile_info),this), TILEMAP_SCAN_ROWS,8,8,64,64);
	m_pf2_wide_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m90_state::dynablsb_get_pf2w_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);

	m_pf2_layer->set_transparent_pen(0);
	m_pf2_wide_layer->set_transparent_pen(0);

	save_item(NAME(m_video_control_data));
}

void m90_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram = m_video_data + 0xee00/2;;
	int offs;

	for (offs = 0x1f2/2; offs >= 0; offs -= 3)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i;

		sprite = spriteram[offs+1];
		colour = (spriteram[offs+0] >> 9) & 0x0f;

		y = spriteram[offs+0] & 0x1ff;
		x = spriteram[offs+2] & 0x1ff;

		x = x - 16;
		y = 512 - y;

		fx = (spriteram[offs+2] >> 8) & 0x02;
		fy = (spriteram[offs+0] >> 8) & 0x80;

		y_multi = 1 << ((spriteram[offs+0] & 0x6000) >> 13);
		y -= 16 * y_multi;

		for (i = 0;i < y_multi;i++)

			if (m_video_control_data[7] & 0x01)
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					(colour & 0x08) ? 0x00 : 0x02,0);
			else if (m_video_control_data[7] & 0x02)
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					((colour & 0x0c)==0x0c) ? 0x00 : 0x02,0);
			else
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					0x02,0);
	}
}

void m90_state::bomblord_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;


	while ((offs < m_spriteram.bytes()/2) & (spriteram16[offs+0] != 0x8000))
	{
		last_sprite = offs;
		offs += 4;
	}

	for (offs = last_sprite; offs >= 0; offs -= 4)
	{
		sprite = spriteram16[offs+1];
		colour = (spriteram16[offs+2] >> 9) & 0x0f;

		y = (spriteram16[offs+0] & 0x1ff) + 152;
		x = (spriteram16[offs+3] & 0x1ff) + 16;

		x = x - 16;
		y = 512 - y;

		if (y < 0) y += 512;

		fx = (spriteram16[offs+3] >> 8) & 0x02;
		fy = (spriteram16[offs+2] >> 8) & 0x80;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				colour,
				fx,fy,
				x,y,
				screen.priority(),
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

void m90_state::dynablsb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;

	while ((offs < m_spriteram.bytes()/2) & (spriteram16[offs+0] != 0xffff))
	{
		last_sprite = offs;
		offs += 4;
	}

	for (offs = last_sprite; offs >= 0; offs -= 4)
	{
		sprite = spriteram16[offs+1];
		colour = (spriteram16[offs+2] >> 9) & 0x0f;

		y = (spriteram16[offs+0] & 0x1ff) + 288;
		x = (spriteram16[offs+3] & 0x1ff) - 64;

		x = x - 16;
		y = 512 - y;

		if (y < 0) y += 512;

		fx = (spriteram16[offs+3] >> 8) & 0x02;
		fy = (spriteram16[offs+2] >> 8) & 0x80;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				colour,
				fx,fy,
				x,y,
				screen.priority(),
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

WRITE16_MEMBER(m90_state::m90_video_control_w)
{
	COMBINE_DATA(&m_video_control_data[offset]);
}

void m90_state::markdirty(tilemap_t *tmap,int page,offs_t offset)
{
	offset -= page * 0x2000;

	if (offset < 0x2000)
		tmap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(m90_state::m90_video_w)
{
	COMBINE_DATA(&m_video_data[offset]);

	markdirty(m_pf1_layer,     m_video_control_data[5] & 0x3,offset);
	markdirty(m_pf1_wide_layer,m_video_control_data[5] & 0x2,offset);
	markdirty(m_pf2_layer,     m_video_control_data[6] & 0x3,offset);
	markdirty(m_pf2_wide_layer,m_video_control_data[6] & 0x2,offset);
}

UINT32 m90_state::screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 pf1_base = m_video_control_data[5] & 0x3;
	UINT8 pf2_base = m_video_control_data[6] & 0x3;
	int i,pf1_enable,pf2_enable, video_enable;

	if (m_video_control_data[7]&0x04) video_enable=0; else video_enable=1;
	if (m_video_control_data[5]&0x10) pf1_enable=0; else pf1_enable=1;
	if (m_video_control_data[6]&0x10) pf2_enable=0; else pf2_enable=1;

// m_pf1_layer->enable(pf1_enable);
// m_pf2_layer->enable(pf2_enable);
// m_pf1_wide_layer->enable(pf1_enable);
// m_pf2_wide_layer->enable(pf2_enable);

	/* Dirty tilemaps if VRAM base changes */
	if (pf1_base!=m_last_pf1)
	{
		m_pf1_layer->mark_all_dirty();
		m_pf1_wide_layer->mark_all_dirty();
	}
	if (pf2_base!=m_last_pf2)
	{
		m_pf2_layer->mark_all_dirty();
		m_pf2_wide_layer->mark_all_dirty();
	}
	m_last_pf1=pf1_base;
	m_last_pf2=pf2_base;

	/* Setup scrolling */
	if (m_video_control_data[5]&0x20)
	{
		m_pf1_layer->set_scroll_rows(512);
		m_pf1_wide_layer->set_scroll_rows(512);

		for (i=0; i<512; i++)
			m_pf1_layer->set_scrollx(i, m_video_data[0xf000/2+i]+2);
		for (i=0; i<512; i++)
			m_pf1_wide_layer->set_scrollx(i, m_video_data[0xf000/2+i]+256+2);

	}
	else
	{
		m_pf1_layer->set_scroll_rows(1);
		m_pf1_wide_layer->set_scroll_rows(1);
		m_pf1_layer->set_scrollx(0, m_video_control_data[1]+2);
		m_pf1_wide_layer->set_scrollx(0, m_video_control_data[1]+256+2);
	}

	/* Setup scrolling */
	if (m_video_control_data[6]&0x20)
	{
		m_pf2_layer->set_scroll_rows(512);
		m_pf2_wide_layer->set_scroll_rows(512);
		for (i=0; i<512; i++)
			m_pf2_layer->set_scrollx(i, m_video_data[0xf400/2+i]-2);
		for (i=0; i<512; i++)
			m_pf2_wide_layer->set_scrollx(i, m_video_data[0xf400/2+i]+256-2);
	} else {
		m_pf2_layer->set_scroll_rows(1);
		m_pf2_wide_layer->set_scroll_rows(1);
		m_pf2_layer->set_scrollx(0, m_video_control_data[3]-2);
		m_pf2_wide_layer->set_scrollx(0, m_video_control_data[3]+256-2 );
	}

	screen.priority().fill(0, cliprect);

	if (video_enable)
	{
		if (pf2_enable)
		{
			// use the playfield 2 y-offset table for each scanline
			if (m_video_control_data[6] & 0x40)
			{
				int line;
				rectangle clip;
				clip.min_x = cliprect.min_x;
				clip.max_x = cliprect.max_x;

				for(line = 0; line < 512; line++)
				{
					clip.min_y = clip.max_y = line;

					if (m_video_control_data[6] & 0x4)
					{
						m_pf2_wide_layer->set_scrolly(0, 0x200 + m_video_data[0xfc00/2 + line]);
						m_pf2_wide_layer->draw(screen, bitmap, clip, 0,0);
						m_pf2_wide_layer->draw(screen, bitmap, clip, 1,1);
					} else {
						m_pf2_layer->set_scrolly(0, 0x200 + m_video_data[0xfc00/2 + line]);
						m_pf2_layer->draw(screen, bitmap, clip, 0,0);
						m_pf2_layer->draw(screen, bitmap, clip, 1,1);
					}
				}
			}
			else
			{
				if (m_video_control_data[6] & 0x4)
				{
					m_pf2_wide_layer->set_scrolly(0, m_video_control_data[2] );
					m_pf2_wide_layer->draw(screen, bitmap, cliprect, 0,0);
					m_pf2_wide_layer->draw(screen, bitmap, cliprect, 1,1);
				} else {
					m_pf2_layer->set_scrolly(0, m_video_control_data[2] );
					m_pf2_layer->draw(screen, bitmap, cliprect, 0,0);
					m_pf2_layer->draw(screen, bitmap, cliprect, 1,1);
				}
			}
		}
		else
		{
			bitmap.fill(0, cliprect);
		}

		if (pf1_enable)
		{
			// use the playfield 1 y-offset table for each scanline
			if (m_video_control_data[5] & 0x40)
			{
				int line;
				rectangle clip;
				clip.min_x = cliprect.min_x;
				clip.max_x = cliprect.max_x;

				for(line = 0; line < 512; line++)
				{
					clip.min_y = clip.max_y = line;

					if (m_video_control_data[5] & 0x4)
					{
						m_pf1_wide_layer->set_scrolly(0, 0x200 + m_video_data[0xf800/2 + line]);
						m_pf1_wide_layer->draw(screen, bitmap, clip, 0,0);
						m_pf1_wide_layer->draw(screen, bitmap, clip, 1,1);
					} else {
						m_pf1_layer->set_scrolly(0, 0x200 + m_video_data[0xf800/2 + line]);
						m_pf1_layer->draw(screen, bitmap, clip, 0,0);
						m_pf1_layer->draw(screen, bitmap, clip, 1,1);
					}
				}
			}
			else
			{
				if (m_video_control_data[5] & 0x4)
				{
					m_pf1_wide_layer->set_scrolly(0, m_video_control_data[0] );
					m_pf1_wide_layer->draw(screen, bitmap, cliprect, 0,0);
					m_pf1_wide_layer->draw(screen, bitmap, cliprect, 1,1);
				} else {
					m_pf1_layer->set_scrolly(0, m_video_control_data[0] );
					m_pf1_layer->draw(screen, bitmap, cliprect, 0,0);
					m_pf1_layer->draw(screen, bitmap, cliprect, 1,1);
				}
			}
		}

		draw_sprites(screen,bitmap,cliprect);

	} else {
		bitmap.fill(m_palette->black_pen(), cliprect);
	}

	return 0;
}

UINT32 m90_state::screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	/* Setup scrolling */
	if (m_video_control_data[6]&0x20) {
		m_pf1_layer->set_scroll_rows(512);
		m_pf1_wide_layer->set_scroll_rows(512);
		for (i=0; i<512; i++)
			m_pf1_layer->set_scrollx(i, m_video_data[0xf400/2+i]-12);
		for (i=0; i<512; i++)
			m_pf1_wide_layer->set_scrollx(i, m_video_data[0xf400/2+i]-12+256);
	} else {
		m_pf1_layer->set_scroll_rows(1);
		m_pf1_wide_layer->set_scroll_rows(1);
		m_pf1_layer->set_scrollx(0,  m_video_data[0xf004/2]-12);
		m_pf1_wide_layer->set_scrollx(0, m_video_data[0xf004/2]-12 );
	}

	if (m_video_control_data[6] & 0x02) {
		m_pf2_wide_layer->mark_all_dirty();
		m_pf2_wide_layer->set_scrollx(0, m_video_data[0xf000/2]-16 );
		m_pf2_wide_layer->set_scrolly(0, m_video_data[0xf008/2]+388 );
		m_pf2_wide_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf2_wide_layer->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf2_layer->mark_all_dirty();
		m_pf2_layer->set_scrollx(0, m_video_data[0xf000/2]-16 );
		m_pf2_layer->set_scrolly(0, m_video_data[0xf008/2]-120 );
		m_pf2_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf2_layer->draw(screen, bitmap, cliprect, 1,1);
	}

	if (m_video_control_data[6] & 0x04) {
		m_pf1_wide_layer->mark_all_dirty();
		m_pf1_wide_layer->set_scrolly(0, m_video_data[0xf00c/2]+392 );
		m_pf1_wide_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf1_wide_layer->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf1_layer->mark_all_dirty();
		m_pf1_layer->set_scrolly(0, m_video_data[0xf00c/2]-116 );
		m_pf1_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf1_layer->draw(screen, bitmap, cliprect, 1,1);
	}

	bomblord_draw_sprites(screen,bitmap,cliprect);

	return 0;
}

UINT32 m90_state::screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_video_data[0xf008/2] & 0x4000)) {
		m_pf1_wide_layer->mark_all_dirty();
		m_pf1_wide_layer->set_scroll_rows(1);
		m_pf1_wide_layer->set_scrollx(0, m_video_data[0xf004/2]+64);
		m_pf1_wide_layer->set_scrolly(0, m_video_data[0xf006/2]+512);
		m_pf1_wide_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf1_wide_layer->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf1_layer->mark_all_dirty();
		m_pf1_layer->set_scroll_rows(1);
		m_pf1_layer->set_scrollx(0, m_video_data[0xf004/2]+64);
		m_pf1_layer->set_scrolly(0, m_video_data[0xf006/2]+4);
		m_pf1_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf1_layer->draw(screen, bitmap, cliprect, 1,1);
	}

	if (!(m_video_data[0xf008/2] & 0x8000)) {
		m_pf2_wide_layer->mark_all_dirty();
		m_pf2_wide_layer->set_scroll_rows(1);
		m_pf2_wide_layer->set_scrollx(0, m_video_data[0xf000/2]+68);
		m_pf2_wide_layer->set_scrolly(0, m_video_data[0xf002/2]+512);
		m_pf2_wide_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf2_wide_layer->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf2_layer->mark_all_dirty();
		m_pf2_layer->set_scroll_rows(1);
		m_pf2_layer->set_scrollx(0, m_video_data[0xf000/2]+68);
		m_pf2_layer->set_scrolly(0, m_video_data[0xf002/2]+4);
		m_pf2_layer->draw(screen, bitmap, cliprect, 0,0);
		m_pf2_layer->draw(screen, bitmap, cliprect, 1,1);
	}

	dynablsb_draw_sprites(screen,bitmap,cliprect);

	return 0;
}
