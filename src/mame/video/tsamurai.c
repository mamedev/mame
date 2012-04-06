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
	UINT8 attributes = state->m_bg_videoram[2*tile_index+1];
	int tile_number = state->m_bg_videoram[2*tile_index];
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
	int tile_number = state->m_videoram[tile_index];
	if (state->m_textbank1 & 0x01) tile_number += 256; /* legacy */
	if (state->m_textbank2 & 0x01) tile_number += 512; /* Mission 660 add-on */
	SET_TILE_INFO(
			1,
			tile_number,
			state->m_colorram[((tile_index&0x1f)*2)+1] & 0x1f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( tsamurai )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	state->m_background = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,32,32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32,32);

	state->m_background->set_transparent_pen(0);
	state->m_foreground->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(tsamurai_state::tsamurai_scrolly_w)
{
	m_background->set_scrolly(0, data );
}

WRITE8_MEMBER(tsamurai_state::tsamurai_scrollx_w)
{
	m_background->set_scrollx(0, data );
}

WRITE8_MEMBER(tsamurai_state::tsamurai_bgcolor_w)
{
	m_bgcolor = data;
}

WRITE8_MEMBER(tsamurai_state::tsamurai_textbank1_w)
{
	if( m_textbank1!=data )
	{
		m_textbank1 = data;
		m_foreground ->mark_all_dirty();
	}
}

WRITE8_MEMBER(tsamurai_state::tsamurai_textbank2_w)
{
	if( m_textbank2!=data )
	{
		m_textbank2 = data;
		m_foreground ->mark_all_dirty();
	}
}

WRITE8_MEMBER(tsamurai_state::tsamurai_bg_videoram_w)
{
	m_bg_videoram[offset]=data;
	offset = offset/2;
	m_background->mark_tile_dirty(offset);
}
WRITE8_MEMBER(tsamurai_state::tsamurai_fg_videoram_w)
{
	m_videoram[offset]=data;
	m_foreground->mark_tile_dirty(offset);
}
WRITE8_MEMBER(tsamurai_state::tsamurai_fg_colorram_w)
{
	if( m_colorram[offset]!=data )
	{
		m_colorram[offset]=data;
		if (offset & 1)
		{
			int col = offset/2;
			int row;
			for (row = 0;row < 32;row++)
				m_foreground->mark_tile_dirty(32*row+col);
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	UINT8 *spriteram = state->m_spriteram;
	gfx_element *gfx = machine.gfx[2];
	const UINT8 *source = spriteram+32*4-4;
	const UINT8 *finish = spriteram; /* ? */
	state->m_flicker = 1-state->m_flicker;

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

SCREEN_UPDATE_IND16( tsamurai )
{
	tsamurai_state *state = screen.machine().driver_data<tsamurai_state>();
	int i;

/* Do the column scroll used for the "660" logo on the title screen */
	state->m_foreground->set_scroll_cols(32);
	for (i = 0 ; i < 32 ; i++)
	{
		state->m_foreground->set_scrolly(i, state->m_colorram[i*2]);
	}
/* end of column scroll code */

	/*
        This following isn't particularly efficient.  We'd be better off to
        dynamically change every 8th palette to the background color, so we
        could draw the background as an opaque tilemap.

        Note that the background color register isn't well understood
        (screenshots would be helpful)
    */
	bitmap.fill(state->m_bgcolor, cliprect);
	state->m_background->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	state->m_foreground->draw(bitmap, cliprect, 0,0);
	return 0;
}

/***************************************************************************

VS Gong Fight runs on older hardware

***************************************************************************/


WRITE8_MEMBER(tsamurai_state::vsgongf_color_w)
{
	if( m_vsgongf_color != data )
	{
		m_vsgongf_color = data;
		m_foreground ->mark_all_dirty();
	}
}


static TILE_GET_INFO( get_vsgongf_tile_info )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	int tile_number = state->m_videoram[tile_index];
	int color = state->m_vsgongf_color&0x1f;
	if( state->m_textbank1 ) tile_number += 0x100;
	SET_TILE_INFO(
			1,
			tile_number,
			color,
			0);
}

VIDEO_START( vsgongf )
{
	tsamurai_state *state = machine.driver_data<tsamurai_state>();
	state->m_foreground = tilemap_create(machine, get_vsgongf_tile_info,tilemap_scan_rows,8,8,32,32);
}

SCREEN_UPDATE_IND16( vsgongf )
{
	tsamurai_state *state = screen.machine().driver_data<tsamurai_state>();
	#ifdef MAME_DEBUG
	if( screen.machine().input().code_pressed( KEYCODE_Q ) ){
		while( screen.machine().input().code_pressed( KEYCODE_Q ) ){
			state->m_key_count++;
			state->m_vsgongf_color = state->m_key_count;
			state->m_foreground ->mark_all_dirty();
		}
	}
	#endif

	state->m_foreground->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}
