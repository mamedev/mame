/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/nycaptor.h"

#define NYCAPTOR_DEBUG	0

#if NYCAPTOR_DEBUG
static  int nycaptor_mask = 0;
#endif


/*
 298 (e298) - spot (0-3) , 299 (e299) - lives
 spot number isn't set to 0 in main menu ; lives - yes
 sprites in main menu req priority 'type' 0
*/
static int nycaptor_spot( running_machine *machine )
{
	nycaptor_state *state = (nycaptor_state *)machine->driver_data;

	if (state->gametype == 0 || state->gametype == 2)
		return state->sharedram[0x299] ? state->sharedram[0x298] : 0;
	else
		return 0;
}

WRITE8_HANDLER(nycaptor_spriteram_w)
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	state->spriteram[offset] = data;
}

READ8_HANDLER(nycaptor_spriteram_r)
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	return state->spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	nycaptor_state *state = (nycaptor_state *)machine->driver_data;
	int pal = state->videoram[tile_index * 2 + 1] & 0x0f;
	tileinfo->category = (state->videoram[tile_index * 2 + 1] & 0x30) >> 4;

	tileinfo->group = 0;

	if ((!nycaptor_spot(machine)) && (pal == 6))
		tileinfo->group = 1;

	if (((nycaptor_spot(machine) == 3) && (pal == 8)) || ((nycaptor_spot(machine) == 1) && (pal == 0xc)))
		tileinfo->group = 2;

	if ((nycaptor_spot(machine) == 1) && (tileinfo->category == 2))
		tileinfo->group = 3;

#if NYCAPTOR_DEBUG
	if (nycaptor_mask & (1 << tileinfo->category))
	{
		if (nycaptor_spot(machine))
			pal = 0xe;
		else
			pal = 4;
	}
#endif

	SET_TILE_INFO(
			0,
			state->videoram[tile_index * 2] + ((state->videoram[tile_index * 2 + 1] & 0xc0) << 2) + 0x400 * state->char_bank,
			pal, 0
			);
}


VIDEO_START( nycaptor )
{
	nycaptor_state *state = (nycaptor_state *)machine->driver_data;

	state->spriteram = auto_alloc_array(machine, UINT8, 160);
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32 );

	tilemap_set_transmask(state->bg_tilemap, 0, 0xf800, 0x7ff); //split 0
	tilemap_set_transmask(state->bg_tilemap, 1, 0xfe00, 0x01ff);//split 1
	tilemap_set_transmask(state->bg_tilemap, 2, 0xfffc, 0x0003);//split 2
	tilemap_set_transmask(state->bg_tilemap, 3, 0xfff0, 0x000f);//split 3

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	machine->generic.paletteram2.u8 = auto_alloc_array(machine, UINT8, 0x200);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	state_save_register_global_pointer(machine, state->spriteram, 160);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram2.u8, 0x200);
}

WRITE8_HANDLER( nycaptor_videoram_w )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset >> 1);
}

READ8_HANDLER( nycaptor_videoram_r )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	return state->videoram[offset];
}

WRITE8_HANDLER( nycaptor_palette_w )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;

	if (state->gametype == 2) //colt
		return;

	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (state->palette_bank << 8), data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (state->palette_bank << 8), data);
}

READ8_HANDLER( nycaptor_palette_r )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;

	if (offset & 0x100)
		return space->machine->generic.paletteram2.u8[(offset & 0xff) + (state->palette_bank << 8)];
	else
		return space->machine->generic.paletteram.u8 [(offset & 0xff) + (state->palette_bank << 8)];
}

WRITE8_HANDLER( nycaptor_gfxctrl_w )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;

	if (state->gfxctrl == data)
		return;

	state->gfxctrl = data;

	if (state->char_bank != ((data & 0x18) >> 3))
	{
		state->char_bank = ((data & 0x18) >> 3);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	state->palette_bank = BIT(data, 5);

}

READ8_HANDLER( nycaptor_gfxctrl_r )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	return state->gfxctrl;
}

READ8_HANDLER( nycaptor_scrlram_r )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	return state->scrlram[offset];
}

WRITE8_HANDLER( nycaptor_scrlram_w )
{
	nycaptor_state *state = (nycaptor_state *)space->machine->driver_data;
	state->scrlram[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, offset, data);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	nycaptor_state *state = (nycaptor_state *)machine->driver_data;
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		int code, sx, sy, flipx, flipy, pal, priori;

		code = state->spriteram[offs + 2] + ((state->spriteram[offs + 1] & 0x10) << 4);//1 bit wolny = 0x20
		pal  = state->spriteram[offs + 1] & 0x0f;
		sx   = state->spriteram[offs + 3];
		sy   = 240 - state->spriteram[offs + 0];
		priori = (pr & 0xe0) >> 5;

		if (priori == pri)
		{
#if NYCAPTOR_DEBUG
			if (nycaptor_mask & (1 << (pri + 4))) pal = 0xd;
#endif
			flipx = BIT(state->spriteram[offs + 1], 6);
			flipy = BIT(state->spriteram[offs + 1], 7);

			drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (state->spriteram[offs + 3] > 240)
			{
				sx = (state->spriteram[offs + 3] - 256);
				drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);
			}
		}
	}
}





#if NYCAPTOR_DEBUG
/*
 Keys :
   q/w/e/r - bg priority display select
   a/s/d/f/g/h/j/k - sprite priority display select
   z - clear
   x - no bg/sprite pri.
*/

#define mKEY_MASK(x,y) if (input_code_pressed_once(machine, x)) { nycaptor_mask |= y; tilemap_mark_all_tiles_dirty(state->bg_tilemap);}

static void nycaptor_setmask( running_machine *machine )
{
	nycaptor_state *state = (nycaptor_state *)machine->driver_data;

	mKEY_MASK(KEYCODE_Q, 1); /* bg */
	mKEY_MASK(KEYCODE_W, 2);
	mKEY_MASK(KEYCODE_E, 4);
	mKEY_MASK(KEYCODE_R, 8);

	mKEY_MASK(KEYCODE_A, 0x10); /* sprites */
	mKEY_MASK(KEYCODE_S, 0x20);
	mKEY_MASK(KEYCODE_D, 0x40);
	mKEY_MASK(KEYCODE_F, 0x80);
	mKEY_MASK(KEYCODE_G, 0x100);
	mKEY_MASK(KEYCODE_H, 0x200);
	mKEY_MASK(KEYCODE_J, 0x400);
	mKEY_MASK(KEYCODE_K, 0x800);

	if (input_code_pressed_once(machine, KEYCODE_Z)){nycaptor_mask = 0; tilemap_mark_all_tiles_dirty(state->bg_tilemap);} /* disable */
	if (input_code_pressed_once(machine, KEYCODE_X)){nycaptor_mask |= 0x1000; tilemap_mark_all_tiles_dirty(state->bg_tilemap);} /* no layers */
}
#endif

VIDEO_UPDATE( nycaptor )
{
	nycaptor_state *state = (nycaptor_state *)screen->machine->driver_data;

#if NYCAPTOR_DEBUG
	nycaptor_setmask(screen->machine);
	if (nycaptor_mask & 0x1000)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 3, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 3, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 1, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 1, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 0, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 1);
		draw_sprites(screen->machine, bitmap, cliprect, 2);
		draw_sprites(screen->machine, bitmap, cliprect, 3);
		draw_sprites(screen->machine, bitmap, cliprect, 4);
		draw_sprites(screen->machine, bitmap, cliprect, 5);
		draw_sprites(screen->machine, bitmap, cliprect, 6);
		draw_sprites(screen->machine, bitmap, cliprect, 7);
	}
	else
	#endif
	switch (nycaptor_spot(screen->machine) & 3)
	{
	case 0:
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 6);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 3, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 3);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 2);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 1:
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 3, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 3);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 3, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 2);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 1, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 0, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 2:
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 3, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 3, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 1, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 2, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 2, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 0, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;

	case 3:
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 1, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1 | 0, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0 | 0, 0);
		break;
	}

	return 0;
}

