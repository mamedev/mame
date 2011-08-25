#include "emu.h"
#include "includes/sderby.h"

/* BG Layer */

static TILE_GET_INFO( get_sderby_tile_info )
{
	sderby_state *state = machine.driver_data<sderby_state>();
	int tileno,colour;

	tileno = state->m_videoram[tile_index*2];
	colour = state->m_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(1,tileno,colour,0);
}

WRITE16_HANDLER( sderby_videoram_w )
{
	sderby_state *state = space->machine().driver_data<sderby_state>();

	COMBINE_DATA(&state->m_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap,offset/2);
}

/* MD Layer */

static TILE_GET_INFO( get_sderby_md_tile_info )
{
	sderby_state *state = machine.driver_data<sderby_state>();
	int tileno,colour;

	tileno = state->m_md_videoram[tile_index*2];
	colour = state->m_md_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(1,tileno,colour+16,0);
}

WRITE16_HANDLER( sderby_md_videoram_w )
{
	sderby_state *state = space->machine().driver_data<sderby_state>();

	COMBINE_DATA(&state->m_md_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_md_tilemap,offset/2);
}

/* FG Layer */

static TILE_GET_INFO( get_sderby_fg_tile_info )
{
	sderby_state *state = machine.driver_data<sderby_state>();
	int tileno,colour;

	tileno = state->m_fg_videoram[tile_index*2];
	colour = state->m_fg_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(0,tileno,colour+32,0);
}

WRITE16_HANDLER( sderby_fg_videoram_w )
{
	sderby_state *state = space->machine().driver_data<sderby_state>();

	COMBINE_DATA(&state->m_fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset/2);
}


static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect,int codeshift)
{
	sderby_state *state = machine.driver_data<sderby_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs;
	int height = machine.gfx[0]->height;
	int colordiv = machine.gfx[0]->color_granularity / 16;

	for (offs = 4;offs < state->m_spriteram_size/2;offs += 4)
	{
		int sx,sy,code,color,flipx;

		sy = spriteram16[offs+3-4];	/* -4? what the... ??? */
		if (sy == 0x2000) return;	/* end of list marker */

		flipx = sy & 0x4000;
		sx = (spriteram16[offs+1] & 0x01ff) - 16-7;
		sy = (256-8-height - sy) & 0xff;
		code = spriteram16[offs+2] >> codeshift;
		color = (spriteram16[offs+1] & 0x3e00) >> 9;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color/colordiv+48,
				flipx,0,
				sx,sy,0);
	}
}


VIDEO_START( sderby )
{
	sderby_state *state = machine.driver_data<sderby_state>();

	state->m_tilemap = tilemap_create(machine, get_sderby_tile_info,tilemap_scan_rows, 16, 16,32,32);
	state->m_md_tilemap = tilemap_create(machine, get_sderby_md_tile_info,tilemap_scan_rows, 16, 16,32,32);

	tilemap_set_transparent_pen(state->m_md_tilemap,0);

	state->m_fg_tilemap = tilemap_create(machine, get_sderby_fg_tile_info,tilemap_scan_rows, 8, 8,64,32);
	tilemap_set_transparent_pen(state->m_fg_tilemap,0);
}

SCREEN_UPDATE( sderby )
{
	sderby_state *state = screen->machine().driver_data<sderby_state>();

	tilemap_draw(bitmap,cliprect,state->m_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,state->m_md_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	return 0;
}

SCREEN_UPDATE( pmroulet )
{
	sderby_state *state = screen->machine().driver_data<sderby_state>();

	tilemap_draw(bitmap,cliprect,state->m_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_md_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	return 0;
}


WRITE16_HANDLER( sderby_scroll_w )
{
	sderby_state *state = space->machine().driver_data<sderby_state>();

	data = COMBINE_DATA(&state->m_scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(state->m_fg_tilemap,0,data+2);break;
		case 1: tilemap_set_scrolly(state->m_fg_tilemap,0,data-8);break;
		case 2: tilemap_set_scrollx(state->m_md_tilemap,0,data+4);break;
		case 3: tilemap_set_scrolly(state->m_md_tilemap,0,data-8);break;
		case 4: tilemap_set_scrollx(state->m_tilemap,0,data+6);   break;
		case 5: tilemap_set_scrolly(state->m_tilemap,0,data-8);   break;
	}
}
