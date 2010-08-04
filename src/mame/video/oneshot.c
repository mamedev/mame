/* One Shot One Kill Video Hardware */

#include "emu.h"
#include "includes/oneshot.h"


/* bg tilemap */
static TILE_GET_INFO( get_oneshot_bg_tile_info )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();
	int tileno = state->bg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 0, 0);
}

WRITE16_HANDLER( oneshot_bg_videoram_w )
{
	oneshot_state *state = space->machine->driver_data<oneshot_state>();
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

/* mid tilemap */
static TILE_GET_INFO( get_oneshot_mid_tile_info )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();
	int tileno = state->mid_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 2, 0);
}

WRITE16_HANDLER( oneshot_mid_videoram_w )
{
	oneshot_state *state = space->machine->driver_data<oneshot_state>();
	COMBINE_DATA(&state->mid_videoram[offset]);
	tilemap_mark_tile_dirty(state->mid_tilemap, offset / 2);
}


/* fg tilemap */
static TILE_GET_INFO( get_oneshot_fg_tile_info )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();
	int tileno = state->fg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 3, 0);
}

WRITE16_HANDLER( oneshot_fg_videoram_w )
{
	oneshot_state *state = space->machine->driver_data<oneshot_state>();
	COMBINE_DATA(&state->fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

VIDEO_START( oneshot )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();

	state->bg_tilemap =  tilemap_create(machine, get_oneshot_bg_tile_info,  tilemap_scan_rows, 16, 16, 32, 32);
	state->mid_tilemap = tilemap_create(machine, get_oneshot_mid_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_tilemap =  tilemap_create(machine, get_oneshot_fg_tile_info,  tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->bg_tilemap,  0);
	tilemap_set_transparent_pen(state->mid_tilemap, 0);
	tilemap_set_transparent_pen(state->fg_tilemap,  0);
}

static void draw_crosshairs( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();
	//int xpos,ypos;

	/* get gun raw coordinates (player 1) */
	state->gun_x_p1 = (input_port_read(machine, "LIGHT0_X") & 0xff) * 320 / 256;
	state->gun_y_p1 = (input_port_read(machine, "LIGHT0_Y") & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009ab0) */
	//xpos = state->gun_x_p1;
	//ypos = state->gun_y_p1;

	state->gun_x_p1 += state->gun_x_shift;

	state->gun_y_p1 -= 0x0a;
	if (state->gun_y_p1 < 0)
		state->gun_y_p1 = 0;


	/* get gun raw coordinates (player 2) */
	state->gun_x_p2 = (input_port_read(machine, "LIGHT1_X") & 0xff) * 320 / 256;
	state->gun_y_p2 = (input_port_read(machine, "LIGHT1_Y") & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009b6e) */
	//xpos = state->gun_x_p2;
	//ypos = state->gun_y_p2;

	state->gun_x_p2 += state->gun_x_shift - 0x0a;
	if (state->gun_x_p2 < 0)
		state->gun_x_p2 = 0;
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	oneshot_state *state = machine->driver_data<oneshot_state>();
	const UINT16 *source = state->sprites;
	const UINT16 *finish = source + (0x1000 / 2);
	const gfx_element *gfx = machine->gfx[1];

	int xpos, ypos;

	while (source < finish)
	{
		int blockx, blocky;
		int num = source[1] & 0xffff;
		int xsize = (source[2] & 0x000f) + 1;
		int ysize = (source[3] & 0x000f) + 1;

		ypos = source[3] & 0xff80;
		xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;


		if (source[0] == 0x0001)
			break;

		xpos -= 8;
		ypos -= 6;

		for (blockx = 0; blockx < xsize; blockx++)
		{
			for (blocky = 0; blocky < ysize; blocky++)
			{
				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8, ypos + blocky * 8, 0);

				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8 - 0x200, ypos + blocky * 8, 0);
			}
		}
		source += 0x4;
	}

}

VIDEO_UPDATE( oneshot )
{
	oneshot_state *state = screen->machine->driver_data<oneshot_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_set_scrollx(state->mid_tilemap, 0, state->scroll[0] - 0x1f5);
	tilemap_set_scrolly(state->mid_tilemap, 0, state->scroll[1]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->mid_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_crosshairs(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( maddonna )
{
	oneshot_state *state = screen->machine->driver_data<oneshot_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_set_scrolly(state->mid_tilemap, 0, state->scroll[1]); // other registers aren't used so we don't know which layers they relate to

	tilemap_draw(bitmap, cliprect, state->mid_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);

//  popmessage ("%04x %04x %04x %04x %04x %04x %04x %04x", state->scroll[0], state->scroll[1], state->scroll[2], state->scroll[3], state->scroll[4], state->scroll[5], state->scroll[6], state->scroll[7]);
	return 0;
}
