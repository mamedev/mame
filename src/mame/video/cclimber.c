/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/cclimber.h"


UINT8 *cclimber_bsvideoram;
size_t cclimber_bsvideoram_size;
UINT8 *cclimber_bigspriteram;
UINT8 *cclimber_column_scroll;
static int palettebank;
static int sidepanel_enabled;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Climber has three 32x8 palette PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( cclimber )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + (offs)])


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


	/* character and sprite lookup table */
	/* they use colors 0-63 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		/* pen 0 always uses color 0 (background in River Patrol and Silver Land) */
		if (i % 4 == 0) COLOR(0,i) = 0;
		else COLOR(0,i) = i;
	}

	/* big sprite lookup table */
	/* it uses colors 64-95 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		if (i % 4 == 0) COLOR(2,i) = 0;
		else COLOR(2,i) = i + 64;
	}
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Swimmer has two 256x4 char/sprite palette PROMs and one 32x8 big sprite
  palette PROM.
  The palette PROMs are connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 3 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
  bit 0 -- 500 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  bit 7 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  Additionally, the background color of the score panel is determined by
  these resistors:

                  /--- tri-state --  470 -- BLUE
  +5V -- 1kohm ------- tri-state --  390 -- GREEN
                  \--- tri-state -- 1000 -- RED

***************************************************************************/

#define BGPEN (256+32)
#define SIDEPEN (256+32+1)

PALETTE_INIT( swimmer )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + (offs)])


	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i+256] >> 0) & 0x01;
		bit2 = (color_prom[i+256] >> 1) & 0x01;
		g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i+256] >> 2) & 0x01;
		bit2 = (color_prom[i+256] >> 3) & 0x01;
		b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		/* side panel */
		if (i % 8)
		{
			COLOR(0,i) = i;
		    COLOR(0,i+256) = i;
		}
		else
		{
			/* background */
			COLOR(0,i) = BGPEN;
			COLOR(0,i+256) = SIDEPEN;
		}
	}

	color_prom += 2 * 256;

	/* big sprite */
	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette_set_color(machine,i+256,MAKE_RGB(r,g,b));

		if (i % 8 == 0) COLOR(2,i) = BGPEN;  /* enforce transparency */
		else COLOR(2,i) = i+256;
	}

	/* background */
	palette_set_color(machine,BGPEN,MAKE_RGB(0,0,0));
	/* side panel background color */
#if 0
	// values calculated from the resistors don't seem to match the real board
	palette_set_color(machine,SIDEPEN,MAKE_RGB(0x24,0x5d,0x4e));
#endif
	palette_set_color(machine,SIDEPEN,MAKE_RGB(0x20,0x98,0x79));
}



/***************************************************************************

  Swimmer can directly set the background color.
  The latch is connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 7 -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/
WRITE8_HANDLER( swimmer_bgcolor_w )
{
	int bit0,bit1,bit2;
	int r,g,b;


	/* red component */
	bit0 = 0;
	bit1 = (data >> 6) & 0x01;
	bit2 = (data >> 7) & 0x01;
	r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* green component */
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* blue component */
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	palette_set_color(Machine,BGPEN,MAKE_RGB(r,g,b));
}



WRITE8_HANDLER( cclimber_colorram_w )
{
	/* bit 5 of the address is not used for color memory. There is just */
	/* 512 bytes of memory; every two consecutive rows share the same memory */
	/* region. */
	offset &= 0xffdf;

	colorram[offset] = data;
	colorram[offset + 0x20] = data;
}



WRITE8_HANDLER( cclimber_bigsprite_videoram_w )
{
	cclimber_bsvideoram[offset] = data;
}



WRITE8_HANDLER( swimmer_palettebank_w )
{
	palettebank = data & 1;
}



WRITE8_HANDLER( swimmer_sidepanel_enable_w )
{
	sidepanel_enabled = data & 1;
}



static void cclimber_draw_big_sprite(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;
	int ox,oy,sx,sy,flipx,flipy;
	int color;


	ox = 136 - cclimber_bigspriteram[3];
	oy = 128 - cclimber_bigspriteram[2];
	flipx = cclimber_bigspriteram[1] & 0x10;
	flipy = cclimber_bigspriteram[1] & 0x20;
	if (flip_screen_y)      /* only the Y direction has to be flipped */
	{
		oy = 128 - oy;
		flipy = !flipy;
	}
	color = cclimber_bigspriteram[1] & 0x07;	/* cclimber */
//  color = cclimber_bigspriteram[1] & 0x03;    /* swimmer */

	for (offs = cclimber_bsvideoram_size - 1;offs >= 0;offs--)
	{
		sx = offs % 16;
		sy = offs / 16;
		if (flipx) sx = 15 - sx;
		if (flipy) sy = 15 - sy;

		drawgfx(bitmap,machine->gfx[2],
//              cclimber_bsvideoram[offs],  /* cclimber */
				cclimber_bsvideoram[offs] + ((cclimber_bigspriteram[1] & 0x08) << 5),	/* swimmer */
				color,
				flipx,flipy,
				(ox+8*sx) & 0xff,(oy+8*sy) & 0xff,
				cliprect,TRANSPARENCY_PEN,0);

		/* wraparound */
		drawgfx(bitmap,machine->gfx[2],
//              cclimber_bsvideoram[offs],  /* cclimber */
				cclimber_bsvideoram[offs] + ((cclimber_bigspriteram[1] & 0x08) << 5),	/* swimmer */
				color,
				flipx,flipy,
				((ox+8*sx) & 0xff) - 256,(oy+8*sy) & 0xff,
				cliprect,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( cclimber )
{
	int offs;

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		/* vertical flipping flips two adjacent characters */
		if (flipy) sy ^= 1;

		if (flip_screen_x)
		{
			sx = 31 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 31 - sy;
			flipy = !flipy;
		}

		drawgfx(tmpbitmap,machine->gfx[(colorram[offs] & 0x10) ? 1 : 0],
				videoram[offs] + 8 * (colorram[offs] & 0x20),
				colorram[offs] & 0x0f,
				flipx,flipy,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scroll[32];


		if (flip_screen_x)
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[31 - offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}
		else
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}

		copyscrollbitmap(bitmap,tmpbitmap,0,0,32,scroll,cliprect,TRANSPARENCY_NONE,0);
	}


	if (cclimber_bigspriteram[0] & 1)
		/* draw the "big sprite" below sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);


	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;


		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[spriteram[offs + 1] & 0x10 ? 4 : 3],
				(spriteram[offs] & 0x3f) + 2 * (spriteram[offs + 1] & 0x20),
				spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}


	if ((cclimber_bigspriteram[0] & 1) == 0)
		/* draw the "big sprite" over sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( cannonb )
{
	int offs;

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int attr,code,color,sx,sy,flipx,flipy;

		code = videoram[offs];
		attr = colorram[offs] & 0x20;
		color = colorram[offs] & 0x0f;
		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		/* vertical flipping flips two adjacent characters */
		if (flipy) sy ^= 1;

		if (flip_screen_x)
		{
			sx = 31 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 31 - sy;
			flipy = !flipy;
		}

		drawgfx(tmpbitmap,machine->gfx[0],
				code + 8 * attr,
				color,
				flipx,flipy,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scroll[32];


		if (flip_screen_x)
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[31 - offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}
		else
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}

		copyscrollbitmap(bitmap,tmpbitmap,0,0,32,scroll,cliprect,TRANSPARENCY_NONE,0);
	}


	if (cclimber_bigspriteram[0] & 1)
		/* draw the "big sprite" below sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);


	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int code,color,sx,sy,flipx,flipy;

		code = spriteram[offs] & 0x3f;
		color = spriteram[offs + 1] & 0x0f;
		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[3],
				code + 0x40,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}

	if ((cclimber_bigspriteram[0] & 1) == 0)
		/* draw the "big sprite" over sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( swimmer )
{
	int offs;

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy,color;

		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		/* vertical flipping flips two adjacent characters */
		if (flipy) sy ^= 1;

		color = (colorram[offs] & 0x0f) + 0x10 * palettebank;
		if (sx >= 24 && sidepanel_enabled)
		{
			color += 32;
		}

		if (flip_screen_x)
		{
			sx = 31 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 31 - sy;
			flipy = !flipy;
		}

		drawgfx(tmpbitmap,machine->gfx[0],
				videoram[offs] + ((colorram[offs] & 0x10) << 4),
				color,
				flipx,flipy,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scroll[32];


		if (flip_screen_y)
		{
			for (offs = 0;offs < 32;offs++)
				scroll[offs] = cclimber_column_scroll[31 - offs];
		}
		else
		{
			for (offs = 0;offs < 32;offs++)
				scroll[offs] = -cclimber_column_scroll[offs];
		}

		copyscrollbitmap(bitmap,tmpbitmap,0,0,32,scroll,cliprect,TRANSPARENCY_NONE,0);
	}


	if (cclimber_bigspriteram[0] & 1)
		/* draw the "big sprite" below sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);


	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;


		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				(spriteram[offs] & 0x3f) | (spriteram[offs + 1] & 0x10) << 2,
				(spriteram[offs + 1] & 0x0f) + 0x10 * palettebank,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}


	if ((cclimber_bigspriteram[0] & 1) == 0)
		/* draw the "big sprite" over sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( yamato )
{
	int offs;
	int i,j;

	/* bg gradient */
	fillbitmap(bitmap, 0, cliprect);

	for(i=8;i<256;i++)
	{
		pen_t pen = 16*4+8*4 + memory_region(REGION_USER1)[(flip_screen_x?0x1280:0x1200)+(i>>1)];

		for(j=0;j<256;j++)
		{
			*BITMAP_ADDR16(bitmap, j, i-8) = pen;
		}
	}


	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		/* vertical flipping flips two adjacent characters */
		if (flipy) sy ^= 1;

		if (flip_screen_x)
		{
			sx = 31 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 31 - sy;
			flipy = !flipy;
		}

		drawgfx(tmpbitmap,machine->gfx[(colorram[offs] & 0x10) ? 1 : 0],
				videoram[offs] + 8 * (colorram[offs] & 0x20),
				colorram[offs] & 0x0f,
				flipx,flipy,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scroll[32];


		if (flip_screen_x)
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[31 - offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}
		else
		{
			for (offs = 0;offs < 32;offs++)
			{
				scroll[offs] = -cclimber_column_scroll[offs];
				if (flip_screen_y) scroll[offs] = -scroll[offs];
			}
		}

		copyscrollbitmap(bitmap,tmpbitmap,0,0,32,scroll,cliprect,TRANSPARENCY_PEN,0);
	}


	if (cclimber_bigspriteram[0] & 1)
		/* draw the "big sprite" below sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);


	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;


		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[spriteram[offs + 1] & 0x10 ? 4 : 3],
				(spriteram[offs] & 0x3f) + 2 * (spriteram[offs + 1] & 0x20),
				spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}


	if ((cclimber_bigspriteram[0] & 1) == 0)
		/* draw the "big sprite" over sprites */
		cclimber_draw_big_sprite(machine, bitmap, cliprect);
	return 0;
}


/* Top Roller */

#define PRIORITY_OVER  	0x20
#define PRIORITY_UNDER 	0x00
#define PRIORITY_MASK 	0x20

static tilemap *bg_tilemap;

UINT8 *toprollr_videoram2;
UINT8 *toprollr_videoram3;
UINT8 *toprollr_videoram4;


static TILE_GET_INFO( get_tile_info_bg )
{
	int code = toprollr_videoram3[tile_index];
	int attr = toprollr_videoram4[tile_index];
	int flipx,flipy,bank,palette;

	bank=(attr&0x40)<<2;





	flipx=1;
	flipy=0;

	code+=bank;

	palette=(attr&0xf)+0x18 ;

	SET_TILE_INFO(1, code, palette, (flipx?TILE_FLIPX:0)|(flipy?TILE_FLIPY:0));
}

VIDEO_START( toprollr )
{
	bg_tilemap = tilemap_create(get_tile_info_bg,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
}

static void toprollr_draw_big_sprite(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int priority)
{
	if((cclimber_bigspriteram[1]&PRIORITY_MASK)==priority)
	{
		int code,xs,ys,palette,bank,x,y;
		int flipx=0;
		int flipy=0;

		xs=136-cclimber_bigspriteram[3];
		ys=128-cclimber_bigspriteram[2];


		if(xs==0)
		{
			return;
		}

		if (flip_screen_x)
		{
			flipx^=1;
		}

		palette=cclimber_bigspriteram[1]&0x7;

		bank=(cclimber_bigspriteram[1]>>3)&3;


		for(y=0;y<16;y++)
			for(x=0;x<16;x++)
			{
				int sx=x;
				int sy=y;
				if (flipx) sx = 15 - x;
				if (flipy) sy = 15 - y;

				code=cclimber_bsvideoram[y*16+x]+bank*256;

				drawgfx(bitmap, machine->gfx[3], code, palette, flipx, flipy,(sx*8+xs)&0xff,(sy*8+ys)&0xff, cliprect, TRANSPARENCY_PEN, 0);
				drawgfx(bitmap, machine->gfx[3], code, palette, flipx, flipy,((sx*8+xs)&0xff)-256,((sy*8+ys)&0xff)-256, cliprect, TRANSPARENCY_PEN, 0);
			}
	}
}

VIDEO_UPDATE( toprollr )
{

	UINT32 x,y;
	int offs;
	rectangle myclip=*cliprect;
	myclip.min_x=4*8;
	myclip.max_x=29*8-1;

	fillbitmap(bitmap, 0, cliprect);

	tilemap_set_scrollx(bg_tilemap,0,toprollr_videoram3[0]+8);
	tilemap_mark_all_tiles_dirty(bg_tilemap);
	tilemap_draw(bitmap, &myclip,bg_tilemap,0,0);

	toprollr_draw_big_sprite(machine, bitmap, &myclip, PRIORITY_UNDER);

	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy,palette;

		sx = spriteram[offs + 3]-8;
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		palette=0x08+(spriteram[offs + 1] & 0x0f);

		drawgfx(bitmap,machine->gfx[2],
			(spriteram[offs] & 0x3f) + 2 * (spriteram[offs + 1] & 0x20)+8*(spriteram[offs + 1] & 0x10),
				palette,
				flipx,flipy,
				sx,sy,
				&myclip,TRANSPARENCY_PEN,0);
	}

	toprollr_draw_big_sprite(machine, bitmap, &myclip, PRIORITY_OVER);

	for(y=0;y<32;y++)
		for(x=0;x<32;x++)
		{
			int sx=x*8;
			int sy=y*8;
			int flipx=0;
			int flipy=0;

			int code=videoram[y*32+x];
			int attr=(x>16)?(toprollr_videoram2[(y&0xfe)*32+x]):(toprollr_videoram2[y*32+x]);
			int palette;

			if (flip_screen_x)
			{
				sx = 240 - sx;
				flipx^=1;
			}
			if (flip_screen_y)
			{
				sy = 240 - sy;
				flipy^=1;
			}

			palette=8+(attr&0xf);
			drawgfx(bitmap, machine->gfx[0], code+((attr&0xf0)<<4),palette, flipx, flipy, sx, sy, cliprect, TRANSPARENCY_PEN, 0);

		}
	return 0;
}



