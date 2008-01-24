/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static int gfx_bank;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/
PALETTE_INIT( exctsccr )
{
	int i,idx;
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += machine->drv->total_colors;

	/* characters */
	idx = 0;
	for (i = 0;i < 32;i++)
	{
		COLOR(0,idx++) = color_prom[256+0+(i*4)];
		COLOR(0,idx++) = color_prom[256+1+(i*4)];
		COLOR(0,idx++) = color_prom[256+2+(i*4)];
		COLOR(0,idx++) = color_prom[256+3+(i*4)];
		COLOR(0,idx++) = color_prom[256+128+0+(i*4)];
		COLOR(0,idx++) = color_prom[256+128+1+(i*4)];
		COLOR(0,idx++) = color_prom[256+128+2+(i*4)];
		COLOR(0,idx++) = color_prom[256+128+3+(i*4)];
	}

	/* sprites */

	idx=0;

	for (i = 0;i < 15*16;i++)
	{
		if ( (i%16) < 8 )
		{
			COLOR(2,idx) = color_prom[i]+16;
			idx++;
		}
	}
	for (i = 15*16;i < 16*16;i++)
	{
		if ( (i%16) > 7 )
		{
			COLOR(2,idx) = color_prom[i]+16;
			idx++;
		}
	}
	for (i = 16;i < 32;i++)
	{
		COLOR(2,idx++) = color_prom[256+0+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+1+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+2+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+3+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+128+0+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+128+1+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+128+2+(i*4)]+16;
		COLOR(2,idx++) = color_prom[256+128+3+(i*4)]+16;
	}

	/* Patch for goalkeeper */
	COLOR(2,29*8+7) = 16;

}

static TIMER_CALLBACK( exctsccr_fm_callback )
{
	cpunum_set_input_line_and_vector(machine, 1, 0, HOLD_LINE, 0xff );
}

WRITE8_HANDLER( exctsccr_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( exctsccr_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( exctsccr_gfx_bank_w )
{
	if (gfx_bank != (data & 0x01))
	{
		gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( exctsccr_flipscreen_w )
{
	if (flip_screen != data)
	{
		flip_screen_set(data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = colorram[tile_index] & 0x1f;

	SET_TILE_INFO(gfx_bank, code, color, 0);
}

VIDEO_START( exctsccr )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	timer_pulse(ATTOTIME_IN_HZ(75), NULL, 0, exctsccr_fm_callback); /* updates fm */
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;
	UINT8 *OBJ1, *OBJ2;

	OBJ1 = videoram;
	OBJ2 = &(spriteram[0x20]);

	for ( offs = 0x0e; offs >= 0; offs -= 2 ) {
		int sx,sy,code,bank,flipx,flipy,color;

		sx = 256 - OBJ2[offs+1];
		sy = OBJ2[offs] - 16;

		code = ( OBJ1[offs] >> 2 ) & 0x3f;
		flipx = ( OBJ1[offs] ) & 0x01;
		flipy = ( OBJ1[offs] ) & 0x02;
		color = ( OBJ1[offs+1] ) & 0x1f;
		bank = 2;
		bank += ( ( OBJ1[offs+1] >> 4 ) & 1 );

		drawgfx(bitmap,machine->gfx[bank],
				code,
				color,
				flipx, flipy,
				sx,sy,
				cliprect,
				TRANSPARENCY_PEN,0);
	}

	OBJ1 = spriteram_2;
	OBJ2 = spriteram;

	for ( offs = 0x0e; offs >= 0; offs -= 2 ) {
		int sx,sy,code,bank,flipx,flipy,color;

		sx = 256 - OBJ2[offs+1];
		sy = OBJ2[offs] - 16;

		code = ( OBJ1[offs] >> 2 ) & 0x3f;
		flipx = ( OBJ1[offs] ) & 0x01;
		flipy = ( OBJ1[offs] ) & 0x02;
		color = ( OBJ1[offs+1] ) & 0x1f;
		bank = 3;

		if ( color == 0 )
			continue;

		if ( color < 0x10 )
			bank++;

		if ( color > 0x10 && color < 0x17 )
		{
			drawgfx(bitmap,machine->gfx[4],
				code,
				0x0e,
				flipx, flipy,
				sx,sy,
				cliprect,
				TRANSPARENCY_PEN,0);

			color += 6;
		}
		if ( color==0x1d && gfx_bank==1 )
		{
			drawgfx(bitmap,machine->gfx[3],
				code,
				color,
				flipx, flipy,
				sx,sy,
				cliprect,
				TRANSPARENCY_PEN,0);
			drawgfx(bitmap,machine->gfx[4],
				code,
				color,
				flipx, flipy,
				sx,sy,
				cliprect,
				TRANSPARENCY_COLOR, 16);

		} else
		{
		drawgfx(bitmap,machine->gfx[bank],
				code,
				color,
				flipx, flipy,
				sx,sy,
				cliprect,
				TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( exctsccr )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
