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
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x80; i++)
	{
		int swapped_i = BITSWAP8(i,2,7,6,5,4,3,1,0);
		UINT8 ctabentry = color_prom[0x100 + swapped_i];
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites */
	for (i = 0; i < 0xf0; i++)
	{
		if ((i & 0x08) == 0)
		{
			int swapped_i = BITSWAP8(i,3,7,6,5,4,2,1,0);
			UINT8 ctabentry = color_prom[i] | 0x10;
			colortable_entry_set_value(machine->colortable, swapped_i + 0x80, ctabentry);
		}
	}

	for (i = 0xf0; i < 0x100; i++)
	{
		if (i & 0x08)
		{
			int swapped_i = BITSWAP8(i,3,7,6,5,4,2,1,0);
			UINT8 ctabentry = color_prom[i] | 0x10;
			colortable_entry_set_value(machine->colortable, swapped_i + 0x80, ctabentry);
		}
	}

	for (i = 0x80; i < 0x100; i++)
	{
		int swapped_i = BITSWAP8(i,2,7,6,5,4,3,1,0);
		UINT8 ctabentry = color_prom[0x100 + swapped_i] | 0x10;
		colortable_entry_set_value(machine->colortable, i + 0x80, ctabentry);
	}

	/* patch for goalkeeper - but of course, this is totally wrong */
	colortable_entry_set_value(machine->colortable, 0x80+(0x1d*8)+7, 0x10);
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
	if (flip_screen_get() != data)
	{
		flip_screen_set(data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];

	/* ZV 02052008 - this used to be "& 0x1f," but that's not supported by the
       color PROM decoding, so I am using 0x0f.  I don't see any glitches */
	int color = colorram[tile_index] & 0x0f;

	SET_TILE_INFO(gfx_bank, code, color, 0);
}

VIDEO_START( exctsccr )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	timer_pulse(ATTOTIME_IN_HZ(75), NULL, 0, exctsccr_fm_callback); /* updates fm */
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
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
				TRANSPARENCY_PENS,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[4], color, 0x10));

		}
		else
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
