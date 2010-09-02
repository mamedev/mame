/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nb1413m3.h"
#include "includes/nbmj8900.h"


static int nbmj8900_scrolly;
static int blitter_destx, blitter_desty;
static int blitter_sizex, blitter_sizey;
static int blitter_src_addr;
static int blitter_direction_x, blitter_direction_y;
static int nbmj8900_vram;
static int nbmj8900_gfxrom;
static int nbmj8900_dispflag;
static int nbmj8900_flipscreen;
static int nbmj8900_clutsel;
static int nbmj8900_screen_refresh;
static int gfxdraw_mode;

static int screen_height;
static int screen_width;

static bitmap_t *nbmj8900_tmpbitmap0, *nbmj8900_tmpbitmap1;
static UINT8 *nbmj8900_videoram0, *nbmj8900_videoram1;
static UINT8 *nbmj8900_palette;
static UINT8 *nbmj8900_clut;


static void nbmj8900_vramflip(running_machine *machine, int vram);
static void nbmj8900_gfxdraw(running_machine *machine);


/******************************************************************************


******************************************************************************/
READ8_HANDLER( nbmj8900_palette_type1_r )
{
	return nbmj8900_palette[offset];
}

WRITE8_HANDLER( nbmj8900_palette_type1_w )
{
	int r, g, b;

	nbmj8900_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((nbmj8900_palette[offset + 0] & 0x0f) >> 0);
	g = ((nbmj8900_palette[offset + 1] & 0xf0) >> 4);
	b = ((nbmj8900_palette[offset + 1] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

#ifdef UNUSED_FUNCTION
READ8_HANDLER( nbmj8900_palette_type2_r )
{
	return nbmj8900_palette[offset];
}

WRITE8_HANDLER( nbmj8900_palette_type2_w )
{
	int r, g, b;

	nbmj8900_palette[offset] = data;

	if (!(offset & 0x100)) return;

	offset &= 0x0ff;

	r = ((nbmj8900_palette[offset + 0x000] & 0x0f) >> 0);
	g = ((nbmj8900_palette[offset + 0x000] & 0xf0) >> 4);
	b = ((nbmj8900_palette[offset + 0x100] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset & 0x0ff), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_HANDLER( nbmj8900_palette_type3_r )
{
	return nbmj8900_palette[offset];
}

WRITE8_HANDLER( nbmj8900_palette_type3_w )
{
	int r, g, b;

	nbmj8900_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((nbmj8900_palette[offset + 1] & 0x0f) >> 0);
	g = ((nbmj8900_palette[offset + 0] & 0xf0) >> 4);
	b = ((nbmj8900_palette[offset + 0] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}
#endif

WRITE8_HANDLER( nbmj8900_clutsel_w )
{
	nbmj8900_clutsel = data;
}

READ8_HANDLER( nbmj8900_clut_r )
{
	return nbmj8900_clut[offset];
}

WRITE8_HANDLER( nbmj8900_clut_w )
{
	nbmj8900_clut[((nbmj8900_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
WRITE8_HANDLER( nbmj8900_blitter_w )
{
	switch (offset)
	{
		case 0x00:	blitter_src_addr = (blitter_src_addr & 0xff00) | data; break;
		case 0x01:	blitter_src_addr = (blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:	blitter_destx = data; break;
		case 0x03:	blitter_desty = data; break;
		case 0x04:	blitter_sizex = data; break;
		case 0x05:	blitter_sizey = data;
					/* writing here also starts the blit */
					nbmj8900_gfxdraw(space->machine);
					break;
		case 0x06:	blitter_direction_x = (data & 0x01) ? 1 : 0;
					blitter_direction_y = (data & 0x02) ? 1 : 0;
					nbmj8900_flipscreen = (data & 0x04) ? 1 : 0;
					nbmj8900_dispflag = (data & 0x08) ? 0 : 1;
					if (gfxdraw_mode) nbmj8900_vramflip(space->machine, 1);
					nbmj8900_vramflip(space->machine, 0);
					break;
		case 0x07:	break;
	}
}

WRITE8_HANDLER( nbmj8900_scrolly_w )
{
	nbmj8900_scrolly = data;
}

WRITE8_HANDLER( nbmj8900_vramsel_w )
{
	/* protection - not sure about this */
	nb1413m3_sndromrgntag = (data & 0x20) ? "protdata" : "voice";

	nbmj8900_vram = data;
}

WRITE8_HANDLER( nbmj8900_romsel_w )
{
	nbmj8900_gfxrom = (data & 0x0f);

	if ((0x20000 * nbmj8900_gfxrom) > (memory_region_length(space->machine, "gfx") - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		nbmj8900_gfxrom &= (memory_region_length(space->machine, "gfx") / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
void nbmj8900_vramflip(running_machine *machine, int vram)
{
	static int nbmj8900_flipscreen_old = 0;
	int x, y;
	unsigned char color1, color2;
	unsigned char *vidram;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	if (nbmj8900_flipscreen == nbmj8900_flipscreen_old) return;

	vidram = vram ? nbmj8900_videoram1 : nbmj8900_videoram0;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = vidram[(y * width) + x];
			color2 = vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)];
			vidram[(y * width) + x] = color2;
			vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)] = color1;
		}
	}

	nbmj8900_flipscreen_old = nbmj8900_flipscreen;
	nbmj8900_screen_refresh = 1;
}


static void update_pixel0(running_machine *machine, int x, int y)
{
	UINT8 color = nbmj8900_videoram0[(y * screen_width) + x];
	*BITMAP_ADDR16(nbmj8900_tmpbitmap0, y, x) = machine->pens[color];
}

static void update_pixel1(running_machine *machine, int x, int y)
{
	UINT8 color = nbmj8900_videoram1[(y * screen_width) + x];
	*BITMAP_ADDR16(nbmj8900_tmpbitmap1, y, x) = machine->pens[color];
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void nbmj8900_gfxdraw(running_machine *machine)
{
	unsigned char *GFX = memory_region(machine, "gfx");

	int x, y;
	int dx1, dx2, dy1, dy2;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	unsigned char color, color1, color2;
	int gfxaddr;

	nb1413m3_busyctr = 0;

	startx = blitter_destx + blitter_sizex;
	starty = blitter_desty + blitter_sizey;

	if (blitter_direction_x)
	{
		sizex = blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = blitter_sizex;
		skipx = -1;
	}

	if (blitter_direction_y)
	{
		sizey = blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		sizey = blitter_sizey;
		skipy = -1;
	}

	gfxaddr = (nbmj8900_gfxrom << 17) + (blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (memory_region_length(machine, "gfx") - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr &= (memory_region_length(machine, "gfx") - 1);
			}

			color = GFX[gfxaddr++];

			// for hanamomo
			if ((nb1413m3_type == NB1413M3_HANAMOMO) && ((gfxaddr >= 0x20000) && (gfxaddr < 0x28000)))
			{
				color |= ((color & 0x0f) << 4);
			}

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;

			if (gfxdraw_mode)
			{
				// 2 layer type
				dy1 = y & 0xff;
				dy2 = (y + nbmj8900_scrolly) & 0xff;
			}
			else
			{
				// 1 layer type
				dy1 = (y + nbmj8900_scrolly) & 0xff;
				dy2 = 0;
			}

			if (!nbmj8900_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy1 ^= 0xff;
				dy2 ^= 0xff;
			}

			if (blitter_direction_x)
			{
				// flip
				color1 = (color & 0x0f) >> 0;
				color2 = (color & 0xf0) >> 4;
			}
			else
			{
				// normal
				color1 = (color & 0xf0) >> 4;
				color2 = (color & 0x0f) >> 0;
			}

			color1 = nbmj8900_clut[((nbmj8900_clutsel & 0x7f) << 4) + color1];
			color2 = nbmj8900_clut[((nbmj8900_clutsel & 0x7f) << 4) + color2];

			if ((!gfxdraw_mode) || (nbmj8900_vram & 0x01))
			{
				// layer 1
				if (color1 != 0xff)
				{
					nbmj8900_videoram0[(dy1 * screen_width) + dx1] = color1;
					update_pixel0(machine, dx1, dy1);
				}
				if (color2 != 0xff)
				{
					nbmj8900_videoram0[(dy1 * screen_width) + dx2] = color2;
					update_pixel0(machine, dx2, dy1);
				}
			}
			if (gfxdraw_mode && (nbmj8900_vram & 0x02))
			{
				// layer 2
				if (nbmj8900_vram & 0x08)
				{
					// transparent enable
					if (color1 != 0xff)
					{
						nbmj8900_videoram1[(dy2 * screen_width) + dx1] = color1;
						update_pixel1(machine, dx1, dy2);
					}
					if (color2 != 0xff)
					{
						nbmj8900_videoram1[(dy2 * screen_width) + dx2] = color2;
						update_pixel1(machine, dx2, dy2);
					}
				}
				else
				{
					// transparent disable
					nbmj8900_videoram1[(dy2 * screen_width) + dx1] = color1;
					update_pixel1(machine, dx1, dy2);
					nbmj8900_videoram1[(dy2 * screen_width) + dx2] = color2;
					update_pixel1(machine, dx2, dy2);
				}
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	timer_set(machine, attotime_mul(ATTOTIME_IN_NSEC(2500), nb1413m3_busyctr), NULL, 0, blitter_timer_callback);
}

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj8900_2layer )
{
	screen_width = machine->primary_screen->width();
	screen_height = machine->primary_screen->height();

	nbmj8900_tmpbitmap0 = machine->primary_screen->alloc_compatible_bitmap();
	nbmj8900_tmpbitmap1 = machine->primary_screen->alloc_compatible_bitmap();
	nbmj8900_videoram0 = auto_alloc_array(machine, UINT8, screen_width * screen_height);
	nbmj8900_videoram1 = auto_alloc_array(machine, UINT8, screen_width * screen_height);
	nbmj8900_palette = auto_alloc_array(machine, UINT8, 0x200);
	nbmj8900_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(nbmj8900_videoram0, 0xff, (screen_width * screen_height * sizeof(UINT8)));
	memset(nbmj8900_videoram1, 0xff, (screen_width * screen_height * sizeof(UINT8)));
//  machine->pens[0x07f] = 0xff;    /* palette_transparent_pen */
	gfxdraw_mode = 1;
}

/******************************************************************************


******************************************************************************/
VIDEO_UPDATE( nbmj8900 )
{
	int x, y;

	if (nbmj8900_screen_refresh)
	{
		nbmj8900_screen_refresh = 0;
		for (y = 0; y < screen_height; y++)
		{
			for (x = 0; x < screen_width; x++)
			{
				update_pixel0(screen->machine, x, y);
			}
		}
		if (gfxdraw_mode)
		{
			for (y = 0; y < screen_height; y++)
			{
				for (x = 0; x < screen_width; x++)
				{
					update_pixel1(screen->machine, x, y);
				}
			}
		}
	}

	if (nbmj8900_dispflag)
	{
		static int scrolly;
		if (!nbmj8900_flipscreen) scrolly =   nbmj8900_scrolly;
		else                      scrolly = (-nbmj8900_scrolly) & 0xff;

		if (gfxdraw_mode)
		{
			copyscrollbitmap(bitmap, nbmj8900_tmpbitmap0, 0, 0, 0, 0, cliprect);
			copyscrollbitmap_trans(bitmap, nbmj8900_tmpbitmap1, 0, 0, 1, &scrolly, cliprect, 0xff);
		}
		else
		{
			copyscrollbitmap(bitmap, nbmj8900_tmpbitmap0, 0, 0, 1, &scrolly, cliprect);
		}
	}
	else
	{
		bitmap_fill(bitmap, 0, 0);
	}
	return 0;
}
