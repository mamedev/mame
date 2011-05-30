/***************************************************************************

  video.c

  Functions to emulate the video hardware of early Toaplan hardware.

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pf_tile_info )	/* For Performan only */
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	int tile,color;

	tile=state->m_slapfight_videoram[tile_index] + ((state->m_slapfight_colorram[tile_index] & 0x03) << 8);
	color=(state->m_slapfight_colorram[tile_index] >> 3) & 0x0f;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	int tile,color;

	tile=state->m_slapfight_videoram[tile_index] + ((state->m_slapfight_colorram[tile_index] & 0x0f) << 8);
	color=(state->m_slapfight_colorram[tile_index] & 0xf0) >> 4;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fix_tile_info )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	int tile,color;

	tile=state->m_slapfight_fixvideoram[tile_index] + ((state->m_slapfight_fixcolorram[tile_index] & 0x03) << 8);
	color=(state->m_slapfight_fixcolorram[tile_index] & 0xfc) >> 2;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( perfrman )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	state->m_pf1_tilemap = tilemap_create(machine, get_pf_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(state->m_pf1_tilemap,0);
}

VIDEO_START( slapfight )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	state->m_pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,tilemap_scan_rows,8,8,64,32);
	state->m_fix_tilemap = tilemap_create(machine, get_fix_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(state->m_fix_tilemap,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( slapfight_videoram_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	state->m_slapfight_videoram[offset]=data;
	tilemap_mark_tile_dirty(state->m_pf1_tilemap,offset);
}

WRITE8_HANDLER( slapfight_colorram_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	state->m_slapfight_colorram[offset]=data;
	tilemap_mark_tile_dirty(state->m_pf1_tilemap,offset);
}

WRITE8_HANDLER( slapfight_fixram_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	state->m_slapfight_fixvideoram[offset]=data;
	tilemap_mark_tile_dirty(state->m_fix_tilemap,offset);
}

WRITE8_HANDLER( slapfight_fixcol_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	state->m_slapfight_fixcolorram[offset]=data;
	tilemap_mark_tile_dirty(state->m_fix_tilemap,offset);
}

WRITE8_HANDLER( slapfight_flipscreen_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	logerror("Writing %02x to flipscreen\n",offset);
	if (offset==0) state->m_flipscreen=1; /* Port 0x2 is flipscreen */
	else state->m_flipscreen=0; /* Port 0x3 is normal */
}

WRITE8_HANDLER( slapfight_palette_bank_w )
{
	slapfght_state *state = space->machine().driver_data<slapfght_state>();
	state->m_slapfight_palette_bank = offset;
}

static void slapfght_log_vram(running_machine &machine)
{
#ifdef MAME_DEBUG
	slapfght_state *state = machine.driver_data<slapfght_state>();
	if ( machine.input().code_pressed_once(KEYCODE_B) )
	{
		int i;
		for (i=0; i<0x800; i++)
		{
			logerror("Offset:%03x   TileRAM:%02x   AttribRAM:%02x   SpriteRAM:%02x\n",i, state->m_slapfight_videoram[i],state->m_slapfight_colorram[i],machine.generic.spriteram.u8[i]);
		}
	}
#endif
}

/***************************************************************************

  Render the Sprites

***************************************************************************/
static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int priority_to_display )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	UINT8 *buffered_spriteram = machine.generic.buffered_spriteram.u8;
	int offs;

	for (offs = 0;offs < machine.generic.spriteram_size;offs += 4)
	{
		int sx, sy;

		if ((buffered_spriteram[offs+2] & 0x80) == priority_to_display)
		{
			if (state->m_flipscreen)
			{
				sx = 265 - buffered_spriteram[offs+1];
				sy = 239 - buffered_spriteram[offs+3];
				sy &= 0xff;
			}
			else
			{
				sx = buffered_spriteram[offs+1] + 3;
				sy = buffered_spriteram[offs+3] - 1;
			}
			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				buffered_spriteram[offs],
				((buffered_spriteram[offs+2] >> 1) & 3)	|
					((buffered_spriteram[offs+2] << 2) & 4) | (state->m_slapfight_palette_bank << 3),
				state->m_flipscreen, state->m_flipscreen,
				sx, sy,0);
		}
	}
}


SCREEN_UPDATE( perfrman )
{
	slapfght_state *state = screen->machine().driver_data<slapfght_state>();
	tilemap_set_flip( state->m_pf1_tilemap, state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_scrolly( state->m_pf1_tilemap ,0 , 0 );
	if (state->m_flipscreen) {
		tilemap_set_scrollx( state->m_pf1_tilemap ,0 , 264 );
	}
	else {
		tilemap_set_scrollx( state->m_pf1_tilemap ,0 , -16 );
	}

	tilemap_draw(bitmap,cliprect,state->m_pf1_tilemap,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,state->m_pf1_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0x80);

	slapfght_log_vram(screen->machine());
	return 0;
}


SCREEN_UPDATE( slapfight )
{
	slapfght_state *state = screen->machine().driver_data<slapfght_state>();
	UINT8 *buffered_spriteram = screen->machine().generic.buffered_spriteram.u8;
	int offs;

	tilemap_set_flip_all(screen->machine(),state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (state->m_flipscreen) {
		tilemap_set_scrollx( state->m_fix_tilemap,0,296);
		tilemap_set_scrollx( state->m_pf1_tilemap,0,(*state->m_slapfight_scrollx_lo + 256 * *state->m_slapfight_scrollx_hi)+296 );
		tilemap_set_scrolly( state->m_pf1_tilemap,0, (*state->m_slapfight_scrolly)+15 );
		tilemap_set_scrolly( state->m_fix_tilemap,0, -1 ); /* Glitch in Tiger Heli otherwise */
	}
	else {
		tilemap_set_scrollx( state->m_fix_tilemap,0,0);
		tilemap_set_scrollx( state->m_pf1_tilemap,0,(*state->m_slapfight_scrollx_lo + 256 * *state->m_slapfight_scrollx_hi) );
		tilemap_set_scrolly( state->m_pf1_tilemap,0, (*state->m_slapfight_scrolly)-1 );
		tilemap_set_scrolly( state->m_fix_tilemap,0, -1 ); /* Glitch in Tiger Heli otherwise */
	}

	tilemap_draw(bitmap,cliprect,state->m_pf1_tilemap,0,0);

	/* Draw the sprites */
	for (offs = 0;offs < screen->machine().generic.spriteram_size;offs += 4)
	{
		if (state->m_flipscreen)
			drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[2],
				buffered_spriteram[offs] + ((buffered_spriteram[offs+2] & 0xc0) << 2),
				(buffered_spriteram[offs+2] & 0x1e) >> 1,
				1,1,
				288-(buffered_spriteram[offs+1] + ((buffered_spriteram[offs+2] & 0x01) << 8)) +18,240-buffered_spriteram[offs+3],0);
		else
			drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[2],
				buffered_spriteram[offs] + ((buffered_spriteram[offs+2] & 0xc0) << 2),
				(buffered_spriteram[offs+2] & 0x1e) >> 1,
				0,0,
				(buffered_spriteram[offs+1] + ((buffered_spriteram[offs+2] & 0x01) << 8)) - 13,buffered_spriteram[offs+3],0);
	}

	tilemap_draw(bitmap,cliprect,state->m_fix_tilemap,0,0);

	slapfght_log_vram(screen->machine());
	return 0;
}
