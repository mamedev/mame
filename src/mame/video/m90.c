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


INLINE void get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,int layer,int page_mask)
{
	m90_state *state = machine.driver_data<m90_state>();
	int tile,color;
	tile_index = 2*tile_index + ((state->m_video_control_data[5+layer] & page_mask) * 0x2000);

	tile=state->m_video_data[tile_index];
	color=state->m_video_data[tile_index+1];
	SET_TILE_INFO(
			0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

INLINE void bomblord_get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,int layer)
{
	m90_state *state = machine.driver_data<m90_state>();
	int tile,color;
	tile_index = 2*tile_index + (layer * 0x2000);

	tile=state->m_video_data[tile_index];
	color=state->m_video_data[tile_index+1];
	SET_TILE_INFO(
			0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

INLINE void dynablsb_get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,int layer)
{
	m90_state *state = machine.driver_data<m90_state>();
	int tile,color;
	tile_index = 2*tile_index + (layer * 0x2000);

	tile=state->m_video_data[tile_index];
	color=state->m_video_data[tile_index+1];
	SET_TILE_INFO(
			0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

static TILE_GET_INFO( get_pf1_tile_info ) { get_tile_info(machine,tileinfo,tile_index,0,0x3); }
static TILE_GET_INFO( get_pf1w_tile_info ) { get_tile_info(machine,tileinfo,tile_index,0,0x2); }
static TILE_GET_INFO( get_pf2_tile_info ) { get_tile_info(machine,tileinfo,tile_index,1,0x3); }
static TILE_GET_INFO( get_pf2w_tile_info ) { get_tile_info(machine,tileinfo,tile_index,1,0x2); }

static TILE_GET_INFO( bomblord_get_pf1_tile_info ) { bomblord_get_tile_info(machine,tileinfo,tile_index,0); }
static TILE_GET_INFO( bomblord_get_pf1w_tile_info ) { bomblord_get_tile_info(machine,tileinfo,tile_index,0); }
static TILE_GET_INFO( bomblord_get_pf2_tile_info ) { bomblord_get_tile_info(machine,tileinfo,tile_index,2); }
static TILE_GET_INFO( bomblord_get_pf2w_tile_info ) { bomblord_get_tile_info(machine,tileinfo,tile_index,2); }

static TILE_GET_INFO( dynablsb_get_pf1_tile_info ) { dynablsb_get_tile_info(machine,tileinfo,tile_index,0); }
static TILE_GET_INFO( dynablsb_get_pf1w_tile_info ) { dynablsb_get_tile_info(machine,tileinfo,tile_index,0); }
static TILE_GET_INFO( dynablsb_get_pf2_tile_info ) { dynablsb_get_tile_info(machine,tileinfo,tile_index,2); }
static TILE_GET_INFO( dynablsb_get_pf2w_tile_info ) { dynablsb_get_tile_info(machine,tileinfo,tile_index,2); }

VIDEO_START( m90 )
{
	m90_state *state = machine.driver_data<m90_state>();
	state->m_pf1_layer =      tilemap_create(machine, get_pf1_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf1_wide_layer = tilemap_create(machine, get_pf1w_tile_info,tilemap_scan_rows,8,8,128,64);
	state->m_pf2_layer =      tilemap_create(machine, get_pf2_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf2_wide_layer = tilemap_create(machine, get_pf2w_tile_info,tilemap_scan_rows,8,8,128,64);

	state->m_pf1_layer->set_transparent_pen(0);
	state->m_pf1_wide_layer->set_transparent_pen(0);

	state_save_register_global_array(machine, state->m_video_control_data);
	state_save_register_global(machine, state->m_last_pf1);
	state_save_register_global(machine, state->m_last_pf2);
}

VIDEO_START( bomblord )
{
	m90_state *state = machine.driver_data<m90_state>();
	state->m_pf1_layer =      tilemap_create(machine, bomblord_get_pf1_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf1_wide_layer = tilemap_create(machine, bomblord_get_pf1w_tile_info,tilemap_scan_rows,8,8,128,64);
	state->m_pf2_layer =      tilemap_create(machine, bomblord_get_pf2_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf2_wide_layer = tilemap_create(machine, bomblord_get_pf2w_tile_info,tilemap_scan_rows,8,8,128,64);

	state->m_pf2_layer->set_transparent_pen(0);
	state->m_pf2_wide_layer->set_transparent_pen(0);
	state->m_pf1_layer->set_transparent_pen(0);
	state->m_pf1_wide_layer->set_transparent_pen(0);

	state_save_register_global_array(machine, state->m_video_control_data);
}

VIDEO_START( dynablsb )
{
	m90_state *state = machine.driver_data<m90_state>();
	state->m_pf1_layer =      tilemap_create(machine, dynablsb_get_pf1_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf1_wide_layer = tilemap_create(machine, dynablsb_get_pf1w_tile_info,tilemap_scan_rows,8,8,128,64);
	state->m_pf2_layer =      tilemap_create(machine, dynablsb_get_pf2_tile_info, tilemap_scan_rows,8,8,64,64);
	state->m_pf2_wide_layer = tilemap_create(machine, dynablsb_get_pf2w_tile_info,tilemap_scan_rows,8,8,128,64);

	state->m_pf2_layer->set_transparent_pen(0);
	state->m_pf2_wide_layer->set_transparent_pen(0);

	state_save_register_global_array(machine, state->m_video_control_data);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	m90_state *state = machine.driver_data<m90_state>();
	UINT16 *spriteram = state->m_video_data + 0xee00/2;;
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

			if (state->m_video_control_data[7] & 0x01)
				pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					machine.priority_bitmap,
					(colour & 0x08) ? 0x00 : 0x02,0);
			else if (state->m_video_control_data[7] & 0x02)
				pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					machine.priority_bitmap,
					((colour & 0x0c)==0x0c) ? 0x00 : 0x02,0);
			else
				pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					machine.priority_bitmap,
					0x02,0);
	}
}

static void bomblord_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	m90_state *state = machine.driver_data<m90_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;


	while ((offs < state->m_spriteram_size/2) & (spriteram16[offs+0] != 0x8000))
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

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite,
				colour,
				fx,fy,
				x,y,
				machine.priority_bitmap,
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

static void dynablsb_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	m90_state *state = machine.driver_data<m90_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;

	while ((offs < state->m_spriteram_size/2) & (spriteram16[offs+0] != 0xffff))
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

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite,
				colour,
				fx,fy,
				x,y,
				machine.priority_bitmap,
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

WRITE16_MEMBER(m90_state::m90_video_control_w)
{
	COMBINE_DATA(&m_video_control_data[offset]);
}

static void markdirty(tilemap_t *tmap,int page,offs_t offset)
{
	offset -= page * 0x2000;

	if (offset >= 0 && offset < 0x2000)
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

SCREEN_UPDATE_IND16( m90 )
{
	m90_state *state = screen.machine().driver_data<m90_state>();
	UINT8 pf1_base = state->m_video_control_data[5] & 0x3;
	UINT8 pf2_base = state->m_video_control_data[6] & 0x3;
	int i,pf1_enable,pf2_enable, video_enable;

	if (state->m_video_control_data[7]&0x04) video_enable=0; else video_enable=1;
	if (state->m_video_control_data[5]&0x10) pf1_enable=0; else pf1_enable=1;
	if (state->m_video_control_data[6]&0x10) pf2_enable=0; else pf2_enable=1;

// state->m_pf1_layer->enable(pf1_enable);
// state->m_pf2_layer->enable(pf2_enable);
// state->m_pf1_wide_layer->enable(pf1_enable);
// state->m_pf2_wide_layer->enable(pf2_enable);

	/* Dirty tilemaps if VRAM base changes */
	if (pf1_base!=state->m_last_pf1)
	{
		state->m_pf1_layer->mark_all_dirty();
		state->m_pf1_wide_layer->mark_all_dirty();
	}
	if (pf2_base!=state->m_last_pf2)
	{
		state->m_pf2_layer->mark_all_dirty();
		state->m_pf2_wide_layer->mark_all_dirty();
	}
	state->m_last_pf1=pf1_base;
	state->m_last_pf2=pf2_base;

	/* Setup scrolling */
	if (state->m_video_control_data[5]&0x20)
	{
		state->m_pf1_layer->set_scroll_rows(512);
		state->m_pf1_wide_layer->set_scroll_rows(512);

		for (i=0; i<512; i++)
			state->m_pf1_layer->set_scrollx(i, state->m_video_data[0xf000/2+i]+2);
		for (i=0; i<512; i++)
			state->m_pf1_wide_layer->set_scrollx(i, state->m_video_data[0xf000/2+i]+256+2);

	}
	else
	{
		state->m_pf1_layer->set_scroll_rows(1);
		state->m_pf1_wide_layer->set_scroll_rows(1);
		state->m_pf1_layer->set_scrollx(0, state->m_video_control_data[1]+2);
		state->m_pf1_wide_layer->set_scrollx(0, state->m_video_control_data[1]+256+2);
	}

	/* Setup scrolling */
	if (state->m_video_control_data[6]&0x20)
	{
		state->m_pf2_layer->set_scroll_rows(512);
		state->m_pf2_wide_layer->set_scroll_rows(512);
		for (i=0; i<512; i++)
			state->m_pf2_layer->set_scrollx(i, state->m_video_data[0xf400/2+i]-2);
		for (i=0; i<512; i++)
			state->m_pf2_wide_layer->set_scrollx(i, state->m_video_data[0xf400/2+i]+256-2);
	} else {
		state->m_pf2_layer->set_scroll_rows(1);
		state->m_pf2_wide_layer->set_scroll_rows(1);
		state->m_pf2_layer->set_scrollx(0, state->m_video_control_data[3]-2);
		state->m_pf2_wide_layer->set_scrollx(0, state->m_video_control_data[3]+256-2 );
	}

	screen.machine().priority_bitmap.fill(0, cliprect);

	if (video_enable)
	{

		if (pf2_enable)
		{
			// use the playfield 2 y-offset table for each scanline
			if (state->m_video_control_data[6] & 0x40)
			{
				int line;
				rectangle clip;
				clip.min_x = cliprect.min_x;
				clip.max_x = cliprect.max_x;

				for(line = 0; line < 512; line++)
				{
					clip.min_y = clip.max_y = line;

					if (state->m_video_control_data[6] & 0x4)
					{
						state->m_pf2_wide_layer->set_scrolly(0, 0x200 + state->m_video_data[0xfc00/2 + line]);
						state->m_pf2_wide_layer->draw(bitmap, clip, 0,0);
						state->m_pf2_wide_layer->draw(bitmap, clip, 1,1);
					} else {
						state->m_pf2_layer->set_scrolly(0, 0x200 + state->m_video_data[0xfc00/2 + line]);
						state->m_pf2_layer->draw(bitmap, clip, 0,0);
						state->m_pf2_layer->draw(bitmap, clip, 1,1);
					}
				}
			}
			else
			{
				if (state->m_video_control_data[6] & 0x4)
				{
					state->m_pf2_wide_layer->set_scrolly(0, state->m_video_control_data[2] );
					state->m_pf2_wide_layer->draw(bitmap, cliprect, 0,0);
					state->m_pf2_wide_layer->draw(bitmap, cliprect, 1,1);
				} else {
					state->m_pf2_layer->set_scrolly(0, state->m_video_control_data[2] );
					state->m_pf2_layer->draw(bitmap, cliprect, 0,0);
					state->m_pf2_layer->draw(bitmap, cliprect, 1,1);
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
			if (state->m_video_control_data[5] & 0x40)
			{
				int line;
				rectangle clip;
				clip.min_x = cliprect.min_x;
				clip.max_x = cliprect.max_x;

				for(line = 0; line < 512; line++)
				{
					clip.min_y = clip.max_y = line;

					if (state->m_video_control_data[5] & 0x4)
					{
						state->m_pf1_wide_layer->set_scrolly(0, 0x200 + state->m_video_data[0xf800/2 + line]);
						state->m_pf1_wide_layer->draw(bitmap, clip, 0,0);
						state->m_pf1_wide_layer->draw(bitmap, clip, 1,1);
					} else {
						state->m_pf1_layer->set_scrolly(0, 0x200 + state->m_video_data[0xf800/2 + line]);
						state->m_pf1_layer->draw(bitmap, clip, 0,0);
						state->m_pf1_layer->draw(bitmap, clip, 1,1);
					}
				}
			}
			else
			{
				if (state->m_video_control_data[5] & 0x4)
				{
					state->m_pf1_wide_layer->set_scrolly(0, state->m_video_control_data[0] );
					state->m_pf1_wide_layer->draw(bitmap, cliprect, 0,0);
					state->m_pf1_wide_layer->draw(bitmap, cliprect, 1,1);
				} else {
					state->m_pf1_layer->set_scrolly(0, state->m_video_control_data[0] );
					state->m_pf1_layer->draw(bitmap, cliprect, 0,0);
					state->m_pf1_layer->draw(bitmap, cliprect, 1,1);
				}
			}
		}

		draw_sprites(screen.machine(),bitmap,cliprect);

	} else {
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
	}

	return 0;
}

SCREEN_UPDATE_IND16( bomblord )
{
	m90_state *state = screen.machine().driver_data<m90_state>();
	int i;
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	/* Setup scrolling */
	if (state->m_video_control_data[6]&0x20) {
		state->m_pf1_layer->set_scroll_rows(512);
		state->m_pf1_wide_layer->set_scroll_rows(512);
		for (i=0; i<512; i++)
			state->m_pf1_layer->set_scrollx(i, state->m_video_data[0xf400/2+i]-12);
		for (i=0; i<512; i++)
			state->m_pf1_wide_layer->set_scrollx(i, state->m_video_data[0xf400/2+i]-12+256);
	} else {
		state->m_pf1_layer->set_scroll_rows(1);
		state->m_pf1_wide_layer->set_scroll_rows(1);
		state->m_pf1_layer->set_scrollx(0,  state->m_video_data[0xf004/2]-12);
		state->m_pf1_wide_layer->set_scrollx(0, state->m_video_data[0xf004/2]-12 );
	}

	if (state->m_video_control_data[6] & 0x02) {
		state->m_pf2_wide_layer->mark_all_dirty();
		state->m_pf2_wide_layer->set_scrollx(0, state->m_video_data[0xf000/2]-16 );
		state->m_pf2_wide_layer->set_scrolly(0, state->m_video_data[0xf008/2]+388 );
		state->m_pf2_wide_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf2_wide_layer->draw(bitmap, cliprect, 1,1);
	} else {
		state->m_pf2_layer->mark_all_dirty();
		state->m_pf2_layer->set_scrollx(0, state->m_video_data[0xf000/2]-16 );
		state->m_pf2_layer->set_scrolly(0, state->m_video_data[0xf008/2]-120 );
		state->m_pf2_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf2_layer->draw(bitmap, cliprect, 1,1);
	}

	if (state->m_video_control_data[6] & 0x04) {
		state->m_pf1_wide_layer->mark_all_dirty();
		state->m_pf1_wide_layer->set_scrolly(0, state->m_video_data[0xf00c/2]+392 );
		state->m_pf1_wide_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf1_wide_layer->draw(bitmap, cliprect, 1,1);
	} else {
		state->m_pf1_layer->mark_all_dirty();
		state->m_pf1_layer->set_scrolly(0, state->m_video_data[0xf00c/2]-116 );
		state->m_pf1_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf1_layer->draw(bitmap, cliprect, 1,1);
	}

	bomblord_draw_sprites(screen.machine(),bitmap,cliprect);

	return 0;
}

SCREEN_UPDATE_IND16( dynablsb )
{
	m90_state *state = screen.machine().driver_data<m90_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!(state->m_video_data[0xf008/2] & 0x4000)) {
		state->m_pf1_wide_layer->mark_all_dirty();
		state->m_pf1_wide_layer->set_scroll_rows(1);
		state->m_pf1_wide_layer->set_scrollx(0, state->m_video_data[0xf004/2]+64);
		state->m_pf1_wide_layer->set_scrolly(0, state->m_video_data[0xf006/2]+512);
		state->m_pf1_wide_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf1_wide_layer->draw(bitmap, cliprect, 1,1);
	} else {
		state->m_pf1_layer->mark_all_dirty();
		state->m_pf1_layer->set_scroll_rows(1);
		state->m_pf1_layer->set_scrollx(0, state->m_video_data[0xf004/2]+64);
		state->m_pf1_layer->set_scrolly(0, state->m_video_data[0xf006/2]+4);
		state->m_pf1_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf1_layer->draw(bitmap, cliprect, 1,1);
	}

	if (!(state->m_video_data[0xf008/2] & 0x8000)) {
		state->m_pf2_wide_layer->mark_all_dirty();
		state->m_pf2_wide_layer->set_scroll_rows(1);
		state->m_pf2_wide_layer->set_scrollx(0, state->m_video_data[0xf000/2]+68);
		state->m_pf2_wide_layer->set_scrolly(0, state->m_video_data[0xf002/2]+512);
		state->m_pf2_wide_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf2_wide_layer->draw(bitmap, cliprect, 1,1);
	} else {
		state->m_pf2_layer->mark_all_dirty();
		state->m_pf2_layer->set_scroll_rows(1);
		state->m_pf2_layer->set_scrollx(0, state->m_video_data[0xf000/2]+68);
		state->m_pf2_layer->set_scrolly(0, state->m_video_data[0xf002/2]+4);
		state->m_pf2_layer->draw(bitmap, cliprect, 0,0);
		state->m_pf2_layer->draw(bitmap, cliprect, 1,1);
	}

	dynablsb_draw_sprites(screen.machine(),bitmap,cliprect);

	return 0;
}
