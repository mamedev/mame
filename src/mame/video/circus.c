/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  CHANGES:
  MAB 05 MAR 99 - changed overlay support to use artwork functions
  AAT 12 MAY 02 - rewrote Ripcord and added pixel-wise collision

***************************************************************************/

#include "driver.h"
#include "sound/samples.h"
#include "circus.h"

static int clown_x=0,clown_y=0;
int clown_z=0;

static tilemap *bg_tilemap;

/***************************************************************************
***************************************************************************/

WRITE8_HANDLER( circus_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( circus_clown_x_w )
{
	clown_x = 240-data;
}

WRITE8_HANDLER( circus_clown_y_w )
{
	clown_y = 240-data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( circus )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_line(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int x1, int y1, int x2, int y2, int dotted)
{
	/* Draws horizontal and Vertical lines only! */

	int count, skip;

	/* Draw the Line */

	if (dotted > 0)
		skip = 2;
	else
		skip = 1;

	if (x1 == x2)
	{
		for (count = y2; count >= y1; count -= skip)
		{
			*BITMAP_ADDR16(bitmap, count, x1) = machine->pens[1];
		}
	}
	else
	{
		for (count = x2; count >= x1; count -= skip)
		{
			*BITMAP_ADDR16(bitmap, y1, count) = machine->pens[1];
		}
	}
}

static void draw_robot_box (running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int x, int y)
{
	/* Box */

	int ex = x + 24;
	int ey = y + 26;

	draw_line(machine,bitmap,cliprect,x,y,ex,y,0);       /* Top */
	draw_line(machine,bitmap,cliprect,x,ey,ex,ey,0);     /* Bottom */
	draw_line(machine,bitmap,cliprect,x,y,x,ey,0);       /* Left */
	draw_line(machine,bitmap,cliprect,ex,y,ex,ey,0);     /* Right */

	/* Score Grid */

	ey = y + 10;
	draw_line(machine,bitmap,cliprect,x+8,ey,ex,ey,0);   /* Horizontal Divide Line */
	draw_line(machine,bitmap,cliprect,x+8,y,x+8,ey,0);
	draw_line(machine,bitmap,cliprect,x+16,y,x+16,ey,0);
}

static void circus_draw_fg(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* The sync generator hardware is used to   */
	/* draw the border and diving boards        */

	draw_line (machine,bitmap,cliprect,0,18,255,18,0);
	draw_line (machine,bitmap,cliprect,0,249,255,249,1);
	draw_line (machine,bitmap,cliprect,0,18,0,248,0);
	draw_line (machine,bitmap,cliprect,247,18,247,248,0);

	draw_line (machine,bitmap,cliprect,0,137,17,137,0);
	draw_line (machine,bitmap,cliprect,231,137,248,137,0);
	draw_line (machine,bitmap,cliprect,0,193,17,193,0);
	draw_line (machine,bitmap,cliprect,231,193,248,193,0);

	drawgfx(bitmap,machine->gfx[1],
			clown_z,
			0,
			0,0,
			clown_y,clown_x,
			cliprect,TRANSPARENCY_PEN,0);
}

VIDEO_UPDATE( circus )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	circus_draw_fg(machine, bitmap, cliprect);
	return 0;
}

static void robotbwl_draw_scoreboard(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* The sync generator hardware is used to   */
	/* draw the bowling alley & scorecards      */

	for(offs=15;offs<=63;offs+=24)
	{
		draw_robot_box(machine, bitmap, cliprect, offs, 31);
		draw_robot_box(machine, bitmap, cliprect, offs, 63);
		draw_robot_box(machine, bitmap, cliprect, offs, 95);

		draw_robot_box(machine, bitmap, cliprect, offs+152, 31);
		draw_robot_box(machine, bitmap, cliprect, offs+152, 63);
		draw_robot_box(machine, bitmap, cliprect, offs+152, 95);
	}

	draw_robot_box(machine, bitmap, cliprect, 39, 127);                  /* 10th Frame */
	draw_line(machine,bitmap, cliprect, 39,137,47,137,0);          /* Extra digit box */

	draw_robot_box(machine, bitmap, cliprect, 39+152, 127);
	draw_line(machine,bitmap, cliprect, 39+152,137,47+152,137,0);
}

static void robotbwl_draw_bowling_alley(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	draw_line(machine,bitmap, cliprect, 103,17,103,205,0);
	draw_line(machine,bitmap, cliprect, 111,17,111,203,1);
	draw_line(machine,bitmap, cliprect, 152,17,152,205,0);
	draw_line(machine,bitmap, cliprect, 144,17,144,203,1);
}

static void robotbwl_draw_ball(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	drawgfx(bitmap,machine->gfx[1],
			clown_z,
			0,
			0,0,
			clown_y+8,clown_x+8, /* Y is horizontal position */
			cliprect,TRANSPARENCY_PEN,0);
}

VIDEO_UPDATE( robotbwl )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	robotbwl_draw_scoreboard(machine, bitmap, cliprect);
	robotbwl_draw_bowling_alley(machine, bitmap, cliprect);
	robotbwl_draw_ball(machine, bitmap, cliprect);
	return 0;
}

static void crash_draw_car(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	drawgfx(bitmap,machine->gfx[1],
		clown_z,
		0,
		0,0,
		clown_y,clown_x, /* Y is horizontal position */
		cliprect,TRANSPARENCY_PEN,0);
}

VIDEO_UPDATE( crash )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	crash_draw_car(machine, bitmap, cliprect);
	return 0;
}

static void ripcord_draw_skydiver(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx;
	const pen_t *pal_ptr;
	UINT8  *src_lineptr, *src_pixptr;
	UINT16 *dst_lineptr, *dst_lineend;
	UINT32 code, color;
	int sx, sy;
	int src_pitch, dst_width, dst_height, dst_pitch, dst_pixoffs, dst_pixend;
	int collision, eax, edx;

	gfx = machine->gfx[0];

	code = clown_z;
	color = 0;

	sx = clown_y;
	sy = clown_x - 1;
	dst_width = 16;
	dst_height = 16;
	edx = 1;

	gfx = machine->gfx[1];
	pal_ptr = &machine->remapped_colortable[gfx->color_base + color * gfx->color_granularity];
	src_lineptr = gfx->gfxdata + code * gfx->char_modulo;
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
			src_pixptr ++;
			if (eax)
			{
				eax = pal_ptr[eax];
				collision |= dst_lineptr[dst_pixoffs];
				dst_lineptr[dst_pixoffs] = eax;
			}
			dst_pixoffs += edx;

		} while((dst_pixoffs &= 0xff) != dst_pixend);

		src_lineptr += src_pitch;

	} while((dst_lineptr += dst_pitch) != dst_lineend);

	// report collision only when the character is not blank and within display area
	if (collision && code!=0xf && clown_x>0 && clown_x<240 && clown_y>-12 && clown_y<240)
	{
		cpunum_set_input_line(0, 0, ASSERT_LINE); // interrupt accuracy is critical in Ripcord
		cpunum_set_input_line(0, 0, CLEAR_LINE);
	}
}

VIDEO_UPDATE( ripcord )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	ripcord_draw_skydiver(machine, bitmap, cliprect);
	return 0;
}
