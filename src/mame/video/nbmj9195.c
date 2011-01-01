/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -
    Special thanks to Tatsuyuki Satoh

******************************************************************************/

#include "emu.h"
#include "includes/nbmj9195.h"


#define	VRAM_MAX	2

#define	SCANLINE_MIN	0
#define	SCANLINE_MAX	512


static int nbmj9195_scrollx[VRAM_MAX], nbmj9195_scrolly[VRAM_MAX];
static int nbmj9195_scrollx_raster[VRAM_MAX][SCANLINE_MAX];
static int nbmj9195_scanline[VRAM_MAX];
static int blitter_destx[VRAM_MAX], blitter_desty[VRAM_MAX];
static int blitter_sizex[VRAM_MAX], blitter_sizey[VRAM_MAX];
static int blitter_src_addr[VRAM_MAX];
static int blitter_direction_x[VRAM_MAX], blitter_direction_y[VRAM_MAX];
static int nbmj9195_dispflag[VRAM_MAX];
static int nbmj9195_flipscreen[VRAM_MAX];
static int nbmj9195_clutmode[VRAM_MAX];
static int nbmj9195_transparency[VRAM_MAX];
static int nbmj9195_clutsel;
static int nbmj9195_screen_refresh;
static int nbmj9195_gfxflag2;
static int gfxdraw_mode;
static int nb19010_busyctr;
static int nb19010_busyflag;

static bitmap_t *nbmj9195_tmpbitmap[VRAM_MAX];
static UINT16 *nbmj9195_videoram[VRAM_MAX];
static UINT16 *nbmj9195_videoworkram[VRAM_MAX];
static UINT8 *nbmj9195_palette, *nbmj9195_nb22090_palette;
static UINT8 *nbmj9195_clut[VRAM_MAX];


static void nbmj9195_vramflip(running_machine *machine, int vram);
static void nbmj9195_gfxdraw(running_machine *machine, int vram);


/******************************************************************************


******************************************************************************/
READ8_HANDLER( nbmj9195_palette_r )
{
	return nbmj9195_palette[offset];
}

WRITE8_HANDLER( nbmj9195_palette_w )
{
	int r, g, b;

	nbmj9195_palette[offset] = data;

	if (offset & 1)
	{
		offset &= 0x1fe;

		r = ((nbmj9195_palette[offset + 0] & 0x0f) >> 0);
		g = ((nbmj9195_palette[offset + 0] & 0xf0) >> 4);
		b = ((nbmj9195_palette[offset + 1] & 0x0f) >> 0);

		palette_set_color_rgb(space->machine, (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

READ8_HANDLER( nbmj9195_nb22090_palette_r )
{
	return nbmj9195_nb22090_palette[offset];
}

WRITE8_HANDLER( nbmj9195_nb22090_palette_w )
{
	int r, g, b;
	int offs_h, offs_l;

	nbmj9195_nb22090_palette[offset] = data;

	offs_h = (offset / 0x0300);
	offs_l = (offset & 0x00ff);

	r = nbmj9195_nb22090_palette[(0x000 + (offs_h * 0x300) + offs_l)];
	g = nbmj9195_nb22090_palette[(0x100 + (offs_h * 0x300) + offs_l)];
	b = nbmj9195_nb22090_palette[(0x200 + (offs_h * 0x300) + offs_l)];

	palette_set_color(space->machine, ((offs_h * 0x100) + offs_l), MAKE_RGB(r, g, b));
}

/******************************************************************************


******************************************************************************/
static int nbmj9195_blitter_r(running_machine *machine, int vram, int offset)
{
	int ret;
	UINT8 *GFXROM = machine->region("gfx1")->base();

	switch (offset)
	{
		case 0x00:	ret = 0xfe | ((nb19010_busyflag & 0x01) ^ 0x01); break;	// NB19010 Busy Flag
		case 0x01:	ret = GFXROM[blitter_src_addr[vram]]; break;			// NB19010 GFX-ROM Read
		default:	ret = 0xff; break;
	}

	return ret;
}

static void nbmj9195_blitter_w(running_machine *machine, int vram, int offset, int data)
{
	int new_line;

	switch (offset)
	{
		case 0x00:	blitter_direction_x[vram] = (data & 0x01) ? 1 : 0;
					blitter_direction_y[vram] = (data & 0x02) ? 1 : 0;
					nbmj9195_clutmode[vram] = (data & 0x04) ? 1 : 0;
				//  if (data & 0x08) popmessage("Unknown GFX Flag!! (0x08)");
					nbmj9195_transparency[vram] = (data & 0x10) ? 1 : 0;
				//  if (data & 0x20) popmessage("Unknown GFX Flag!! (0x20)");
					nbmj9195_flipscreen[vram] = (data & 0x40) ? 0 : 1;
					nbmj9195_dispflag[vram] = (data & 0x80) ? 1 : 0;
					nbmj9195_vramflip(machine, vram);
					break;
		case 0x01:	nbmj9195_scrollx[vram] = (nbmj9195_scrollx[vram] & 0x0100) | data; break;
		case 0x02:	nbmj9195_scrollx[vram] = (nbmj9195_scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100);
					new_line = machine->primary_screen->vpos();
					if (nbmj9195_flipscreen[vram])
					{
						for ( ; nbmj9195_scanline[vram] < new_line; nbmj9195_scanline[vram]++)
							nbmj9195_scrollx_raster[vram][nbmj9195_scanline[vram]] = (((-nbmj9195_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					else
					{
						for ( ; nbmj9195_scanline[vram] < new_line; nbmj9195_scanline[vram]++)
							nbmj9195_scrollx_raster[vram][(nbmj9195_scanline[vram] ^ 0x1ff)] = (((-nbmj9195_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					break;
		case 0x03:	nbmj9195_scrolly[vram] = (nbmj9195_scrolly[vram] & 0x0100) | data; break;
		case 0x04:	nbmj9195_scrolly[vram] = (nbmj9195_scrolly[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x05:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0xffff00) | data; break;
		case 0x06:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0xff00ff) | (data << 8); break;
		case 0x07:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0x00ffff) | (data << 16); break;
		case 0x08:	blitter_sizex[vram] = data; break;
		case 0x09:	blitter_sizey[vram] = data; break;
		case 0x0a:	blitter_destx[vram] = (blitter_destx[vram]  & 0xff00) | data; break;
		case 0x0b:	blitter_destx[vram] = (blitter_destx[vram]  & 0x00ff) | (data << 8); break;
		case 0x0c:	blitter_desty[vram] = (blitter_desty[vram]  & 0xff00) | data; break;
		case 0x0d:	blitter_desty[vram] = (blitter_desty[vram]  & 0x00ff) | (data << 8);
					nbmj9195_gfxdraw(machine, vram);
					break;
		default:	break;
	}
}

void nbmj9195_clutsel_w(int data)
{
	nbmj9195_clutsel = data;
}

static void nbmj9195_clut_w(int vram, int offset, int data)
{
	nbmj9195_clut[vram][((nbmj9195_clutsel & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

void nbmj9195_gfxflag2_w(int data)
{
	nbmj9195_gfxflag2 = data;
}

/******************************************************************************


******************************************************************************/
static void nbmj9195_vramflip(running_machine *machine, int vram)
{
	static int nbmj9195_flipscreen_old[VRAM_MAX] = { 0, 0 };
	int x, y;
	UINT16 color1, color2;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	if (nbmj9195_flipscreen[vram] == nbmj9195_flipscreen_old[vram]) return;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = nbmj9195_videoram[vram][(y * width) + x];
			color2 = nbmj9195_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			nbmj9195_videoram[vram][(y * width) + x] = color2;
			nbmj9195_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	if (gfxdraw_mode == 2)
	{
		for (y = 0; y < (height / 2); y++)
		{
			for (x = 0; x < width; x++)
			{
				color1 = nbmj9195_videoworkram[vram][(y * width) + x];
				color2 = nbmj9195_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
				nbmj9195_videoworkram[vram][(y * width) + x] = color2;
				nbmj9195_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
			}
		}
	}

	nbmj9195_flipscreen_old[vram] = nbmj9195_flipscreen[vram];
	nbmj9195_screen_refresh = 1;
}

static void update_pixel(running_machine *machine, int vram, int x, int y)
{
	UINT16 color = nbmj9195_videoram[vram][(y * machine->primary_screen->width()) + x];
	*BITMAP_ADDR16(nbmj9195_tmpbitmap[vram], y, x) = color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb19010_busyflag = 1;
}

static void nbmj9195_gfxdraw(running_machine *machine, int vram)
{
	UINT8 *GFX = machine->region("gfx1")->base();
	int width = machine->primary_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT16 color, color1, color2;
	int gfxaddr, gfxlen;

	nb19010_busyctr = 0;

	if ((gfxdraw_mode == 2) && (nbmj9195_clutmode[vram]))
	{
		// NB22090 clut256 mode
		blitter_sizex[vram] = GFX[((blitter_src_addr[vram] + 0) & 0x00ffffff)];
		blitter_sizey[vram] = GFX[((blitter_src_addr[vram] + 1) & 0x00ffffff)];
	}

	if (blitter_direction_x[vram])
	{
		startx = blitter_destx[vram];
		sizex = blitter_sizex[vram];
		skipx = 1;
	}
	else
	{
		startx = blitter_destx[vram] + blitter_sizex[vram];
		sizex = blitter_sizex[vram];
		skipx = -1;
	}

	if (blitter_direction_y[vram])
	{
		starty = blitter_desty[vram];
		sizey = blitter_sizey[vram];
		skipy = 1;
	}
	else
	{
		starty = blitter_desty[vram] + blitter_sizey[vram];
		sizey = blitter_sizey[vram];
		skipy = -1;
	}

	gfxlen = machine->region("gfx1")->bytes();
	gfxaddr = ((blitter_src_addr[vram] + 2) & 0x00ffffff);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d", gfxaddr, startx, starty, sizex,sizey);
				logerror("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d\n", gfxaddr, startx, starty, sizex,sizey);
#endif
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!nbmj9195_flipscreen[vram])
			{
				dx1 ^= 0x3ff;
				dx2 ^= 0x3ff;
				dy ^= 0x1ff;
			}

			if (blitter_direction_x[vram])
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

			if ((gfxdraw_mode == 2) && (nbmj9195_clutmode[vram]))
			{
				// clut256 mode

				if (nbmj9195_gfxflag2 & 0xc0)
				{
					// clut256 mode 1st(low)
					nbmj9195_videoworkram[vram][(dy * width) + dx1] &= 0x00f0;
					nbmj9195_videoworkram[vram][(dy * width) + dx1] |= color1 & 0x0f;
					nbmj9195_videoworkram[vram][(dy * width) + dx2] &= 0x00f0;
					nbmj9195_videoworkram[vram][(dy * width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					nbmj9195_videoworkram[vram][(dy * width) + dx1] &= 0x000f;
					nbmj9195_videoworkram[vram][(dy * width) + dx1] |= (color1 & 0x0f) << 4;
					nbmj9195_videoworkram[vram][(dy * width) + dx2] &= 0x000f;
					nbmj9195_videoworkram[vram][(dy * width) + dx2] |= (color2 & 0x0f) << 4;

					nbmj9195_videoworkram[vram][(dy * width) + dx1] += nbmj9195_clut[vram][(nbmj9195_clutsel * 0x10)];
					nbmj9195_videoworkram[vram][(dy * width) + dx2] += nbmj9195_clut[vram][(nbmj9195_clutsel * 0x10)];
				}

				color1 = nbmj9195_videoworkram[vram][(dy * width) + dx1];
				color2 = nbmj9195_videoworkram[vram][(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = nbmj9195_clut[vram][(nbmj9195_clutsel * 0x10) + color1];
				color2 = nbmj9195_clut[vram][(nbmj9195_clutsel * 0x10) + color2];
			}

			if (gfxdraw_mode == 2)
			{
				color1 |= (0x0100 * vram);
				color2 |= (0x0100 * vram);
			}

			if (((color1 & 0x00ff) != 0x00ff) || (!nbmj9195_transparency[vram]))
			{
				nbmj9195_videoram[vram][(dy * width) + dx1] = color1;
				update_pixel(machine, vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!nbmj9195_transparency[vram]))
			{
				nbmj9195_videoram[vram][(dy * width) + dx2] = color2;
				update_pixel(machine, vram, dx2, dy);
			}

			nb19010_busyctr++;
		}
	}

	if ((gfxdraw_mode == 2) && (nbmj9195_clutmode[vram]))
	{
		// NB22090 clut256 mode
		blitter_src_addr[vram] = gfxaddr;
	}

	nb19010_busyflag = 0;

	/* 1650ns per count */
	timer_set(machine, ATTOTIME_IN_NSEC(nb19010_busyctr * 1650), NULL, 0, blitter_timer_callback);
}

/******************************************************************************


******************************************************************************/
WRITE8_HANDLER( nbmj9195_blitter_0_w )	{ nbmj9195_blitter_w(space->machine, 0, offset, data); }
WRITE8_HANDLER( nbmj9195_blitter_1_w )	{ nbmj9195_blitter_w(space->machine, 1, offset, data); }

READ8_HANDLER( nbmj9195_blitter_0_r )	{ return nbmj9195_blitter_r(space->machine, 0, offset); }
READ8_HANDLER( nbmj9195_blitter_1_r )	{ return nbmj9195_blitter_r(space->machine, 1, offset); }

WRITE8_HANDLER( nbmj9195_clut_0_w )		{ nbmj9195_clut_w(0, offset, data); }
WRITE8_HANDLER( nbmj9195_clut_1_w )		{ nbmj9195_clut_w(1, offset, data); }

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj9195_1layer )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	nbmj9195_tmpbitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	nbmj9195_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_palette = auto_alloc_array(machine, UINT8, 0x200);
	nbmj9195_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	nbmj9195_scanline[0] = nbmj9195_scanline[1] = SCANLINE_MIN;
	nb19010_busyflag = 1;
	gfxdraw_mode = 0;
}

VIDEO_START( nbmj9195_2layer )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	nbmj9195_tmpbitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	nbmj9195_tmpbitmap[1] = machine->primary_screen->alloc_compatible_bitmap();
	nbmj9195_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_videoram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_palette = auto_alloc_array(machine, UINT8, 0x200);
	nbmj9195_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	nbmj9195_clut[1] = auto_alloc_array(machine, UINT8, 0x1000);
	nbmj9195_scanline[0] = nbmj9195_scanline[1] = SCANLINE_MIN;
	nb19010_busyflag = 1;
	gfxdraw_mode = 1;
}

VIDEO_START( nbmj9195_nb22090 )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	nbmj9195_tmpbitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	nbmj9195_tmpbitmap[1] = machine->primary_screen->alloc_compatible_bitmap();
	nbmj9195_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_videoram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_videoworkram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_videoworkram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	nbmj9195_nb22090_palette = auto_alloc_array(machine, UINT8, 0xc00);
	nbmj9195_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	nbmj9195_clut[1] = auto_alloc_array(machine, UINT8, 0x1000);
	nbmj9195_scanline[0] = nbmj9195_scanline[1] = SCANLINE_MIN;
	nb19010_busyflag = 1;
	gfxdraw_mode = 2;
}

/******************************************************************************


******************************************************************************/
VIDEO_UPDATE( nbmj9195 )
{
	int i;
	int x, y;
	int scrolly[2];

	if (nbmj9195_screen_refresh)
	{
		int width = screen->width();
		int height = screen->height();

		nbmj9195_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				update_pixel(screen->machine, 0, x, y);

				if (gfxdraw_mode)
					update_pixel(screen->machine, 1, x, y);
			}
	}

	for (i = 0; i < 2; i++)
	{
		if (nbmj9195_flipscreen[i])
		{
			for ( ; nbmj9195_scanline[i] < SCANLINE_MAX; nbmj9195_scanline[i]++)
			{
				nbmj9195_scrollx_raster[i][nbmj9195_scanline[i]] = (((-nbmj9195_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = (-nbmj9195_scrolly[i]) & 0x1ff;
		}
		else
		{
			for ( ; nbmj9195_scanline[i] < SCANLINE_MAX; nbmj9195_scanline[i]++)
			{
				nbmj9195_scrollx_raster[i][(nbmj9195_scanline[i] ^ 0x1ff)] = (((-nbmj9195_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = nbmj9195_scrolly[i] & 0x1ff;
		}
		nbmj9195_scanline[i] = SCANLINE_MIN;
	}

	if (nbmj9195_dispflag[0])
		// nbmj9195 1layer
		copyscrollbitmap(bitmap, nbmj9195_tmpbitmap[0], SCANLINE_MAX, nbmj9195_scrollx_raster[0], 1, &scrolly[0], cliprect);
	else
		bitmap_fill(bitmap, 0, 0x0ff);

	if (nbmj9195_dispflag[1])
	{
		if (gfxdraw_mode == 1)
			// nbmj9195 2layer
			copyscrollbitmap_trans(bitmap, nbmj9195_tmpbitmap[1], SCANLINE_MAX, nbmj9195_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x0ff);

		if (gfxdraw_mode == 2)
			// nbmj9195 nb22090 2layer
			copyscrollbitmap_trans(bitmap, nbmj9195_tmpbitmap[1], SCANLINE_MAX, nbmj9195_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x1ff);
	}
	return 0;
}
