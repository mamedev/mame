/***************************************************************************

  circus.c video

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/circus.h"


WRITE8_MEMBER(circus_state::circus_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(circus_state::circus_clown_x_w)
{
	m_clown_x = 240 - data;
}

WRITE8_MEMBER(circus_state::circus_clown_y_w)
{
	m_clown_y = 240 - data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	circus_state *state = machine.driver_data<circus_state>();
	int code = state->m_videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( circus )
{
	circus_state *state = machine.driver_data<circus_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_line( bitmap_ind16 &bitmap, const rectangle &cliprect, int x1, int y1, int x2, int y2, int dotted )
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
			bitmap.pix16(count, x1) = 1;
	else
		for (count = x2; count >= x1; count -= skip)
			bitmap.pix16(y1, count) = 1;
}

static void draw_sprite_collision( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	circus_state *state = machine.driver_data<circus_state>();
	const gfx_element *sprite_gfx = machine.gfx[1];
	const UINT8 *sprite_data = gfx_element_get_data(sprite_gfx, state->m_clown_z);
	int sx, sy, dx, dy;
	int pixel, collision = 0;

	// draw sprite and check collision on a pixel basis
	for (sy = 0; sy < 16; sy++)
	{
		dy = state->m_clown_x + sy-1;
		if (dy>=0 && dy<bitmap.height())
		{
			for (sx = 0; sx < 16; sx++)
			{
				dx = state->m_clown_y + sx;
				if (dx>=0 && dx<bitmap.width())
				{
					pixel = sprite_data[sy * sprite_gfx->line_modulo + sx];
					if (pixel)
					{
						collision |= bitmap.pix16(dy, dx);
						bitmap.pix16(dy, dx) = machine.pens[pixel];
					}
				}
			}
		}
	}

	if (collision)
		device_set_input_line(state->m_maincpu, 0, ASSERT_LINE);
}

static void circus_draw_fg( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* The sync generator hardware is used to   */
	/* draw the border and diving boards        */

	draw_line(bitmap, cliprect, 0, 18, 255, 18, 0);
	draw_line(bitmap, cliprect, 0, 249, 255, 249, 1);
	draw_line(bitmap, cliprect, 0, 18, 0, 248, 0);
	draw_line(bitmap, cliprect, 247, 18, 247, 248, 0);

	draw_line(bitmap, cliprect, 0, 136, 17, 136, 0);
	draw_line(bitmap, cliprect, 231, 136, 248, 136, 0);
	draw_line(bitmap, cliprect, 0, 192, 17, 192, 0);
	draw_line(bitmap, cliprect, 231, 192, 248, 192, 0);
}

SCREEN_UPDATE_IND16( circus )
{
	circus_state *state = screen.machine().driver_data<circus_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	circus_draw_fg(screen.machine(), bitmap, cliprect);
	draw_sprite_collision(screen.machine(), bitmap, cliprect);
	return 0;
}

static void robotbwl_draw_box( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y )
{
	/* Box */
	int ex = x + 24;
	int ey = y + 26;

	draw_line(bitmap, cliprect, x, y, ex, y, 0);		/* Top */
	draw_line(bitmap, cliprect, x, ey, ex, ey, 0);		/* Bottom */
	draw_line(bitmap, cliprect, x, y, x, ey, 0);		/* Left */
	draw_line(bitmap, cliprect, ex, y, ex, ey, 0);		/* Right */

	/* Score Grid */
	ey = y + 10;
	draw_line(bitmap, cliprect, x + 8, ey, ex, ey, 0);	/* Horizontal Divide Line */
	draw_line(bitmap, cliprect, x + 8, y, x + 8, ey, 0);
	draw_line(bitmap, cliprect, x + 16, y, x + 16, ey, 0);
}

static void robotbwl_draw_scoreboard( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	/* The sync generator hardware is used to   */
	/* draw the bowling alley & scorecards      */

	for (offs = 15; offs <= 63; offs += 24)
	{
		robotbwl_draw_box(bitmap, cliprect, offs, 31);
		robotbwl_draw_box(bitmap, cliprect, offs, 63);
		robotbwl_draw_box(bitmap, cliprect, offs, 95);

		robotbwl_draw_box(bitmap, cliprect, offs + 152, 31);
		robotbwl_draw_box(bitmap, cliprect, offs + 152, 63);
		robotbwl_draw_box(bitmap, cliprect, offs + 152, 95);
	}

	robotbwl_draw_box(bitmap, cliprect, 39, 127);		/* 10th Frame */
	draw_line(bitmap, cliprect, 39, 137, 47, 137, 0);	/* Extra digit box */

	robotbwl_draw_box(bitmap, cliprect, 39 + 152, 127);
	draw_line(bitmap, cliprect, 39 + 152, 137, 47 + 152, 137, 0);
}

static void robotbwl_draw_bowling_alley( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	draw_line(bitmap, cliprect, 103, 17, 103, 205, 0);
	draw_line(bitmap, cliprect, 111, 17, 111, 203, 1);
	draw_line(bitmap, cliprect, 152, 17, 152, 205, 0);
	draw_line(bitmap, cliprect, 144, 17, 144, 203, 1);
}

static void robotbwl_draw_ball( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	circus_state *state = machine.driver_data<circus_state>();
	drawgfx_transpen(bitmap,/* Y is horizontal position */
			cliprect,machine.gfx[1],
			state->m_clown_z,
			0,
			0,0,
			state->m_clown_y + 8, state->m_clown_x + 8, 0);
}

SCREEN_UPDATE_IND16( robotbwl )
{
	circus_state *state = screen.machine().driver_data<circus_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	robotbwl_draw_scoreboard(bitmap, cliprect);
	robotbwl_draw_bowling_alley(bitmap, cliprect);
	robotbwl_draw_ball(screen.machine(), bitmap, cliprect);
	return 0;
}

static void crash_draw_car( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	circus_state *state = machine.driver_data<circus_state>();
	drawgfx_transpen(bitmap,/* Y is horizontal position */
		cliprect,machine.gfx[1],
		state->m_clown_z,
		0,
		0,0,
		state->m_clown_y, state->m_clown_x - 1, 0);
}

SCREEN_UPDATE_IND16( crash )
{
	circus_state *state = screen.machine().driver_data<circus_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	crash_draw_car(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( ripcord )
{
	circus_state *state = screen.machine().driver_data<circus_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprite_collision(screen.machine(), bitmap, cliprect);
	return 0;
}
