#include "emu.h"
#include "includes/silkroad.h"

/* Sprites probably need to be delayed */
/* Some scroll layers may need to be offset slightly? */
/* Check Sprite Colours after redump */
/* Clean Up */
/* is theres a bg colour register? */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	silkroad_state *state = machine->driver_data<silkroad_state>();
	const gfx_element *gfx = machine->gfx[0];
	UINT32 *source = state->sprram;
	UINT32 *finish = source + 0x1000/4;

	while( source < finish )
	{

		int xpos = (source[0] & 0x01ff0000) >> 16;
		int ypos = (source[0] & 0x0000ffff);
		int tileno = (source[1] & 0xffff0000) >> 16;
		int attr = (source[1] & 0x0000ffff);
		int flipx = (attr & 0x0080);
		int width = ((attr & 0x0f00) >> 8) + 1;
		int wcount;
		int color = (attr & 0x003f) ;
		int pri		 =	((attr & 0x1000)>>12);	// Priority (1 = Low)
		int pri_mask =	~((1 << (pri+1)) - 1);	// Above the first "pri" levels

		// attr & 0x2000 -> another priority bit?

		if ( (source[1] & 0xff00) == 0xff00 ) break;

		if ( (attr & 0x8000) == 0x8000 ) tileno+=0x10000;

		if (!flipx)
		{
			for (wcount=0;wcount<width;wcount++)
			{
				pdrawgfx_transpen(bitmap,cliprect,gfx,tileno+wcount,color,0,0,xpos+wcount*16+8,ypos,machine->priority_bitmap,pri_mask,0);
			}
		}
		else
		{

			for (wcount=width;wcount>0;wcount--)
			{
				pdrawgfx_transpen(bitmap,cliprect,gfx,tileno+(width-wcount),color,1,0,xpos+wcount*16-16+8,ypos,machine->priority_bitmap,pri_mask,0);
			}
		}

		source += 2;
	}
}


static TILE_GET_INFO( get_fg_tile_info )
{
	silkroad_state *state = machine->driver_data<silkroad_state>();
	int code = ((state->vidram[tile_index] & 0xffff0000) >> 16 );
	int color = ((state->vidram[tile_index] & 0x000001f));
	int flipx =  ((state->vidram[tile_index] & 0x0000080) >> 7);

	code += 0x18000;

	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_HANDLER( silkroad_fgram_w )
{
	silkroad_state *state = space->machine->driver_data<silkroad_state>();

	COMBINE_DATA(&state->vidram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

static TILE_GET_INFO( get_fg2_tile_info )
{
	silkroad_state *state = machine->driver_data<silkroad_state>();
	int code = ((state->vidram2[tile_index] & 0xffff0000) >> 16 );
	int color = ((state->vidram2[tile_index] & 0x000001f));
	int flipx =  ((state->vidram2[tile_index] & 0x0000080) >> 7);
	code += 0x18000;
	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_HANDLER( silkroad_fgram2_w )
{
	silkroad_state *state = space->machine->driver_data<silkroad_state>();

	COMBINE_DATA(&state->vidram2[offset]);
	tilemap_mark_tile_dirty(state->fg2_tilemap,offset);
}

static TILE_GET_INFO( get_fg3_tile_info )
{
	silkroad_state *state = machine->driver_data<silkroad_state>();
	int code = ((state->vidram3[tile_index] & 0xffff0000) >> 16 );
	int color = ((state->vidram3[tile_index] & 0x000001f));
	int flipx =  ((state->vidram3[tile_index] & 0x0000080) >> 7);
	code += 0x18000;
	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_HANDLER( silkroad_fgram3_w )
{
	silkroad_state *state = space->machine->driver_data<silkroad_state>();

	COMBINE_DATA(&state->vidram3[offset]);
	tilemap_mark_tile_dirty(state->fg3_tilemap,offset);
}

VIDEO_START(silkroad)
{
	silkroad_state *state = machine->driver_data<silkroad_state>();
	state->fg_tilemap  = tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 16, 16, 64, 64);
	state->fg2_tilemap = tilemap_create(machine, get_fg2_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->fg3_tilemap = tilemap_create(machine, get_fg3_tile_info, tilemap_scan_rows, 16, 16, 64, 64);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->fg2_tilemap, 0);
	tilemap_set_transparent_pen(state->fg3_tilemap, 0);
}

VIDEO_UPDATE(silkroad)
{
	silkroad_state *state = screen->machine->driver_data<silkroad_state>();
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x7c0);

	tilemap_set_scrollx( state->fg_tilemap, 0, ((state->regs[0] & 0xffff0000) >> 16) );
	tilemap_set_scrolly( state->fg_tilemap, 0, (state->regs[0] & 0x0000ffff) >> 0 );

	tilemap_set_scrolly( state->fg3_tilemap, 0, (state->regs[1] & 0xffff0000) >> 16 );
	tilemap_set_scrollx( state->fg3_tilemap, 0, (state->regs[2] & 0xffff0000) >> 16 );

	tilemap_set_scrolly( state->fg2_tilemap, 0, ((state->regs[5] & 0xffff0000) >> 16));
	tilemap_set_scrollx( state->fg2_tilemap, 0, (state->regs[2] & 0x0000ffff) >> 0 );

	tilemap_draw(bitmap,cliprect,state->fg_tilemap, 0,0);
	tilemap_draw(bitmap,cliprect,state->fg2_tilemap,0,1);
	tilemap_draw(bitmap,cliprect,state->fg3_tilemap,0,2);
	draw_sprites(screen->machine,bitmap,cliprect);

	if (0)
	{
	    popmessage ("Regs %08x %08x %08x %08x %08x",
		state->regs[0],
		state->regs[1],
		state->regs[2],
		state->regs[4],
		state->regs[5]);
	}

	return 0;
}
