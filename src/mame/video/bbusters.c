// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Emulation by Bryan McPhail, mish@tendril.co.uk

    Like NeoGeo sprite scale Y line selection is from an external rom.

    For 16 high sprites scale data starts at 0x3800 (9 scale levels)
    For 32 high sprites scale data starts at 0x7000 (17 scale levels)
    For 64 high sprites scale data starts at 0xa000 (33 scale levels)
    For 128 pixel high sprites scale data starts 0xc000 (65 scale levels)

    0xe000 and up - possibly X scale data?  unconfirmed

    Sprites are also double buffered, and this seems to be performed
    by having two complete sprite chips that are toggled per frame, rather
    than just ram.  Beast Busters has 4 sprite chips as it has two sprite
    banks.

    Todo: Sprite priority looks to be wrong on level 2 (some sprites should
    be behind the playfield).

***************************************************************************/

#include "emu.h"
#include "includes/bbusters.h"


/******************************************************************************/

TILE_GET_INFO_MEMBER(bbusters_state::get_tile_info)
{
	UINT16 tile = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0,tile&0xfff,tile>>12,0);
}

TILE_GET_INFO_MEMBER(bbusters_state::get_pf1_tile_info)
{
	UINT16 tile = m_pf1_data[tile_index];

	SET_TILE_INFO_MEMBER(3,tile&0xfff,tile>>12,0);
}

TILE_GET_INFO_MEMBER(bbusters_state::get_pf2_tile_info)
{
	UINT16 tile = m_pf2_data[tile_index];

	SET_TILE_INFO_MEMBER(4,tile&0xfff,tile>>12,0);
}

WRITE16_MEMBER(bbusters_state::video_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(bbusters_state::pf1_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(bbusters_state::pf2_w)
{
	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset);
}

/******************************************************************************/

VIDEO_START_MEMBER(bbusters_state,bbuster)
{
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_pf1_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 128, 32);
	m_pf2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_pf2_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 128, 32);

	m_pf1_tilemap->set_transparent_pen(15);
	m_fix_tilemap->set_transparent_pen(15);

	save_item(NAME(m_scale_line_count));
}

VIDEO_START_MEMBER(bbusters_state,mechatt)
{
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_pf1_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 256, 32);
	m_pf2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bbusters_state::get_pf2_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 256, 32);

	m_pf1_tilemap->set_transparent_pen(15);
	m_fix_tilemap->set_transparent_pen(15);

	save_item(NAME(m_scale_line_count));
}

/******************************************************************************/

#define ADJUST_4x4 \
		if ((dx&0x10) && (dy&0x10)) code+=3;    \
		else if (dy&0x10) code+=2;              \
		else if (dx&0x10) code+=1

#define ADJUST_8x8 \
		if ((dx&0x20) && (dy&0x20)) code+=12;   \
		else if (dy&0x20) code+=8;              \
		else if (dx&0x20) code+=4

#define ADJUST_16x16 \
		if ((dx&0x40) && (dy&0x40)) code+=48;   \
		else if (dy&0x40) code+=32;             \
		else if (dx&0x40) code+=16

inline const UINT8 *bbusters_state::get_source_ptr(gfx_element *gfx, UINT32 sprite, int dx, int dy, int block)
{
	int code=0;

	/* Get a tile index from the x,y position in the block */
	switch (block)
	{
	case 0: /* 16 x 16 sprite */
		break;

	case 1: /* 32 x 32 block
                0 1
                2 3
            */
		ADJUST_4x4;
		break;

	case 2: /* 64 by 64 block
                0  1    4  5
                2  3    6  7

                8  9    12 13
                10 11   14 15
            */
		ADJUST_4x4;
		ADJUST_8x8;
		break;

	case 3: /* 128 by 128 block */
		ADJUST_4x4;
		ADJUST_8x8;
		ADJUST_16x16;
		break;
	}

	return gfx->get_data((sprite+code) % gfx->elements()) + ((dy%16) * gfx->rowbytes());
}

void bbusters_state::draw_block(bitmap_ind16 &dest,int x,int y,int size,int flipx,int flipy,UINT32 sprite,int color,int bank,int block)
{
	gfx_element *gfx = m_gfxdecode->gfx(bank);
	pen_t pen_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	UINT32 xinc=(m_scale_line_count * 0x10000 ) / size;
	UINT8 pixel;
	int x_index;
	int dy=y;
	int sx, ex = m_scale_line_count;

	while (m_scale_line_count) {
		if (dy>=16 && dy<240) {
			UINT16 *destline = &dest.pix16(dy);
			UINT8 srcline=*m_scale_table_ptr;
			const UINT8 *srcptr=0;

			if (!flipy)
				srcline=size-srcline-1;

			if (flipx)
				x_index=(ex-1)*0x10000;
			else
				x_index=0;

			for (sx=0; sx<size; sx++) {
				if ((sx%16)==0)
					srcptr=get_source_ptr(gfx,sprite,sx,srcline,block);

				pixel=*srcptr++;
				if (pixel!=15)
					destline[(x+(x_index>>16)) & 0x1ff]= pen_base + pixel;

				if (flipx)
					x_index-=xinc;
				else
					x_index+=xinc;
			}
		}

		dy++;
		m_scale_table_ptr--;
		m_scale_line_count--;
	}
}

void bbusters_state::draw_sprites(bitmap_ind16 &bitmap, const UINT16 *source, int bank, int colval, int colmask)
{
	const UINT8 *scale_table=memregion("user1")->base();
	int offs;

	for (offs = 0;offs <0x800 ;offs += 4) {
		int x,y,sprite,colour,fx,fy,scale;
		int block;

		sprite=source[offs+1];
		colour=source[offs+0];

		if (colour==0xf7 && (sprite==0x3fff || sprite==0xffff))
			continue;

		y=source[offs+3];
		x=source[offs+2];
		if (x&0x200) x=-(0x100-(x&0xff));

		/*
		    Source[0]:
		        0xf000: Colour
		        0x0800: FX
		        0x0400: FY?
		        0x0300: Block control
		        0x0080: ?
		        0x007f: scale

		    Scale varies according to block size.
		    Block type 0: 0x70 = no scale, 0x7f == half size - 16 pixel sprite
		    Block type 1: 0x60 = no scale, 0x6f == half size - 32 pixel sprite
		    Block type 2: 0x40 = no scale, 0x5f == half size - 64 pixel sprite
		    Block type 3: 0x00 = no scale, 0x3f == half size - 128 pixel sprite

		*/
		colour=colour>>12;
		block=(source[offs+0]>>8)&0x3;
		fy=source[offs+0]&0x400;
		fx=source[offs+0]&0x800;
		sprite=sprite&0x3fff;

		if ((colour&colmask)!=colval)
			continue;

		switch ((source[offs+0]>>8)&0x3) {
			case 0:
				scale=source[offs+0]&0x7;
				m_scale_table_ptr = scale_table+0x387f+(0x80*scale);
				m_scale_line_count = 0x10-scale;
				draw_block(bitmap,x,y,16,fx,fy,sprite,colour,bank,block);
				break;
			case 1: /* 2 x 2 */
				scale=source[offs+0]&0xf;
				m_scale_table_ptr = scale_table+0x707f+(0x80*scale);
				m_scale_line_count = 0x20-scale;
				draw_block(bitmap,x,y,32,fx,fy,sprite,colour,bank,block);
				break;
			case 2: /* 64 by 64 block (2 x 2) x 2 */
				scale=source[offs+0]&0x1f;
				m_scale_table_ptr = scale_table+0xa07f+(0x80*scale);
				m_scale_line_count = 0x40-scale;
				draw_block(bitmap,x,y,64,fx,fy,sprite,colour,bank,block);
				break;
			case 3: /* 2 x 2 x 2 x 2 */
				scale=source[offs+0]&0x3f;
				m_scale_table_ptr = scale_table+0xc07f+(0x80*scale);
				m_scale_line_count = 0x80-scale;
				draw_block(bitmap,x,y,128,fx,fy,sprite,colour,bank,block);
				break;
		}
	}
}

/******************************************************************************/

UINT32 bbusters_state::screen_update_bbuster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->set_scrollx(0, m_pf1_scroll_data[0]);
	m_pf1_tilemap->set_scrolly(0, m_pf1_scroll_data[1]);
	m_pf2_tilemap->set_scrollx(0, m_pf2_scroll_data[0]);
	m_pf2_tilemap->set_scrolly(0, m_pf2_scroll_data[1]);

	m_pf2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	//draw_sprites(machine(), bitmap, m_spriteram2->buffer(), 2, 0x8, 0x8);
	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, m_spriteram2->buffer(), 2, 0, 0);
	draw_sprites(bitmap, m_spriteram->buffer(), 1, 0, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 bbusters_state::screen_update_mechatt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->set_scrollx(0, m_pf1_scroll_data[0]);
	m_pf1_tilemap->set_scrolly(0, m_pf1_scroll_data[1]);
	m_pf2_tilemap->set_scrollx(0, m_pf2_scroll_data[0]);
	m_pf2_tilemap->set_scrolly(0, m_pf2_scroll_data[1]);

	m_pf2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, m_spriteram->buffer(), 1, 0, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
