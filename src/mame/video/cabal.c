/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/cabal.h"

static TILE_GET_INFO( get_back_tile_info )
{
	cabal_state *state = machine->driver_data<cabal_state>();

	int tile = state->videoram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	cabal_state *state = machine->driver_data<cabal_state>();

	int tile = state->colorram[tile_index];
	int color = (tile>>10);

	tile &= 0x3ff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}


VIDEO_START( cabal )
{
	cabal_state *state = machine->driver_data<cabal_state>();

	state->background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,16,16);
	state->text_layer       = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,32,32);

	tilemap_set_transparent_pen(state->text_layer,3);
	tilemap_set_transparent_pen(state->background_layer,15);
}


/**************************************************************************/

WRITE16_HANDLER( cabal_flipscreen_w )
{
	if (ACCESSING_BITS_0_7)
	{
		cabal_state *state = space->machine->driver_data<cabal_state>();
		int flip = (data & 0x20) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
		tilemap_set_flip(state->background_layer,flip);
		tilemap_set_flip(state->text_layer,flip);

		flip_screen_set(space->machine, data & 0x20);
	}
}

WRITE16_HANDLER( cabal_background_videoram16_w )
{
	cabal_state *state = space->machine->driver_data<cabal_state>();
	COMBINE_DATA(&state->videoram[offset]);
	tilemap_mark_tile_dirty(state->background_layer,offset);
}

WRITE16_HANDLER( cabal_text_videoram16_w )
{
	cabal_state *state = space->machine->driver_data<cabal_state>();
	COMBINE_DATA(&state->colorram[offset]);
	tilemap_mark_tile_dirty(state->text_layer,offset);
}


/********************************************************************

    Cabal Spriteram
    ---------------

    +0   .......x ........  Sprite enable bit
    +0   ........ xxxxxxxx  Sprite Y coordinate
    +1   ..??.... ........  ??? unknown ???
    +1   ....xxxx xxxxxxxx  Sprite tile number
    +2   .xxxx... ........  Sprite color bank
    +2   .....x.. ........  Sprite flip x
    +2   .......x xxxxxxxx  Sprite X coordinate
    +3   (unused)

            -------E YYYYYYYY
            ----BBTT TTTTTTTT
            -CCCCF-X XXXXXXXX
            -------- --------

********************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	cabal_state *state = machine->driver_data<cabal_state>();
	int offs,data0,data1,data2;
	UINT16 *spriteram16 = state->spriteram;

	for( offs = state->spriteram_size/2 - 4; offs >= 0; offs -= 4 )
	{
		data0 = spriteram16[offs];
		data1 = spriteram16[offs+1];
		data2 = spriteram16[offs+2];

		if( data0 & 0x100 )
		{
			int tile_number = data1 & 0xfff;
			int color   = ( data2 & 0x7800 ) >> 11;
			int sy = ( data0 & 0xff );
			int sx = ( data2 & 0x1ff );
			int flipx = ( data2 & 0x0400 );
			int flipy = 0;

			if ( sx>256 )   sx -= 512;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen( bitmap,cliprect,machine->gfx[2],
				tile_number,
				color,
				flipx,flipy,
				sx,sy,0xf );
		}
	}
}


VIDEO_UPDATE( cabal )
{
	cabal_state *state = screen->machine->driver_data<cabal_state>();
	tilemap_draw(bitmap,cliprect,state->background_layer,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->text_layer,0,0);
	return 0;
}


