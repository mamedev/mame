/* video/stlforce.c - see main driver for other notes */

#include "emu.h"
#include "includes/stlforce.h"

/* background, appears to be the bottom layer */

static TILE_GET_INFO( get_stlforce_bg_tile_info )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();
	int tileno,colour;

	tileno = state->m_bg_videoram[tile_index] & 0x0fff;
	colour = state->m_bg_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_bg_videoram_w )
{
	stlforce_state *state = space->machine().driver_data<stlforce_state>();

	state->m_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset);
}

/* middle layer, low */

static TILE_GET_INFO( get_stlforce_mlow_tile_info )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();
	int tileno,colour;

	tileno = state->m_mlow_videoram[tile_index] & 0x0fff;
	colour = state->m_mlow_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 8;
	tileno += 0x1000;

	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_mlow_videoram_w )
{
	stlforce_state *state = space->machine().driver_data<stlforce_state>();

	state->m_mlow_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_mlow_tilemap,offset);
}

/* middle layer, high */

static TILE_GET_INFO( get_stlforce_mhigh_tile_info )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();
	int tileno,colour;

	tileno = state->m_mhigh_videoram[tile_index] & 0x0fff;
	colour = state->m_mhigh_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 16;
	tileno += 0x2000;

	SET_TILE_INFO(0,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_mhigh_videoram_w )
{
	stlforce_state *state = space->machine().driver_data<stlforce_state>();

	state->m_mhigh_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_mhigh_tilemap,offset);
}

/* text layer, appears to be the top layer */

static TILE_GET_INFO( get_stlforce_tx_tile_info )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();
	int tileno,colour;

	tileno = state->m_tx_videoram[tile_index] & 0x0fff;
	colour = state->m_tx_videoram[tile_index] & 0xe000;
	colour = colour >> 13;

	tileno += 0xc000;

	colour += 24;
	SET_TILE_INFO(1,tileno,colour,0);
}

WRITE16_HANDLER( stlforce_tx_videoram_w )
{
	stlforce_state *state = space->machine().driver_data<stlforce_state>();

	state->m_tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset);
}

/* sprites - quite a bit still needs doing .. */

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();
	const UINT16 *source = state->m_spriteram+0x0;
	const UINT16 *finish = state->m_spriteram+0x800;
	const gfx_element *gfx = machine.gfx[2];
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
					 xpos+state->m_sprxoffs,ypos,0 );
		}

		source += 0x4;
	}
}

SCREEN_UPDATE( stlforce )
{
	stlforce_state *state = screen->machine().driver_data<stlforce_state>();
	int i;

	if (state->m_vidattrram[6] & 1)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_bg_tilemap, i, state->m_bg_scrollram[i]+9); //+9 for twinbrat
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_bg_tilemap, i, state->m_bg_scrollram[0]+9); //+9 for twinbrat
	}

	if (state->m_vidattrram[6] & 4)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_mlow_tilemap, i, state->m_mlow_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_mlow_tilemap, i, state->m_mlow_scrollram[0]+8);
	}

	if (state->m_vidattrram[6] & 0x10)
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_mhigh_tilemap, i, state->m_mhigh_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			tilemap_set_scrollx(state->m_mhigh_tilemap, i, state->m_mhigh_scrollram[0]+8);
	}

	tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_vidattrram[1]);
	tilemap_set_scrolly(state->m_mlow_tilemap, 0, state->m_vidattrram[2]);
	tilemap_set_scrolly(state->m_mhigh_tilemap, 0, state->m_vidattrram[3]);

	tilemap_set_scrollx(state->m_tx_tilemap, 0, state->m_vidattrram[0]+8);
	tilemap_set_scrolly(state->m_tx_tilemap, 0,state->m_vidattrram[4]);

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_mlow_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_mhigh_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	return 0;
}

VIDEO_START( stlforce )
{
	stlforce_state *state = machine.driver_data<stlforce_state>();

	state->m_bg_tilemap    = tilemap_create(machine, get_stlforce_bg_tile_info,   tilemap_scan_cols,      16,16,64,16);
	state->m_mlow_tilemap  = tilemap_create(machine, get_stlforce_mlow_tile_info, tilemap_scan_cols, 16,16,64,16);
	state->m_mhigh_tilemap = tilemap_create(machine, get_stlforce_mhigh_tile_info,tilemap_scan_cols, 16,16,64,16);
	state->m_tx_tilemap    = tilemap_create(machine, get_stlforce_tx_tile_info,   tilemap_scan_rows,  8, 8,64,32);

	tilemap_set_transparent_pen(state->m_mlow_tilemap,0);
	tilemap_set_transparent_pen(state->m_mhigh_tilemap,0);
	tilemap_set_transparent_pen(state->m_tx_tilemap,0);

	tilemap_set_scroll_rows(state->m_bg_tilemap, 256);
	tilemap_set_scroll_rows(state->m_mlow_tilemap, 256);
	tilemap_set_scroll_rows(state->m_mhigh_tilemap, 256);
}
