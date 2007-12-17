/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static int palettebank;

static tilemap *bg_tilemap;

PALETTE_INIT( tagteam )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( tagteam_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( tagteam_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

READ8_HANDLER( tagteam_mirrorvideoram_r )
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return videoram_r(offset);
}

READ8_HANDLER( tagteam_mirrorcolorram_r )
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return colorram_r(offset);
}

WRITE8_HANDLER( tagteam_mirrorvideoram_w )
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	tagteam_videoram_w(offset,data);
}

WRITE8_HANDLER( tagteam_mirrorcolorram_w )
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	tagteam_colorram_w(offset,data);
}

WRITE8_HANDLER( tagteam_control_w )
{
logerror("%04x: control = %02x\n",activecpu_get_pc(),data);

	/* bit 7 is the palette bank */
	palettebank = (data & 0x80) >> 7;
}

WRITE8_HANDLER( tagteam_flipscreen_w )
{
	if (flip_screen != (data &0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + 256 * colorram[tile_index];
	int color = palettebank * 2; // GUESS

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( tagteam )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows_flip_x,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0; offs < 0x20; offs += 4)
	{
		int spritebank = (videoram[offs] & 0x30) << 4;
		int code = videoram[offs + 1] + 256 * spritebank;
		int color = 1 + 2 * palettebank; // GUESS
		int flipx = videoram[offs] & 0x04;
		int flipy = videoram[offs] & 0x02;
		int sx = 240 - videoram[offs + 3];
		int sy = 240 - videoram[offs + 2];

		if (!(videoram[offs] & 0x01)) continue;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);

		/* Wrap around */

		code = videoram[offs + 0x20] + 256 * spritebank;
		color = palettebank;
		sy += (flip_screen ? -256 : 256);

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( tagteam )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
