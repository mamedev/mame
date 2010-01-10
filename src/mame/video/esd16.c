/***************************************************************************

                          -= ESD 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W           Shows Layer 0 / 1
        A               Shows Sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

        Tile Size:              8 x 8 x 8
        Color Codes:            1 per Layer (banked for Layer 0)
        Layer Size (tiles) :    128 x 64
        Layer Size (pixels):    1024 x 512

    [ 256 Sprites ]

        Sprites are made of 16 x 16 x 5 tiles. Size can vary from 1 to
        8 tiles vertically, while their width is always 1 tile.

    [ Priorities ]

        The game only uses this scheme:

        Back -> Front:  Layer 0, Layer 1, Sprites

***************************************************************************/

#include "emu.h"
#include "includes/esd16.h"


/***************************************************************************

                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

    Color code:  layer 0 (backmost) can bank at every 256 colors,
                 layer 1 uses the first 256.

***************************************************************************/

static TILE_GET_INFO( get_tile_info_0 )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	UINT16 code = state->vram_0[tile_index];
	SET_TILE_INFO(
			1,
			code,
			state->tilemap0_color,
			0);
}

static TILE_GET_INFO( get_tile_info_0_16x16 )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	UINT16 code = state->vram_0[tile_index];
	SET_TILE_INFO(
			2,
			code,
			state->tilemap0_color,
			0);
}


static TILE_GET_INFO( get_tile_info_1 )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	UINT16 code = state->vram_1[tile_index];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

static TILE_GET_INFO( get_tile_info_1_16x16 )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	UINT16 code = state->vram_1[tile_index];
	SET_TILE_INFO(
			2,
			code,
			0,
			0);
}

WRITE16_HANDLER( esd16_vram_0_w )
{
	esd16_state *state = (esd16_state *)space->machine->driver_data;
	COMBINE_DATA(&state->vram_0[offset]);
	tilemap_mark_tile_dirty(state->tilemap_0, offset);
	tilemap_mark_tile_dirty(state->tilemap_0_16x16, offset);
}

WRITE16_HANDLER( esd16_vram_1_w )
{
	esd16_state *state = (esd16_state *)space->machine->driver_data;
	COMBINE_DATA(&state->vram_1[offset]);
	tilemap_mark_tile_dirty(state->tilemap_1, offset);
	tilemap_mark_tile_dirty(state->tilemap_1_16x16, offset);
}

WRITE16_HANDLER( esd16_tilemap0_color_w )
{
	esd16_state *state = (esd16_state *)space->machine->driver_data;
	state->tilemap0_color = data & 3;
	tilemap_mark_all_tiles_dirty(state->tilemap_0);

	flip_screen_set(space->machine, data & 0x80);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/


VIDEO_START( esd16 )
{
	esd16_state *state = (esd16_state *)machine->driver_data;

	state->tilemap_0 = tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows, 8, 8, 0x80, 0x40);
	state->tilemap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows, 8, 8, 0x80, 0x40);

	/* swatpolc changes tilemap 0 to 16x16 at various times */
	state->tilemap_0_16x16 = tilemap_create(machine, get_tile_info_0_16x16, tilemap_scan_rows, 16,16, 0x40, 0x40);

	/* hedpanic changes tilemap 1 to 16x16 at various times */
	state->tilemap_1_16x16 = tilemap_create(machine, get_tile_info_1_16x16, tilemap_scan_rows, 16,16, 0x40, 0x40);

	tilemap_set_scrolldx(state->tilemap_0, -0x60 + 2, -0x60);
	tilemap_set_scrolldx(state->tilemap_1, -0x60, -0x60 + 2);
	tilemap_set_scrolldx(state->tilemap_0_16x16, -0x60 + 2, -0x60);
	tilemap_set_scrolldx(state->tilemap_1_16x16, -0x60, -0x60 + 2);

	tilemap_set_transparent_pen(state->tilemap_1, 0x00);
	tilemap_set_transparent_pen(state->tilemap_1_16x16, 0x00);
}




/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     fedc b--- ---- ----
                ---- -a9- ---- ----     Y Size: (1 << N) Tiles
                ---- ---8 7654 3210     Y (Signed, Bottom-Up)

        2.w                             Code

        4.w     f--- ---- ---- ----     Sprite priority
                -ed- ---- ---- ----
                ---c ---- ---- ----     Color?
                ---- ba9- ---- ----     Color
                ---- ---8 7654 3210     X (Signed)

        6.w     fedc ba9- ---- ----
                ---- ---8 ---- ----     ? 1 (Display Sprite?)
                ---- ---- 7654 3210

- To Do: Flip X&Y ? They seem unused.

  these are clearly the same as the tumble pop (bootleg?) sprites

***************************************************************************/

static void esd16_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	int offs;

	int max_x = video_screen_get_width(machine->primary_screen);
	int max_y = video_screen_get_height(machine->primary_screen);

	for (offs = state->spriteram_size / 2 - 8 / 2; offs >= 0 ; offs -= 8 / 2)
	{
		int y, starty, endy, incy;

		int sy = state->spriteram[offs + 0];
		int code = state->spriteram[offs + 1];
		int sx = state->spriteram[offs + 2];
		int attr = state->spriteram[offs + 3];

		int dimy = 1 << ((sy >> 9) & 3);

		int flipx = sy & 0x2000;
		int flipy = attr & 0x0000;
		int flash = sy & 0x1000;

		int color = (sx >> 9) & 0xf;

		int pri_mask;

		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		if (sx & 0x8000)
			pri_mask = 0xfffe; // under "tilemap 1"
		else
			pri_mask = 0; // above everything

		sx = sx & 0x1ff;
		if (sx >= 0x180)
			sx -= 0x200;

		sy = 0x100 - ((sy & 0xff)  - (sy & 0x100));
		sy -= dimy * 16;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;		sx = max_x - sx -    1 * 16 + 2;	// small offset
			flipy = !flipy;		sy = max_y - sy - dimy * 16;
		}

		if (flipy)	{	starty = sy + (dimy - 1) * 16;	endy = sy-16;		incy = -16;	}
		else		{	starty = sy;				endy = sy + dimy * 16;	incy = +16;	}

		for (y = starty ; y != endy ; y += incy)
		{
			pdrawgfx_transpen(bitmap, cliprect, machine->gfx[0],
						code++,
						color,
						flipx, flipy,
						sx, y,
						machine->priority_bitmap, pri_mask, 0);
		}
	}
}

/* note, check if i can re-merge this with the other or if its really different */
static void hedpanic_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	esd16_state *state = (esd16_state *)machine->driver_data;
	int offs;

	int max_x = video_screen_get_width(machine->primary_screen);
	int max_y = video_screen_get_height(machine->primary_screen);

	for (offs = state->spriteram_size / 2 - 8 / 2; offs >= 0 ; offs -= 8 / 2)
	{
		int y, starty, endy, incy;

		int sy = state->spriteram[offs + 0];
		int code = state->spriteram[offs + 1];
		int sx = state->spriteram[offs + 2];
//      int attr = state->spriteram[offs + 3];

		int dimy = 1 << ((sy >> 9) & 3);

		int flipx = sy & 0x2000;
		int flipy = sy & 0x0000;
		int flash = sy & 0x1000;

		int color = (sx >> 9) & 0xf;

		int pri_mask;

		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		if (sx & 0x8000)
			pri_mask = 0xfffe; // under "tilemap 1"
		else
			pri_mask = 0; // above everything

		sx = sx & 0x1ff;
		if (sx >= 0x180)
			sx -= 0x200;

		sy &= 0x1ff;

		sx -= 24;

		sy = 0x1ff - sy;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;		sx = max_x - sx -    1 * 16 + 2;	// small offset
			flipy = !flipy;		sy = max_y - sy - dimy * 16;
		}

		if (flipy)	{	starty = sy + (dimy - 1) * 16;	endy = sy - 16;		incy = -16;	}
		else		{	starty = sy - dimy * 16;		endy = sy;			incy = +16;	}

		for (y = starty ; y != endy ; y += incy)
		{
			pdrawgfx_transpen(bitmap, cliprect, machine->gfx[0],
						code++,
						color,
						flipx, flipy,
						sx, y,
						machine->priority_bitmap, pri_mask, 0);
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( esd16 )
{
	esd16_state *state = (esd16_state *)screen->machine->driver_data;
	int layers_ctrl = -1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_set_scrollx(state->tilemap_0, 0, state->scroll_0[0]);
	tilemap_set_scrolly(state->tilemap_0, 0, state->scroll_0[1]);

	tilemap_set_scrollx(state->tilemap_1, 0, state->scroll_1[0]);
	tilemap_set_scrolly(state->tilemap_1, 0, state->scroll_1[1]);

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_0, 0, 0);
	else					bitmap_fill(bitmap, cliprect, 0);

	if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_1, 0, 1);

	if (layers_ctrl & 4)	esd16_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( hedpanic )
{
	esd16_state *state = (esd16_state *)screen->machine->driver_data;
	int layers_ctrl = -1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
	{
		if (state->head_layersize[0] & 0x0001)
		{
			tilemap_set_scrollx(state->tilemap_0_16x16, 0, state->scroll_0[0]);
			tilemap_set_scrolly(state->tilemap_0_16x16, 0, state->scroll_0[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_0_16x16, 0, 0);
		}
		else
		{
			tilemap_set_scrollx(state->tilemap_0, 0, state->scroll_0[0]);
			tilemap_set_scrolly(state->tilemap_0, 0, state->scroll_0[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_0, 0, 0);
		}
	}
	else
	{
		bitmap_fill(bitmap, cliprect, 0);
	}


	if (layers_ctrl & 2)
	{
		if (state->head_layersize[0] & 0x0002)
		{
			tilemap_set_scrollx(state->tilemap_1_16x16, 0, state->scroll_1[0]);
			tilemap_set_scrolly(state->tilemap_1_16x16, 0, state->scroll_1[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_1_16x16, 0, 1);
		}
		else
		{
			tilemap_set_scrollx(state->tilemap_1, 0, state->scroll_1[0]);
			tilemap_set_scrolly(state->tilemap_1, 0, state->scroll_1[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_1, 0, 1);
		}

	}

	if (layers_ctrl & 4)	hedpanic_draw_sprites(screen->machine,bitmap,cliprect);

//  popmessage("%04x %04x %04x %04x %04x",head_unknown1[0],head_layersize[0],head_unknown3[0],head_unknown4[0],head_unknown5[0]);
	return 0;
}

// uses older style sprites
VIDEO_UPDATE( hedpanio )
{
	esd16_state *state = (esd16_state *)screen->machine->driver_data;
	int layers_ctrl = -1;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

#ifdef MAME_DEBUG
if ( input_code_pressed(screen->machine, KEYCODE_Z) )
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
	{
		if (state->head_layersize[0] & 0x0001)
		{
			tilemap_set_scrollx(state->tilemap_0_16x16, 0, state->scroll_0[0]);
			tilemap_set_scrolly(state->tilemap_0_16x16, 0, state->scroll_0[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_0_16x16, 0, 0);
		}
		else
		{
			tilemap_set_scrollx(state->tilemap_0, 0, state->scroll_0[0]);
			tilemap_set_scrolly(state->tilemap_0, 0, state->scroll_0[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_0, 0, 0);
		}
	}
	else
	{
		bitmap_fill(bitmap, cliprect, 0);
	}


	if (layers_ctrl & 2)
	{
		if (state->head_layersize[0] & 0x0002)
		{
			tilemap_set_scrollx(state->tilemap_1_16x16, 0, state->scroll_1[0]);
			tilemap_set_scrolly(state->tilemap_1_16x16, 0, state->scroll_1[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_1_16x16, 0, 1);
		}
		else
		{
			tilemap_set_scrollx(state->tilemap_1, 0, state->scroll_1[0]);
			tilemap_set_scrolly(state->tilemap_1, 0, state->scroll_1[1]);
			tilemap_draw(bitmap, cliprect, state->tilemap_1, 0, 1);
		}

	}

	if (layers_ctrl & 4)	esd16_draw_sprites(screen->machine,bitmap,cliprect);

//  popmessage("%04x %04x %04x %04x %04x",head_unknown1[0],head_layersize[0],head_unknown3[0],head_unknown4[0],head_unknown5[0]);
	return 0;
}
