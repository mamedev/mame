/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static int star_speed;
static int gfxbank;

static tilemap *fg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm using the same palette conversion as Lady Bug, but the Zero Hour
  schematics show a different resistor network.

***************************************************************************/
PALETTE_INIT( redclash )
{
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit1,bit2,r,g,b;


		bit1 = (color_prom[i] >> 0) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		r = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (color_prom[i] >> 2) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		g = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit1 + 0x97 * bit2;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* This is for the stars colors */
	for (i = 32;i < 64;i++)
	{
			int bit1,bit2,r,g,b;

			bit2 = (i >> 4) & 0x01;
			bit1 = (i >> 3) & 0x01;
			b = 0x47 * bit1 + 0x97 * bit2;
			bit2 = (i >> 2) & 0x01;
			bit1 = (i >> 1) & 0x01;
			g = 0x47 * bit1 + 0x97 * bit2;
			bit1 = i & 0x01;
			r = 0x47 * bit1;
			palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* characters */
	for (i = 0;i < 8;i++)
	{
		colortable[4 * i] = 0;
		colortable[4 * i + 1] = i + 0x08;
		colortable[4 * i + 2] = i + 0x10;
		colortable[4 * i + 3] = i + 0x18;
	}

	/* sprites */
	for (i = 0;i < 4 * 8;i++)
	{
		int bit0,bit1,bit2,bit3;


		/* low 4 bits are for sprite n */
		bit0 = (color_prom[i + 32] >> 3) & 0x01;
		bit1 = (color_prom[i + 32] >> 2) & 0x01;
		bit2 = (color_prom[i + 32] >> 1) & 0x01;
		bit3 = (color_prom[i + 32] >> 0) & 0x01;
		colortable[i + 4 * 8] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;

		/* high 4 bits are for sprite n + 8 */
		bit0 = (color_prom[i + 32] >> 7) & 0x01;
		bit1 = (color_prom[i + 32] >> 6) & 0x01;
		bit2 = (color_prom[i + 32] >> 5) & 0x01;
		bit3 = (color_prom[i + 32] >> 4) & 0x01;
		colortable[i + 4 * 16] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;
	}
}

WRITE8_HANDLER( redclash_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( redclash_gfxbank_w )
{
	if (gfxbank != (data & 0x01))
	{
		gfxbank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( redclash_flipscreen_w )
{
	flip_screen_set(data & 0x01);
}

void redclash_set_stars_enable( UINT8 on ); //temp
void redclash_set_stars_speed( UINT8 speed );  //temp

/*
star_speed:
0 = unused
1 = unused
2 = forward fast
3 = forward medium
4 = forward slow
5 = backwards slow
6 = backwards medium
7 = backwards fast
*/
WRITE8_HANDLER( redclash_star0_w )
{
	star_speed = (star_speed & ~1) | ((data & 1) << 0);
	redclash_set_stars_speed(star_speed);
}

WRITE8_HANDLER( redclash_star1_w )
{
	star_speed = (star_speed & ~2) | ((data & 1) << 1);
	redclash_set_stars_speed(star_speed);
}
WRITE8_HANDLER( redclash_star2_w )
{
	star_speed = (star_speed & ~4) | ((data & 1) << 2);
	redclash_set_stars_speed(star_speed);
}
WRITE8_HANDLER( redclash_star_reset_w )
{
	redclash_set_stars_enable(1);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = videoram[tile_index];
	int color = (videoram[tile_index] & 0x70) >> 4; // ??

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( redclash )
{
	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int i, offs;

	for (offs = spriteram_size - 0x20;offs >= 0;offs -= 0x20)
	{
		i = 0;
		while (i < 0x20 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				int color = spriteram[offs + i + 2] & 0x0f;
				int sx = spriteram[offs + i + 3];
				int sy = offs / 4 + (spriteram[offs + i] & 0x07);


				switch ((spriteram[offs + i] & 0x18) >> 3)
				{
					case 3:	/* 24x24 */
					{
						int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((gfxbank & 1) << 4);

						drawgfx(bitmap,machine->gfx[3],
								code,
								color,
								0,0,
								sx,sy - 16,
								cliprect,TRANSPARENCY_PEN,0);
						/* wraparound */
						drawgfx(bitmap,machine->gfx[3],
								code,
								color,
								0,0,
								sx - 256,sy - 16,
								cliprect,TRANSPARENCY_PEN,0);
						break;
					}

					case 2:	/* 16x16 */
						if (spriteram[offs + i] & 0x20)	/* zero hour spaceships */
						{
							int code = ((spriteram[offs + i + 1] & 0xf8) >> 3) + ((gfxbank & 1) << 5);
							int bank = (spriteram[offs + i + 1] & 0x02) >> 1;

							drawgfx(bitmap,machine->gfx[4+bank],
									code,
									color,
									0,0,
									sx,sy - 16,
									cliprect,TRANSPARENCY_PEN,0);
						}
						else
						{
							int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((gfxbank & 1) << 4);

							drawgfx(bitmap,machine->gfx[2],
									code,
									color,
									0,0,
									sx,sy - 16,
									cliprect,TRANSPARENCY_PEN,0);
						}
						break;

					case 1:	/* 8x8 */
						drawgfx(bitmap,machine->gfx[1],
								spriteram[offs + i + 1],// + 4 * (spriteram[offs + i + 2] & 0x10),
								color,
								0,0,
								sx,sy - 16,
								cliprect,TRANSPARENCY_PEN,0);
						break;

					case 0:
popmessage("unknown sprite size 0");
						break;
				}
			}
		}
	}
}

static void draw_bullets(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0; offs < 0x20; offs++)
	{
//      sx = videoram[offs];
		int sx = 8 * offs + (videoram[offs] & 0x07);	/* ?? */
		int sy = 0xff - videoram[offs + 0x20];

		if (flip_screen)
		{
			sx = 240 - sx;
		}

		if (sx >= cliprect->min_x && sx <= cliprect->max_x &&
			sy >= cliprect->min_y && sy <= cliprect->max_y)
			*BITMAP_ADDR16(bitmap, sy, sx) = machine->pens[0x0e];
	}
}

/*
 * These functions emulate the star generator board
 * All this comes from the schematics for Zero Hour
 *
 * It has a 17-bit LFSR which has a period of 2^17-1 clocks
 * (This is one pixel shy of "two screens" worth.)
 * So, there are two starfields drawn on alternate frames
 * These will scroll at a rate controlled by the speed register
 *
 * I'm basically doing the same thing by drawing each
 *  starfield on alternate frames, and then offseting them
 */

static UINT8 stars_enable = 0;
static UINT8 stars_speed = 0;
static UINT32 stars_state = 0;
static UINT16 stars_offset = 0;

/* This line can reset the LFSR to zero and disables the star generator */
void redclash_set_stars_enable( UINT8 on )
{
	if ((stars_enable == 0) && (on == 1))
	{
		stars_offset = 0;
	}
	stars_enable = on;
}

/* This sets up which starfield to draw and the offset, */
/* To be called from VIDEO_EOF() */

void redclash_update_stars_state(void)
{
	static UINT8 count = 0;

	if (stars_enable == 0)
		return;

	count++;
	count%=2;

	if (count == 0)
	{
		stars_offset += ((stars_speed*2) - 0x09);
		stars_offset %= 256*256;
		stars_state = 0;
	}
	else
	{
		stars_state = 0x1fc71;
	}
}

/* Set the speed register (3 bits) */

/*
 * 0 left/down fastest (-9/2 pix per frame)
 * 1 left/down faster  (-7/2 pix per frame)
 * 2 left/down fast    (-5/2 pix per frame)
 * 3 left/down medium  (-3/2 pix per frame)
 * 4 left/down slow    (-1/2 pix per frame)
 * 5 right/up slow     (+1/2 pix per frame)
 * 6 right/up medium   (+3/2 pix per frame)
 * 7 right/up fast     (+5/2 pix per frame)
 */

void redclash_set_stars_speed( UINT8 speed )
{
	stars_speed = speed;
}

/* Draw the stars */

/* Space Raider doesn't use the Va bit, and it is also set up to */
/* window the stars to a certain x range */

void redclash_draw_stars(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx)
{
	int i;
	UINT8 tempbit, feedback, star_color, xloc, yloc;
	UINT32 state;
	UINT8 hcond,vcond;

	if (stars_enable == 0)
		return;

	state = stars_state;

	for(i=0;i<256*256;i++)
	{
		xloc = (stars_offset+i)%256;
		yloc = ((stars_offset+i)/256)%256;

		if ((state & 0x10000) == 0)
			tempbit = 1;
		else
			tempbit = 0;
		if ((state & 0x00020) != 0)
			feedback = tempbit ^ 1;
		else
			feedback = tempbit ^ 0;

		hcond = ((xloc+8) & 0x10) >> 4;

		// sraider doesn't have Va hooked up
		if (sraider)
			vcond = 1;
		else
			vcond = yloc & 0x01;

		if (xloc >= cliprect->min_x && xloc <= cliprect->max_x &&
			yloc >= cliprect->min_y && yloc <= cliprect->max_y)
		{
			if ((hcond ^ vcond) == 0)
			{
				/* enable condition */
				if (((state & 0x000ff) == 0x000ff) && (feedback == 0))
				{
					/* used by space raider */
					if ((xloc>=firstx) && (xloc<=lastx))
					{
						star_color = (state >> 9) & 0x1f;
						*BITMAP_ADDR16(bitmap, yloc, xloc) = machine->pens[palette_offset+star_color];
					}
				}
			}
		}

		/* update LFSR state */
		state = ((state<<1) & 0x1fffe) | feedback;
	}
}

VIDEO_EOF( redclash )
{
	redclash_update_stars_state();
}

VIDEO_UPDATE( redclash )
{
	fillbitmap(bitmap, get_black_pen(machine), cliprect);
	redclash_draw_stars(machine, bitmap, cliprect, 32, 0, 0x00, 0xff);
	draw_sprites(machine, bitmap, cliprect);
	draw_bullets(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
