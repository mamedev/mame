/*
**  Video Driver for Taito Samurai (1985)
*/

#include "emu.h"
#include "includes/tsamurai.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	UINT8 attributes = state->bg_videoram[2*tile_index+1];
	int tile_number = state->bg_videoram[2*tile_index];
	tile_number += (( attributes & 0xc0 ) >> 6 ) * 256;	 /* legacy */
	tile_number += (( attributes & 0x20 ) >> 5 ) * 1024; /* Mission 660 add-on*/
	SET_TILE_INFO(
			0,
			tile_number,
			attributes & 0x1f,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	int tile_number = state->videoram[tile_index];
	if (state->textbank1 & 0x01) tile_number += 256; /* legacy */
	if (state->textbank2 & 0x01) tile_number += 512; /* Mission 660 add-on */
	SET_TILE_INFO(
			1,
			tile_number,
			state->colorram[((tile_index&0x1f)*2)+1] & 0x1f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( tsamurai )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	state->background = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,32,32);
	state->foreground = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(state->background,0);
	tilemap_set_transparent_pen(state->foreground,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( tsamurai_scrolly_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	tilemap_set_scrolly( state->background, 0, data );
}

WRITE8_HANDLER( tsamurai_scrollx_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	tilemap_set_scrollx( state->background, 0, data );
}

WRITE8_HANDLER( tsamurai_bgcolor_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	state->bgcolor = data;
}

WRITE8_HANDLER( tsamurai_textbank1_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	if( state->textbank1!=data )
	{
		state->textbank1 = data;
		tilemap_mark_all_tiles_dirty( state->foreground );
	}
}

WRITE8_HANDLER( tsamurai_textbank2_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	if( state->textbank2!=data )
	{
		state->textbank2 = data;
		tilemap_mark_all_tiles_dirty( state->foreground );
	}
}

WRITE8_HANDLER( tsamurai_bg_videoram_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	state->bg_videoram[offset]=data;
	offset = offset/2;
	tilemap_mark_tile_dirty(state->background,offset);
}
WRITE8_HANDLER( tsamurai_fg_videoram_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	state->videoram[offset]=data;
	tilemap_mark_tile_dirty(state->foreground,offset);
}
WRITE8_HANDLER( tsamurai_fg_colorram_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	if( state->colorram[offset]!=data )
	{
		state->colorram[offset]=data;
		if (offset & 1)
		{
			int col = offset/2;
			int row;
			for (row = 0;row < 32;row++)
				tilemap_mark_tile_dirty(state->foreground,32*row+col);
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	UINT8 *spriteram = state->spriteram;
	gfx_element *gfx = machine.gfx[2];
	const UINT8 *source = spriteram+32*4-4;
	const UINT8 *finish = spriteram; /* ? */
	state->flicker = 1-state->flicker;

	while( source>=finish )
	{
		int attributes = source[2]; /* bit 0x10 is usually, but not always set */

		int sx = source[3] - 16;
		int sy = 240-source[0];
		int sprite_number = source[1];
		int color = attributes&0x1f;

#if 0
		/* VS Gong Fight */
		if (attributes == 0xe)
			attributes = 4;
		if (attributes > 7 || attributes < 4 || attributes == 5 )
			attributes = 6;
		color = attributes&0x1f;
#endif

#if 0
		/* Nunchakun */
		color = 0x2d - (attributes&0x1f);
#endif

		if( sy<-16 ) sy += 256;

		/* 240-source[0] seems nice,but some dangling sprites appear on the left      */
		/* side in Mission 660.Setting it to 242 fixes it,but will break other games. */
		/* So I'm using this specific check. -kal 11 jul 2002 */
//      if(sprite_type == 1) sy=sy+2;

		if( flip_screen_get(machine) )
		{
			drawgfx_transpen( bitmap,cliprect,gfx,
				sprite_number&0x7f,
				color,
				1,(sprite_number&0x80)?0:1,
				256-32-sx,256-32-sy,0 );
		}
		else
		{
			drawgfx_transpen( bitmap,cliprect,gfx,
				sprite_number&0x7f,
				color,
				0,sprite_number&0x80,
				sx,sy,0 );
		}

		source -= 4;
	}
}

SCREEN_UPDATE( tsamurai )
{
	tsamurai_state *state = screen->machine().driver_data<tsamurai_state>();
	int i;

/* Do the column scroll used for the "660" logo on the title screen */
	tilemap_set_scroll_cols(state->foreground, 32);
	for (i = 0 ; i < 32 ; i++)
	{
		tilemap_set_scrolly(state->foreground, i, state->colorram[i*2]);
	}
/* end of column scroll code */

	/*
        This following isn't particularly efficient.  We'd be better off to
        dynamically change every 8th palette to the background color, so we
        could draw the background as an opaque tilemap.

        Note that the background color register isn't well understood
        (screenshots would be helpful)
    */
	bitmap_fill(bitmap,cliprect,state->bgcolor);
	tilemap_draw(bitmap,cliprect,state->background,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->foreground,0,0);
	return 0;
}

/***************************************************************************

VS Gong Fight runs on older hardware

***************************************************************************/


WRITE8_HANDLER( vsgongf_color_w )
{
	tsamurai_state *state = space->machine().driver_data<tsamurai_state>();
	if( state->vsgongf_color != data )
	{
		state->vsgongf_color = data;
		tilemap_mark_all_tiles_dirty( state->foreground );
	}
}


static TILE_GET_INFO( get_vsgongf_tile_info )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	int tile_number = state->videoram[tile_index];
	int color = state->vsgongf_color&0x1f;
	if( state->textbank1 ) tile_number += 0x100;
	SET_TILE_INFO(
			1,
			tile_number,
			color,
			0);
}

VIDEO_START( vsgongf )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	state->foreground = tilemap_create(machine, get_vsgongf_tile_info,tilemap_scan_rows,8,8,32,32);
}

SCREEN_UPDATE( vsgongf )
{
	tsamurai_state *state = screen->machine().driver_data<tsamurai_state>();
	#ifdef MAME_DEBUG
	if( input_code_pressed( screen->machine(), KEYCODE_Q ) ){
		while( input_code_pressed( screen->machine(), KEYCODE_Q ) ){
			state->key_count++;
			state->vsgongf_color = state->key_count;
			tilemap_mark_all_tiles_dirty( state->foreground );
		}
	}
	#endif

	tilemap_draw(bitmap,cliprect,state->foreground,0,0);
	draw_sprites(screen->machine(),bitmap,cliprect);
	return 0;
}
