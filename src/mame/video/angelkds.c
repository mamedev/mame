/* video/angelkds.c - see drivers/angelkds.c for more info */

/* graphical issues

enable / disable tilemap bits might be wrong

*/

#include "emu.h"
#include "includes/angelkds.h"


/*** Text Layer Tilemap

*/

static TILE_GET_INFO( get_tx_tile_info )
{
	angelkds_state *state = (angelkds_state *)machine->driver_data;
	int tileno;

	tileno = state->txvideoram[tile_index] + (state->txbank * 0x100);
	SET_TILE_INFO(0, tileno, 0, 0);
}

WRITE8_HANDLER( angelkds_txvideoram_w )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	state->txvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE8_HANDLER( angelkds_txbank_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	if (state->txbank != data)
	{
		state->txbank = data;
		tilemap_mark_all_tiles_dirty(state->tx_tilemap);
	}
}

/*** Top Half Background Tilemap

*/

static TILE_GET_INFO( get_bgtop_tile_info )
{
	angelkds_state *state = (angelkds_state *)machine->driver_data;
	int tileno;

	tileno = state->bgtopvideoram[tile_index];

	tileno += state->bgtopbank * 0x100 ;
	SET_TILE_INFO(1, tileno, 0, 0);
}

WRITE8_HANDLER( angelkds_bgtopvideoram_w )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	state->bgtopvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bgtop_tilemap, offset);
}

WRITE8_HANDLER( angelkds_bgtopbank_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	if (state->bgtopbank != data)
	{
		state->bgtopbank = data;
		tilemap_mark_all_tiles_dirty(state->bgtop_tilemap);
	}
}

WRITE8_HANDLER( angelkds_bgtopscroll_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	tilemap_set_scrollx(state->bgtop_tilemap, 0, data);
}

/*** Bottom Half Background Tilemap

*/

static TILE_GET_INFO( get_bgbot_tile_info )
{
	angelkds_state *state = (angelkds_state *)machine->driver_data;
	int tileno;

	tileno = state->bgbotvideoram[tile_index];

	tileno += state->bgbotbank * 0x100 ;
	SET_TILE_INFO(2, tileno, 1, 0);
}

WRITE8_HANDLER( angelkds_bgbotvideoram_w )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	state->bgbotvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bgbot_tilemap, offset);
}


WRITE8_HANDLER( angelkds_bgbotbank_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	if (state->bgbotbank != data)
	{
		state->bgbotbank = data;
		tilemap_mark_all_tiles_dirty(state->bgbot_tilemap);
	}
}

WRITE8_HANDLER( angelkds_bgbotscroll_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	tilemap_set_scrollx(state->bgbot_tilemap, 0, data);
}


WRITE8_HANDLER( angelkds_layer_ctrl_write )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;

	state->layer_ctrl = data;
}

/*** Sprites

the sprites are similar to the tilemaps in the sense that there is
a split down the middle of the screen

*/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int enable_n)
{
	angelkds_state *state = (angelkds_state *)machine->driver_data;
	const UINT8 *source = state->spriteram + 0x100 - 4;
	const UINT8 *finish = state->spriteram;
	const gfx_element *gfx = machine->gfx[3];

	while (source >= finish)
	{
	/*

    nnnn nnnn - EeFf B?cc - yyyy yyyy - xxxx xxxx

    n = sprite number
    E = Sprite Enabled in Top Half of Screen
    e = Sprite Enabled in Bottom Half of Screen
    F = Flip Y
    f = Flip X
    B = Tile Bank
    ? = unknown, nothing / unused? recheck
    c = color
    y = Y position
    x = X position

    */
		UINT16 tile_no = source[0];
		UINT8 attr = source[1];
		UINT8 ypos = source[2];
		UINT8 xpos = source[3];

		UINT8 enable = attr & 0xc0;
		UINT8 flipx = (attr & 0x10) >> 4;
		UINT8 flipy = (attr & 0x20) >> 5;
		UINT8 bank = attr & 0x08;
		UINT8 color = attr & 0x03;

		if (bank)
			tile_no += 0x100;

		ypos = 0xff - ypos;

		if (enable & enable_n)
		{
			drawgfx_transpen(
					bitmap,
					cliprect,
					gfx,
					tile_no,
					color*4,
					flipx,flipy,
					xpos,ypos,15
					);
			/* wraparound */
			if (xpos > 240)
				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						tile_no,
						color*4,
						flipx,flipy,
						xpos-256,ypos,15
						);
			/* wraparound */
			if (ypos > 240)
			{
				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						tile_no,
						color*4,
						flipx,flipy,
						xpos,ypos-256,15
						);
				/* wraparound */
				if (xpos > 240)
					drawgfx_transpen(
							bitmap,
							cliprect,
							gfx,
							tile_no,
							color*4,
							flipx,flipy,
							xpos-256,ypos-256,15
							);
			}
		}

		source -= 0x04;
	}

}


/*** Palette Handling

 4 bits of Red, 4 bits of Green, 4 bits of Blue

*/

WRITE8_HANDLER( angelkds_paletteram_w )
{
	angelkds_state *state = (angelkds_state *)space->machine->driver_data;
	int no;

	state->paletteram[offset] = data;

	no = offset & 0xff;
	palette_set_color_rgb(space->machine, no, pal4bit(state->paletteram[no]), pal4bit(state->paletteram[no]>>4), pal4bit(state->paletteram[no + 0x100]));
}

/*** Video Start & Update

*/

VIDEO_START( angelkds )
{
	angelkds_state *state = (angelkds_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->tx_tilemap, 0);

	state->bgbot_tilemap = tilemap_create(machine, get_bgbot_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->bgbot_tilemap, 15);

	state->bgtop_tilemap = tilemap_create(machine, get_bgtop_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->bgtop_tilemap, 15);
}

/* enable bits are uncertain */

VIDEO_UPDATE( angelkds )
{
	angelkds_state *state = (angelkds_state *)screen->machine->driver_data;
	const rectangle *visarea = video_screen_get_visible_area(screen);
	rectangle clip;

	bitmap_fill(bitmap, cliprect, 0x3f); /* is there a register controling the colour?, we currently use the last colour of the tx palette */

	/* draw top of screen */
	clip.min_x = 8*0;
	clip.max_x = 8*16-1;
	clip.min_y = visarea->min_y;
	clip.max_y = visarea->max_y;

	if ((state->layer_ctrl & 0x80) == 0x00)
		tilemap_draw(bitmap, &clip, state->bgtop_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, &clip, 0x80);

	if ((state->layer_ctrl & 0x20) == 0x00)
		tilemap_draw(bitmap, &clip, state->tx_tilemap, 0, 0);

	/* draw bottom of screen */
	clip.min_x = 8*16;
	clip.max_x = 8*32-1;
	clip.min_y = visarea->min_y;
	clip.max_y = visarea->max_y;

	if ((state->layer_ctrl & 0x40) == 0x00)
		tilemap_draw(bitmap, &clip, state->bgbot_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, &clip, 0x40);

	if ((state->layer_ctrl & 0x20) == 0x00)
		tilemap_draw(bitmap, &clip, state->tx_tilemap, 0, 0);

	return 0;
}
