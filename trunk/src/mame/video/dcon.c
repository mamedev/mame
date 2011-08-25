/***************************************************************************

    D-Con video hardware.

***************************************************************************/

#include "emu.h"
#include "includes/dcon.h"


/******************************************************************************/

READ16_HANDLER( dcon_control_r )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	return state->m_enable;
}

WRITE16_HANDLER( dcon_control_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_enable=data;
		if ((state->m_enable&4)==4)
			tilemap_set_enable(state->m_foreground_layer,0);
		else
			tilemap_set_enable(state->m_foreground_layer,1);

		if ((state->m_enable&2)==2)
			tilemap_set_enable(state->m_midground_layer,0);
		else
			tilemap_set_enable(state->m_midground_layer,1);

		if ((state->m_enable&1)==1)
			tilemap_set_enable(state->m_background_layer,0);
		else
			tilemap_set_enable(state->m_background_layer,1);
	}
}

WRITE16_HANDLER( dcon_gfxbank_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	if (data&1)
		state->m_gfx_bank_select=0x1000;
	else
		state->m_gfx_bank_select=0;
}

WRITE16_HANDLER( dcon_background_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	COMBINE_DATA(&state->m_back_data[offset]);
	tilemap_mark_tile_dirty(state->m_background_layer,offset);
}

WRITE16_HANDLER( dcon_foreground_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	COMBINE_DATA(&state->m_fore_data[offset]);
	tilemap_mark_tile_dirty(state->m_foreground_layer,offset);
}

WRITE16_HANDLER( dcon_midground_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	COMBINE_DATA(&state->m_mid_data[offset]);
	tilemap_mark_tile_dirty(state->m_midground_layer,offset);
}

WRITE16_HANDLER( dcon_text_w )
{
	dcon_state *state = space->machine().driver_data<dcon_state>();
	COMBINE_DATA(&state->m_textram[offset]);
	tilemap_mark_tile_dirty(state->m_text_layer,offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			3,
			tile|state->m_gfx_bank_select,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile = state->m_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( dcon )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	state->m_background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,     16,16,32,32);
	state->m_foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	state->m_midground_layer =  tilemap_create(machine, get_mid_tile_info, tilemap_scan_rows,16,16,32,32);
	state->m_text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	tilemap_set_transparent_pen(state->m_midground_layer,15);
	tilemap_set_transparent_pen(state->m_foreground_layer,15);
	tilemap_set_transparent_pen(state->m_text_layer,15);

	state->m_gfx_bank_select = 0;
}

static void draw_sprites(running_machine& machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	dcon_state *state = machine.driver_data<dcon_state>();
	UINT16 *spriteram16 = state->m_spriteram;
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
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else if (fx && !fy)
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else if (!fx && fy)
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}

				inc++;
			}
	}
}

SCREEN_UPDATE( dcon )
{
	dcon_state *state = screen->machine().driver_data<dcon_state>();
	bitmap_fill(screen->machine().priority_bitmap,cliprect,0);

	/* Setup the tilemaps */
	tilemap_set_scrollx( state->m_background_layer,0, state->m_scroll_ram[0] );
	tilemap_set_scrolly( state->m_background_layer,0, state->m_scroll_ram[1] );
	tilemap_set_scrollx( state->m_midground_layer, 0, state->m_scroll_ram[2] );
	tilemap_set_scrolly( state->m_midground_layer, 0, state->m_scroll_ram[3] );
	tilemap_set_scrollx( state->m_foreground_layer,0, state->m_scroll_ram[4] );
	tilemap_set_scrolly( state->m_foreground_layer,0, state->m_scroll_ram[5] );

	if ((state->m_enable&1)!=1)
		tilemap_draw(bitmap,cliprect,state->m_background_layer,0,0);
	else
		bitmap_fill(bitmap,cliprect,15); /* Should always be black, not pen 15 */

	tilemap_draw(bitmap,cliprect,state->m_midground_layer,0,1);
	tilemap_draw(bitmap,cliprect,state->m_foreground_layer,0,2);
	tilemap_draw(bitmap,cliprect,state->m_text_layer,0,4);

	draw_sprites(screen->machine(),bitmap,cliprect);
	return 0;
}

SCREEN_UPDATE( sdgndmps )
{
	dcon_state *state = screen->machine().driver_data<dcon_state>();

	bitmap_fill(screen->machine().priority_bitmap,cliprect,0);

	/* Gfx banking */
	if (state->m_last_gfx_bank!=state->m_gfx_bank_select)
	{
		tilemap_mark_all_tiles_dirty(state->m_midground_layer);
		state->m_last_gfx_bank=state->m_gfx_bank_select;
	}

	/* Setup the tilemaps */
	tilemap_set_scrollx( state->m_background_layer,0, state->m_scroll_ram[0]+128 );
	tilemap_set_scrolly( state->m_background_layer,0, state->m_scroll_ram[1] );
	tilemap_set_scrollx( state->m_midground_layer, 0, state->m_scroll_ram[2]+128 );
	tilemap_set_scrolly( state->m_midground_layer, 0, state->m_scroll_ram[3] );
	tilemap_set_scrollx( state->m_foreground_layer,0, state->m_scroll_ram[4]+128 );
	tilemap_set_scrolly( state->m_foreground_layer,0, state->m_scroll_ram[5] );
	tilemap_set_scrollx( state->m_text_layer,0, /*state->m_scroll_ram[6] + */ 128 );
	tilemap_set_scrolly( state->m_text_layer,0, /*state->m_scroll_ram[7] + */ 0 );

	if ((state->m_enable&1)!=1)
		tilemap_draw(bitmap,cliprect,state->m_background_layer,0,0);
	else
		bitmap_fill(bitmap,cliprect,15); /* Should always be black, not pen 15 */

	tilemap_draw(bitmap,cliprect,state->m_midground_layer,0,1);
	tilemap_draw(bitmap,cliprect,state->m_foreground_layer,0,2);
	tilemap_draw(bitmap,cliprect,state->m_text_layer,0,4);

	draw_sprites(screen->machine(),bitmap,cliprect);
	return 0;
}
