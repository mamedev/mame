/* video/stlforce.c - see main driver for other notes */

#include "emu.h"
#include "includes/stlforce.h"

/* background, appears to be the bottom layer */

static TILE_GET_INFO( get_stlforce_bg_tile_info )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;
	int tileno,colour;

	tileno = state->bg_videoram[tile_index] & 0x0fff;
	colour = state->bg_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_bg_videoram_w )
{
	stlforce_state *state = (stlforce_state *)space->machine->driver_data;

	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

/* middle layer, low */

static TILE_GET_INFO( get_stlforce_mlow_tile_info )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;
	int tileno,colour;

	tileno = state->mlow_videoram[tile_index] & 0x0fff;
	colour = state->mlow_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 8;
	tileno += 0x1000;

	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_mlow_videoram_w )
{
	stlforce_state *state = (stlforce_state *)space->machine->driver_data;

	state->mlow_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->mlow_tilemap,offset);
}

/* middle layer, high */

static TILE_GET_INFO( get_stlforce_mhigh_tile_info )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;
	int tileno,colour;

	tileno = state->mhigh_videoram[tile_index] & 0x0fff;
	colour = state->mhigh_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 16;
	tileno += 0x2000;

	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_mhigh_videoram_w )
{
	stlforce_state *state = (stlforce_state *)space->machine->driver_data;

	state->mhigh_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->mhigh_tilemap,offset);
}

/* text layer, appears to be the top layer */

static TILE_GET_INFO( get_stlforce_tx_tile_info )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;
	int tileno,colour;

	tileno = state->tx_videoram[tile_index] & 0x0fff;
	colour = state->tx_videoram[tile_index] & 0xe000;
	colour = colour >> 13;

	tileno += 0xc000;

	colour += 24;
	SET_TILE_INFO(1,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_tx_videoram_w )
{
	stlforce_state *state = (stlforce_state *)space->machine->driver_data;

	state->tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap,offset);
}

/* sprites - quite a bit still needs doing .. */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;
	const UINT16 *source = state->spriteram+0x0;
	const UINT16 *finish = state->spriteram+0x800;
	const gfx_element *gfx = machine->gfx[2];
	int ypos, xpos, attr, num;

	while (source<finish)
	{
		if (source[0] & 0x0800)
		{
			ypos = source[0]& 0x01ff;
			attr = source[1]& 0x000f;
			xpos = source[3]& 0x03ff;
			num = (source[2] & 0x1fff);

			ypos = 512-ypos;

			drawgfx_transpen( bitmap,
					 cliprect,
					 gfx,
					 num,
					 64+attr,
					 0,0,
					 xpos+state->sprxoffs,ypos,0 );
		}

		source += 0x4;
	}
}

VIDEO_UPDATE( stlforce )
{
	stlforce_state *state = (stlforce_state *)screen->machine->driver_data;
	int i;

	if (state->vidattrram[6] & 1)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->bg_tilemap, i, state->bg_scrollram[i]+9); //+9 for twinbrat
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->bg_tilemap, i, state->bg_scrollram[0]+9); //+9 for twinbrat
	}

	if (state->vidattrram[6] & 4)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->mlow_tilemap, i, state->mlow_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->mlow_tilemap, i, state->mlow_scrollram[0]+8);
	}

	if (state->vidattrram[6] & 0x10)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->mhigh_tilemap, i, state->mhigh_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->mhigh_tilemap, i, state->mhigh_scrollram[0]+8);
	}

	tilemap_set_scrolly(state->bg_tilemap, 0, state->vidattrram[1]);
	tilemap_set_scrolly(state->mlow_tilemap, 0, state->vidattrram[2]);
	tilemap_set_scrolly(state->mhigh_tilemap, 0, state->vidattrram[3]);

	tilemap_set_scrollx(state->tx_tilemap, 0, state->vidattrram[0]+8);
	tilemap_set_scrolly(state->tx_tilemap, 0,state->vidattrram[4]);

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->mlow_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->mhigh_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->tx_tilemap,0,0);
	return 0;
}

VIDEO_START( stlforce )
{
	stlforce_state *state = (stlforce_state *)machine->driver_data;

	state->bg_tilemap    = tilemap_create(machine, get_stlforce_bg_tile_info,   tilemap_scan_cols,      16,16,64,16);
	state->mlow_tilemap  = tilemap_create(machine, get_stlforce_mlow_tile_info, tilemap_scan_cols, 16,16,64,16);
	state->mhigh_tilemap = tilemap_create(machine, get_stlforce_mhigh_tile_info,tilemap_scan_cols, 16,16,64,16);
	state->tx_tilemap    = tilemap_create(machine, get_stlforce_tx_tile_info,   tilemap_scan_rows,  8, 8,64,32);

	tilemap_set_transparent_pen(state->mlow_tilemap,0);
	tilemap_set_transparent_pen(state->mhigh_tilemap,0);
	tilemap_set_transparent_pen(state->tx_tilemap,0);

	tilemap_set_scroll_rows(state->bg_tilemap, 256);
	tilemap_set_scroll_rows(state->mlow_tilemap, 256);
	tilemap_set_scroll_rows(state->mhigh_tilemap, 256);
}
