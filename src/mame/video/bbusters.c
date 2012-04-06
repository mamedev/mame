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

static TILE_GET_INFO( get_bbusters_tile_info )
{
	bbusters_state *state = machine.driver_data<bbusters_state>();
	UINT16 tile = state->m_videoram[tile_index];

	SET_TILE_INFO(0,tile&0xfff,tile>>12,0);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	bbusters_state *state = machine.driver_data<bbusters_state>();
	UINT16 tile = state->m_pf1_data[tile_index];

	SET_TILE_INFO(3,tile&0xfff,tile>>12,0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	bbusters_state *state = machine.driver_data<bbusters_state>();
	UINT16 tile = state->m_pf2_data[tile_index];

	SET_TILE_INFO(4,tile&0xfff,tile>>12,0);
}

WRITE16_MEMBER(bbusters_state::bbusters_video_w)
{

	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(bbusters_state::bbusters_pf1_w)
{

	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(bbusters_state::bbusters_pf2_w)
{

	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset);
}

/******************************************************************************/

VIDEO_START( bbuster )
{
	bbusters_state *state = machine.driver_data<bbusters_state>();

	state->m_fix_tilemap = tilemap_create(machine, get_bbusters_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_pf1_tilemap = tilemap_create(machine, get_pf1_tile_info, tilemap_scan_cols, 16, 16, 128, 32);
	state->m_pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, tilemap_scan_cols, 16, 16, 128, 32);

	state->m_pf1_tilemap->set_transparent_pen(15);
	state->m_fix_tilemap->set_transparent_pen(15);
}

VIDEO_START( mechatt )
{
	bbusters_state *state = machine.driver_data<bbusters_state>();

	state->m_fix_tilemap = tilemap_create(machine, get_bbusters_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_pf1_tilemap = tilemap_create(machine, get_pf1_tile_info, tilemap_scan_cols, 16, 16, 256, 32);
	state->m_pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, tilemap_scan_cols, 16, 16, 256, 32);

	state->m_pf1_tilemap->set_transparent_pen(15);
	state->m_fix_tilemap->set_transparent_pen(15);
}

/******************************************************************************/

#define ADJUST_4x4 \
		if ((dx&0x10) && (dy&0x10)) code+=3;	\
		else if (dy&0x10) code+=2;				\
		else if (dx&0x10) code+=1

#define ADJUST_8x8 \
		if ((dx&0x20) && (dy&0x20)) code+=12;	\
		else if (dy&0x20) code+=8;				\
		else if (dx&0x20) code+=4

#define ADJUST_16x16 \
		if ((dx&0x40) && (dy&0x40)) code+=48;	\
		else if (dy&0x40) code+=32;				\
		else if (dx&0x40) code+=16

INLINE const UINT8 *get_source_ptr(gfx_element *gfx, UINT32 sprite, int dx, int dy, int block)
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

	return gfx_element_get_data(gfx, (sprite+code) % gfx->total_elements) + ((dy%16) * gfx->line_modulo);
}

static void bbusters_draw_block(running_machine &machine, bitmap_ind16 &dest,int x,int y,int size,int flipx,int flipy,UINT32 sprite,int color,int bank,int block)
{
	bbusters_state *state = machine.driver_data<bbusters_state>();
	gfx_element *gfx = machine.gfx[bank];
	pen_t pen_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	UINT32 xinc=(state->m_scale_line_count * 0x10000 ) / size;
	UINT8 pixel;
	int x_index;
	int dy=y;
	int sx, ex = state->m_scale_line_count;

	while (state->m_scale_line_count) {

		if (dy>=16 && dy<240) {
			UINT16 *destline = &dest.pix16(dy);
			UINT8 srcline=*state->m_scale_table_ptr;
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
		state->m_scale_table_ptr--;
		state->m_scale_line_count--;
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const UINT16 *source, int bank, int colval, int colmask)
{
	bbusters_state *state = machine.driver_data<bbusters_state>();
	const UINT8 *scale_table=machine.region("user1")->base();
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
				state->m_scale_table_ptr = scale_table+0x387f+(0x80*scale);
				state->m_scale_line_count = 0x10-scale;
				bbusters_draw_block(machine,bitmap,x,y,16,fx,fy,sprite,colour,bank,block);
				break;
			case 1: /* 2 x 2 */
				scale=source[offs+0]&0xf;
				state->m_scale_table_ptr = scale_table+0x707f+(0x80*scale);
				state->m_scale_line_count = 0x20-scale;
				bbusters_draw_block(machine,bitmap,x,y,32,fx,fy,sprite,colour,bank,block);
				break;
			case 2: /* 64 by 64 block (2 x 2) x 2 */
				scale=source[offs+0]&0x1f;
				state->m_scale_table_ptr = scale_table+0xa07f+(0x80*scale);
				state->m_scale_line_count = 0x40-scale;
				bbusters_draw_block(machine,bitmap,x,y,64,fx,fy,sprite,colour,bank,block);
				break;
			case 3: /* 2 x 2 x 2 x 2 */
				scale=source[offs+0]&0x3f;
				state->m_scale_table_ptr = scale_table+0xc07f+(0x80*scale);
				state->m_scale_line_count = 0x80-scale;
				bbusters_draw_block(machine,bitmap,x,y,128,fx,fy,sprite,colour,bank,block);
				break;
		}
	}
}

/******************************************************************************/

SCREEN_UPDATE_IND16( bbuster )
{
	bbusters_state *state = screen.machine().driver_data<bbusters_state>();

	state->m_pf1_tilemap->set_scrollx(0, state->m_pf1_scroll_data[0]);
	state->m_pf1_tilemap->set_scrolly(0, state->m_pf1_scroll_data[1]);
	state->m_pf2_tilemap->set_scrollx(0, state->m_pf2_scroll_data[0]);
	state->m_pf2_tilemap->set_scrolly(0, state->m_pf2_scroll_data[1]);

	state->m_pf2_tilemap->draw(bitmap, cliprect, 0, 0);
	//draw_sprites(screen.machine(), bitmap, state->m_spriteram2->buffer(), 2, 0x8, 0x8);
	state->m_pf1_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, state->m_spriteram2->buffer(), 2, 0, 0);
	draw_sprites(screen.machine(), bitmap, state->m_spriteram->buffer(), 1, 0, 0);
	state->m_fix_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( mechatt )
{
	bbusters_state *state = screen.machine().driver_data<bbusters_state>();

	state->m_pf1_tilemap->set_scrollx(0, state->m_pf1_scroll_data[0]);
	state->m_pf1_tilemap->set_scrolly(0, state->m_pf1_scroll_data[1]);
	state->m_pf2_tilemap->set_scrollx(0, state->m_pf2_scroll_data[0]);
	state->m_pf2_tilemap->set_scrolly(0, state->m_pf2_scroll_data[1]);

	state->m_pf2_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_pf1_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, state->m_spriteram->buffer(), 1, 0, 0);
	state->m_fix_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
