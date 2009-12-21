/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  CHANGES:
  MAB 05 MAR 99 - changed overlay support to use artwork functions
  AAT 12 MAY 02 - rewrote Ripcord and added pixel-wise collision

***************************************************************************/

#include "driver.h"
#include "sound/samples.h"
#include "includes/circus.h"


WRITE8_HANDLER( circus_videoram_w )
{
	circus_state *state = (circus_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( circus_clown_x_w )
{
	circus_state *state = (circus_state *)space->machine->driver_data;
	state->clown_x = 240 - data;
}

WRITE8_HANDLER( circus_clown_y_w )
{
	circus_state *state = (circus_state *)space->machine->driver_data;
	state->clown_y = 240 - data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	circus_state *state = (circus_state *)machine->driver_data;
	int code = state->videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( circus )
{
	circus_state *state = (circus_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_line( bitmap_t *bitmap, const rectangle *cliprect, int x1, int y1, int x2, int y2, int dotted )
{
	/* Draws horizontal and Vertical lines only! */
	int count, skip;

	/* Draw the Line */
	if (dotted > 0)
		skip = 2;
	else
		skip = 1;

	if (x1 == x2)
		for (count = y2; count >= y1; count -= skip)
			*BITMAP_ADDR16(bitmap, count, x1) = 1;
	else
		for (count = x2; count >= x1; count -= skip)
			*BITMAP_ADDR16(bitmap, y1, count) = 1;
}

static void draw_robot_box( bitmap_t *bitmap, const rectangle *cliprect, int x, int y )
{
	/* Box */
	int ex = x + 24;
	int ey = y + 26;

	draw_line(bitmap, cliprect, x, y, ex, y, 0);       /* Top */
	draw_line(bitmap, cliprect, x, ey, ex, ey, 0);     /* Bottom */
	draw_line(bitmap, cliprect, x, y, x, ey, 0);       /* Left */
	draw_line(bitmap, cliprect, ex, y, ex, ey, 0);     /* Right */

	/* Score Grid */
	ey = y + 10;
	draw_line(bitmap, cliprect, x + 8, ey, ex, ey, 0);   /* Horizontal Divide Line */
	draw_line(bitmap, cliprect, x + 8, y, x + 8, ey, 0);
	draw_line(bitmap, cliprect, x + 16, y, x + 16, ey, 0);
}

static void circus_draw_fg( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	circus_state *state = (circus_state *)machine->driver_data;

	/* The sync generator hardware is used to   */
	/* draw the border and diving boards        */

	draw_line(bitmap, cliprect, 0, 18, 255, 18, 0);
	draw_line(bitmap, cliprect, 0, 249, 255, 249, 1);
	draw_line(bitmap, cliprect, 0, 18, 0, 248, 0);
	draw_line(bitmap, cliprect, 247, 18, 247, 248, 0);

	draw_line(bitmap, cliprect, 0, 137, 17, 137, 0);
	draw_line(bitmap, cliprect, 231, 137, 248, 137, 0);
	draw_line(bitmap, cliprect, 0, 193, 17, 193, 0);
	draw_line(bitmap, cliprect, 231, 193, 248, 193, 0);

	drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
			state->clown_z,
			0,
			0,0,
			state->clown_y, state->clown_x, 0);
}

VIDEO_UPDATE( circus )
{
	circus_state *state = (circus_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	circus_draw_fg(screen->machine, bitmap, cliprect);
	return 0;
}

static void robotbwl_draw_scoreboard( bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;

	/* The sync generator hardware is used to   */
	/* draw the bowling alley & scorecards      */

	for (offs = 15; offs <= 63; offs += 24)
	{
		draw_robot_box(bitmap, cliprect, offs, 31);
		draw_robot_box(bitmap, cliprect, offs, 63);
		draw_robot_box(bitmap, cliprect, offs, 95);

		draw_robot_box(bitmap, cliprect, offs + 152, 31);
		draw_robot_box(bitmap, cliprect, offs + 152, 63);
		draw_robot_box(bitmap, cliprect, offs + 152, 95);
	}

	draw_robot_box(bitmap, cliprect, 39, 127);                 /* 10th Frame */
	draw_line(bitmap, cliprect, 39, 137, 47, 137, 0);          /* Extra digit box */

	draw_robot_box(bitmap, cliprect, 39 + 152, 127);
	draw_line(bitmap, cliprect, 39 + 152, 137, 47 + 152, 137, 0);
}

static void robotbwl_draw_bowling_alley( bitmap_t *bitmap, const rectangle *cliprect )
{
	draw_line(bitmap, cliprect, 103, 17, 103, 205, 0);
	draw_line(bitmap, cliprect, 111, 17, 111, 203, 1);
	draw_line(bitmap, cliprect, 152, 17, 152, 205, 0);
	draw_line(bitmap, cliprect, 144, 17, 144, 203, 1);
}

static void robotbwl_draw_ball( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	circus_state *state = (circus_state *)machine->driver_data;
	drawgfx_transpen(bitmap,/* Y is horizontal position */
			cliprect,machine->gfx[1],
			state->clown_z,
			0,
			0,0,
			state->clown_y + 8, state->clown_x + 8, 0);
}

VIDEO_UPDATE( robotbwl )
{
	circus_state *state = (circus_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	robotbwl_draw_scoreboard(bitmap, cliprect);
	robotbwl_draw_bowling_alley(bitmap, cliprect);
	robotbwl_draw_ball(screen->machine, bitmap, cliprect);
	return 0;
}

static void crash_draw_car( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	circus_state *state = (circus_state *)machine->driver_data;
	drawgfx_transpen(bitmap,/* Y is horizontal position */
		cliprect,machine->gfx[1],
		state->clown_z,
		0,
		0,0,
		state->clown_y, state->clown_x, 0);
}

VIDEO_UPDATE( crash )
{
	circus_state *state = (circus_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	crash_draw_car(screen->machine, bitmap, cliprect);
	return 0;
}

static void ripcord_draw_skydiver( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	circus_state *state = (circus_state *)machine->driver_data;
	const gfx_element *gfx;
	const UINT8  *src_lineptr, *src_pixptr;
	UINT16 *dst_lineptr, *dst_lineend;
	UINT32 code;
	int sx, sy;
	int src_pitch, dst_width, dst_height, dst_pitch, dst_pixoffs, dst_pixend;
	int collision, eax, edx;

	gfx = machine->gfx[0];

	code = state->clown_z;

	sx = state->clown_y;
	sy = state->clown_x - 1;
	dst_width = 16;
	dst_height = 16;
	edx = 1;

	gfx = machine->gfx[1];
	src_lineptr = gfx_element_get_data(gfx, code);
	src_pitch = gfx->line_modulo;
	dst_pitch = bitmap->rowpixels;

	dst_lineptr = BITMAP_ADDR16(bitmap, sy, 0);
	dst_pixend = (sx + dst_width) & 0xff;
	dst_lineend = dst_lineptr + dst_pitch * dst_height;

	// draw sky diver and check collision on a pixel basis
	collision = 0;
	do
	{
		src_pixptr = src_lineptr;
		dst_pixoffs = sx;

		do
		{
			eax = *src_pixptr;
			src_pixptr++;
			if (eax)
			{
				eax = machine->pens[eax];
				collision |= dst_lineptr[dst_pixoffs];
				dst_lineptr[dst_pixoffs] = eax;
			}
			dst_pixoffs += edx;

		} while((dst_pixoffs &= 0xff) != dst_pixend);

		src_lineptr += src_pitch;

	} while((dst_lineptr += dst_pitch) != dst_lineend);

	// report collision only when the character is not blank and within display area
	if (collision && code != 0xf && state->clown_x > 0 && state->clown_x < 240 && state->clown_y > -12 && state->clown_y < 240)
	{
		cpu_set_input_line(state->maincpu, 0, ASSERT_LINE); // interrupt accuracy is critical in Ripcord
		cpu_set_input_line(state->maincpu, 0, CLEAR_LINE);
	}
}

VIDEO_UPDATE( ripcord )
{
	circus_state *state = (circus_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	ripcord_draw_skydiver(screen->machine, bitmap, cliprect);
	return 0;
}
