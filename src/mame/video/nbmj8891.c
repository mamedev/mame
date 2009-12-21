/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "driver.h"
#include "includes/nb1413m3.h"
#include "includes/nbmj8891.h"


static int nbmj8891_scrolly;
static int blitter_destx, blitter_desty;
static int blitter_sizex, blitter_sizey;
static int blitter_src_addr;
static int blitter_direction_x, blitter_direction_y;
static int nbmj8891_vram;
static int nbmj8891_gfxrom;
static int nbmj8891_dispflag;
static int nbmj8891_flipscreen;
static int nbmj8891_clutsel;
static int nbmj8891_screen_refresh;
static int gfxdraw_mode;

static bitmap_t *nbmj8891_tmpbitmap0, *nbmj8891_tmpbitmap1;
static UINT8 *nbmj8891_videoram0, *nbmj8891_videoram1;
static UINT8 *nbmj8891_palette;
static UINT8 *nbmj8891_clut;


static void nbmj8891_vramflip(running_machine *machine, int vram);
static void nbmj8891_gfxdraw(running_machine *machine);


/******************************************************************************


******************************************************************************/
READ8_HANDLER( nbmj8891_palette_type1_r )
{
	return nbmj8891_palette[offset];
}

WRITE8_HANDLER( nbmj8891_palette_type1_w )
{
	int r, g, b;

	nbmj8891_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((nbmj8891_palette[offset + 0] & 0x0f) >> 0);
	g = ((nbmj8891_palette[offset + 1] & 0xf0) >> 4);
	b = ((nbmj8891_palette[offset + 1] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_HANDLER( nbmj8891_palette_type2_r )
{
	return nbmj8891_palette[offset];
}

WRITE8_HANDLER( nbmj8891_palette_type2_w )
{
	int r, g, b;

	nbmj8891_palette[offset] = data;

	if (!(offset & 0x100)) return;

	offset &= 0x0ff;

	r = ((nbmj8891_palette[offset + 0x000] & 0x0f) >> 0);
	g = ((nbmj8891_palette[offset + 0x000] & 0xf0) >> 4);
	b = ((nbmj8891_palette[offset + 0x100] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset & 0x0ff), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_HANDLER( nbmj8891_palette_type3_r )
{
	return nbmj8891_palette[offset];
}

WRITE8_HANDLER( nbmj8891_palette_type3_w )
{
	int r, g, b;

	nbmj8891_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((nbmj8891_palette[offset + 1] & 0x0f) >> 0);
	g = ((nbmj8891_palette[offset + 0] & 0xf0) >> 4);
	b = ((nbmj8891_palette[offset + 0] & 0x0f) >> 0);

	palette_set_color_rgb(space->machine, (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_HANDLER( nbmj8891_clutsel_w )
{
	nbmj8891_clutsel = data;
}

READ8_HANDLER( nbmj8891_clut_r )
{
	return nbmj8891_clut[offset];
}

WRITE8_HANDLER( nbmj8891_clut_w )
{
	nbmj8891_clut[((nbmj8891_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
WRITE8_HANDLER( nbmj8891_blitter_w )
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
					nbmj8891_gfxdraw(space->machine);
					break;
		case 0x06:	blitter_direction_x = (data & 0x01) ? 1 : 0;
					blitter_direction_y = (data & 0x02) ? 1 : 0;
					nbmj8891_flipscreen = (data & 0x04) ? 1 : 0;
					nbmj8891_dispflag = (data & 0x08) ? 0 : 1;
					if (gfxdraw_mode) nbmj8891_vramflip(space->machine, 1);
					nbmj8891_vramflip(space->machine, 0);
					break;
		case 0x07:	break;
	}
}

WRITE8_HANDLER( nbmj8891_taiwanmb_blitter_w )
{
	switch (offset)
	{
		case 0:	blitter_src_addr = (blitter_src_addr & 0xff00) | data; break;
		case 1:	blitter_src_addr = (blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2:	blitter_destx = data; break;
		case 3:	blitter_desty = data; break;
		case 4:	blitter_sizex = (data - 1) & 0xff; break;
		case 5:	blitter_sizey = (data - 1) & 0xff; break;
	}
}

WRITE8_HANDLER( nbmj8891_taiwanmb_gfxdraw_w )
{
//  nbmj8891_gfxdraw(space->machine);
}

WRITE8_HANDLER( nbmj8891_taiwanmb_gfxflag_w )
{
	nbmj8891_flipscreen = (data & 0x04) ? 1 : 0;

	nbmj8891_vramflip(space->machine, 0);
}

WRITE8_HANDLER( nbmj8891_taiwanmb_mcu_w )
{
	static int param_old[0x10];
	static int param_cnt = 0;

	param_old[param_cnt & 0x0f] = data;

	if (data == 0x00)
	{
		blitter_direction_x = 0;
		blitter_direction_y = 0;
		blitter_destx = 0;
		blitter_desty = 0;
		blitter_sizex = 0;
		blitter_sizey = 0;
		nbmj8891_dispflag = 0;
	}

/*
    if (data == 0x02)
    {
        if (param_old[(param_cnt - 1) & 0x0f] == 0x18)
        {
            nbmj8891_dispflag = 1;
        }
        else if (param_old[(param_cnt - 1) & 0x0f] == 0x1a)
        {
            nbmj8891_dispflag = 0;
        }
    }
*/

	if (data == 0x04)
	{
		// CLUT Transfer?
	}

	if (data == 0x12)
	{
		if (param_old[(param_cnt - 1) & 0x0f] == 0x08)
		{
			blitter_direction_x = 1;
			blitter_direction_y = 0;
			blitter_destx += blitter_sizex + 1;
			blitter_desty += 0;
			blitter_sizex ^= 0xff;
			blitter_sizey ^= 0x00;
		}
		else if (param_old[(param_cnt - 1) & 0x0f] == 0x0a)
		{
			blitter_direction_x = 0;
			blitter_direction_y = 1;
			blitter_destx += 0;
			blitter_desty += blitter_sizey + 1;
			blitter_sizex ^= 0x00;
			blitter_sizey ^= 0xff;
		}
		else if (param_old[(param_cnt - 1) & 0x0f] == 0x0c)
		{
			blitter_direction_x = 1;
			blitter_direction_y = 1;
			blitter_destx += blitter_sizex + 1;
			blitter_desty += blitter_sizey + 1;
			blitter_sizex ^= 0xff;
			blitter_sizey ^= 0xff;
		}
		else if (param_old[(param_cnt - 1) & 0x0f] == 0x0e)
		{
			blitter_direction_x = 0;
			blitter_direction_y = 0;
			blitter_destx += 0;
			blitter_desty += 0;
			blitter_sizex ^= 0x00;
			blitter_sizey ^= 0x00;
		}

		nbmj8891_gfxdraw(space->machine);
	}

//  blitter_direction_x = 0;                // for debug
//  blitter_direction_y = 0;                // for debug
	nbmj8891_dispflag = 1;					// for debug

	param_cnt++;
}

WRITE8_HANDLER( nbmj8891_scrolly_w )
{
	nbmj8891_scrolly = data;
}

WRITE8_HANDLER( nbmj8891_vramsel_w )
{
	/* protection - not sure about this */
	nb1413m3_sndromrgntag = (data & 0x20) ? "protection" : "voice";

	nbmj8891_vram = data;
}

WRITE8_HANDLER( nbmj8891_romsel_w )
{
	int gfxlen = memory_region_length(space->machine, "gfx1");
	nbmj8891_gfxrom = (data & 0x0f);

	if ((0x20000 * nbmj8891_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		nbmj8891_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
void nbmj8891_vramflip(running_machine *machine, int vram)
{
	static int nbmj8891_flipscreen_old = 0;
	int x, y;
	UINT8 color1, color2;
	UINT8 *vidram;

	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	if (nbmj8891_flipscreen == nbmj8891_flipscreen_old) return;

	vidram = vram ? nbmj8891_videoram1 : nbmj8891_videoram0;

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

	nbmj8891_flipscreen_old = nbmj8891_flipscreen;
	nbmj8891_screen_refresh = 1;
}


static void update_pixel0(running_machine *machine, int x, int y)
{
	UINT8 color = nbmj8891_videoram0[(y * video_screen_get_width(machine->primary_screen)) + x];
	*BITMAP_ADDR16(nbmj8891_tmpbitmap0, y, x) = color;
}

static void update_pixel1(running_machine *machine, int x, int y)
{
	UINT8 color = nbmj8891_videoram1[(y * video_screen_get_width(machine->primary_screen)) + x];
	*BITMAP_ADDR16(nbmj8891_tmpbitmap1, y, x) = (color == 0x7f) ? 0xff : color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void nbmj8891_gfxdraw(running_machine *machine)
{
	UINT8 *GFX = memory_region(machine, "gfx1");
	int width = video_screen_get_width(machine->primary_screen);

	int x, y;
	int dx1, dx2, dy1, dy2;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT8 color, color1, color2;
	int gfxaddr, gfxlen;

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

	gfxlen = memory_region_length(machine, "gfx1");
	gfxaddr = (nbmj8891_gfxrom << 17) + (blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr &= (gfxlen - 1);
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
				dy2 = (y + nbmj8891_scrolly) & 0xff;
			}
			else
			{
				// 1 layer type
				dy1 = (y + nbmj8891_scrolly) & 0xff;
				dy2 = 0;
			}

			if (!nbmj8891_flipscreen)
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

			color1 = nbmj8891_clut[((nbmj8891_clutsel & 0x7f) << 4) + color1];
			color2 = nbmj8891_clut[((nbmj8891_clutsel & 0x7f) << 4) + color2];

			if ((!gfxdraw_mode) || (nbmj8891_vram & 0x01))
			{
				// layer 1
				if (color1 != 0xff)
				{
					nbmj8891_videoram0[(dy1 * width) + dx1] = color1;
					update_pixel0(machine, dx1, dy1);
				}
				if (color2 != 0xff)
				{
					nbmj8891_videoram0[(dy1 * width) + dx2] = color2;
					update_pixel0(machine, dx2, dy1);
				}
			}
			if (gfxdraw_mode && (nbmj8891_vram & 0x02))
			{
				// layer 2
				if (nbmj8891_vram & 0x08)
				{
					// transparent enable
					if (color1 != 0xff)
					{
						nbmj8891_videoram1[(dy2 * width) + dx1] = color1;
						update_pixel1(machine, dx1, dy2);
					}
					if (color2 != 0xff)
					{
						nbmj8891_videoram1[(dy2 * width) + dx2] = color2;
						update_pixel1(machine, dx2, dy2);
					}
				}
				else
				{
					// transparent disable
					nbmj8891_videoram1[(dy2 * width) + dx1] = color1;
					update_pixel1(machine, dx1, dy2);
					nbmj8891_videoram1[(dy2 * width) + dx2] = color2;
					update_pixel1(machine, dx2, dy2);
				}
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	timer_set(machine, attotime_mul(ATTOTIME_IN_HZ(400000), nb1413m3_busyctr), NULL, 0, blitter_timer_callback);
}

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj8891_1layer )
{
	UINT8 *CLUT = memory_region(machine, "protection");
	int i;
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	nbmj8891_tmpbitmap0 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	nbmj8891_videoram0 = auto_alloc_array(machine, UINT8, width * height);
	nbmj8891_palette = auto_alloc_array(machine, UINT8, 0x200);
	nbmj8891_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(nbmj8891_videoram0, 0xff, (width * height * sizeof(char)));
	gfxdraw_mode = 0;

	if (nb1413m3_type == NB1413M3_TAIWANMB)
		for (i = 0; i < 0x0800; i++) nbmj8891_clut[i] = CLUT[i];
}

VIDEO_START( nbmj8891_2layer )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	nbmj8891_tmpbitmap0 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	nbmj8891_tmpbitmap1 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	nbmj8891_videoram0 = auto_alloc_array(machine, UINT8, width * height);
	nbmj8891_videoram1 = auto_alloc_array(machine, UINT8, width * height);
	nbmj8891_palette = auto_alloc_array(machine, UINT8, 0x200);
	nbmj8891_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(nbmj8891_videoram0, 0xff, (width * height * sizeof(UINT8)));
	memset(nbmj8891_videoram1, 0xff, (width * height * sizeof(UINT8)));
	gfxdraw_mode = 1;
}

/******************************************************************************


******************************************************************************/
VIDEO_UPDATE( nbmj8891 )
{
	int x, y;

	if (nbmj8891_screen_refresh)
	{
		int width = video_screen_get_width(screen);
		int height = video_screen_get_height(screen);

		nbmj8891_screen_refresh = 0;
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel0(screen->machine, x, y);

		if (gfxdraw_mode)
			for (y = 0; y < height; y++)
				for (x = 0; x < width; x++)
					update_pixel1(screen->machine, x, y);
	}

	if (nbmj8891_dispflag)
	{
		static int scrolly;
		if (!nbmj8891_flipscreen) scrolly =   nbmj8891_scrolly;
		else                      scrolly = (-nbmj8891_scrolly) & 0xff;

		if (gfxdraw_mode)
		{
			copyscrollbitmap      (bitmap, nbmj8891_tmpbitmap0, 0, 0, 0, 0, cliprect);
			copyscrollbitmap_trans(bitmap, nbmj8891_tmpbitmap1, 0, 0, 1, &scrolly, cliprect, 0xff);
		}
		else
			copyscrollbitmap(bitmap, nbmj8891_tmpbitmap0, 0, 0, 1, &scrolly, cliprect);
	}
	else
		bitmap_fill(bitmap, 0, 0xff);

	return 0;
}
