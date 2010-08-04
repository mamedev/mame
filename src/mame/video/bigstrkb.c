/* Big Striker (bootleg) Video Hardware */

#include "emu.h"
#include "includes/bigstrkb.h"


/* Sprites */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	/*- SPR RAM Format -**

     16 bytes per sprite

      nnnn nnnn  nnnn nnnn  aaaa aaaa  aaaa aaaa  xxxx xxxx  xxxx xxxx  yyyy yyyy  yyyy yyyy
        ( rest unused )
    **- End of Comments -*/

	bigstrkb_state *state = machine->driver_data<bigstrkb_state>();
	const gfx_element *gfx = machine->gfx[2];
	UINT16 *source = state->spriteram;
	UINT16 *finish = source + 0x800/2;

	while( source<finish )
	{
		int xpos, ypos, num, attr;

		int flipx, col;

		xpos = source[2];
		ypos = source[3];
		num = source[0];
		attr = source[1];

		ypos = 0xffff - ypos;


		xpos -= 126;
		ypos -= 16;

		flipx = attr & 0x0100;
		col = attr & 0x000f;

		drawgfx_transpen(bitmap,cliprect,gfx,num,col,flipx,0,xpos,ypos,15);
		source+=8;
	}
}

/* Tilemaps */

static TILEMAP_MAPPER( bsb_bg_scan )
{
	int offset;

	offset = ((col&0xf)*16) + (row&0xf);
	offset += (col >> 4) * 0x100;
	offset += (row >> 4) * 0x800;

	return offset;
}

static TILE_GET_INFO( get_bsb_tile_info )
{
	bigstrkb_state *state = machine->driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->videoram[tile_index] & 0x0fff;
	col=	state->videoram[tile_index] & 0xf000;

	SET_TILE_INFO(0,tileno,col>>12,0);
}

WRITE16_HANDLER( bsb_videoram_w )
{
	bigstrkb_state *state = space->machine->driver_data<bigstrkb_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap,offset);
}

static TILE_GET_INFO( get_bsb_tile2_info )
{
	bigstrkb_state *state = machine->driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->videoram2[tile_index] & 0x0fff;
	col=	state->videoram2[tile_index] & 0xf000;

	SET_TILE_INFO(1,tileno,col>>12,0);
}

WRITE16_HANDLER( bsb_videoram2_w )
{
	bigstrkb_state *state = space->machine->driver_data<bigstrkb_state>();
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap2,offset);
}


static TILE_GET_INFO( get_bsb_tile3_info )
{
	bigstrkb_state *state = machine->driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->videoram3[tile_index] & 0x0fff;
	col=	state->videoram3[tile_index] & 0xf000;

	SET_TILE_INFO(1,tileno+0x2000,(col>>12)+(0x100/16),0);
}

WRITE16_HANDLER( bsb_videoram3_w )
{
	bigstrkb_state *state = space->machine->driver_data<bigstrkb_state>();
	state->videoram3[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap3,offset);
}

/* Video Start / Update */

VIDEO_START(bigstrkb)
{
	bigstrkb_state *state = machine->driver_data<bigstrkb_state>();
	state->tilemap = tilemap_create(machine, get_bsb_tile_info,tilemap_scan_cols, 8, 8,64,32);
	state->tilemap2 = tilemap_create(machine, get_bsb_tile2_info,bsb_bg_scan, 16, 16,128,64);
	state->tilemap3 = tilemap_create(machine, get_bsb_tile3_info,bsb_bg_scan, 16, 16,128,64);

	tilemap_set_transparent_pen(state->tilemap,15);
	//tilemap_set_transparent_pen(state->tilemap2,15);
	tilemap_set_transparent_pen(state->tilemap3,15);
}

VIDEO_UPDATE(bigstrkb)
{
	bigstrkb_state *state = screen->machine->driver_data<bigstrkb_state>();

//  bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	tilemap_set_scrollx(state->tilemap2,0, state->vidreg1[0]+(256-14));
	tilemap_set_scrolly(state->tilemap2,0, state->vidreg2[0]);

	tilemap_set_scrollx(state->tilemap3,0, state->vidreg1[1]+(256-14));
	tilemap_set_scrolly(state->tilemap3,0, state->vidreg2[1]);

	tilemap_draw(bitmap,cliprect,state->tilemap2,0,0);
	tilemap_draw(bitmap,cliprect,state->tilemap3,0,0);

	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->tilemap,0,0);

//  popmessage ("Regs %08x %08x %08x %08x",bsb_vidreg2[0],bsb_vidreg2[1],bsb_vidreg2[2],bsb_vidreg2[3]);
	return 0;
}
