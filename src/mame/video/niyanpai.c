/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/

#include "driver.h"
#include "deprecat.h"


#define	VRAM_MAX	3


static int niyanpai_scrollx[VRAM_MAX], niyanpai_scrolly[VRAM_MAX];
static int blitter_destx[VRAM_MAX], blitter_desty[VRAM_MAX];
static int blitter_sizex[VRAM_MAX], blitter_sizey[VRAM_MAX];
static int blitter_src_addr[VRAM_MAX];
static int blitter_direction_x[VRAM_MAX], blitter_direction_y[VRAM_MAX];
static int niyanpai_dispflag[VRAM_MAX];
static int niyanpai_flipscreen[VRAM_MAX];
static int niyanpai_clutmode[VRAM_MAX];
static int niyanpai_transparency[VRAM_MAX];
static int niyanpai_clutsel[VRAM_MAX];
static int niyanpai_screen_refresh;
static int nb19010_busyctr;
static int nb19010_busyflag;

static mame_bitmap *niyanpai_tmpbitmap[VRAM_MAX];
static UINT16 *niyanpai_videoram[VRAM_MAX];
static UINT16 *niyanpai_videoworkram[VRAM_MAX];
static UINT16 *niyanpai_palette;
static UINT8 *niyanpai_clut[VRAM_MAX];


static void niyanpai_vramflip(int vram);
static void niyanpai_gfxdraw(int vram);


/******************************************************************************


******************************************************************************/
READ16_HANDLER( niyanpai_palette_r )
{
	return niyanpai_palette[offset];
}

WRITE16_HANDLER( niyanpai_palette_w )
{
	int r, g, b;
	int offs_h, offs_l;
	UINT16 oldword = niyanpai_palette[offset];
	UINT16 newword;

	COMBINE_DATA(&niyanpai_palette[offset]);
	newword = niyanpai_palette[offset];

	if (oldword != newword)
	{
		offs_h = (offset / 0x180);
		offs_l = (offset & 0x7f);

		if (ACCESSING_MSB16)
		{
			r  = ((niyanpai_palette[(0x000 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			g  = ((niyanpai_palette[(0x080 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			b  = ((niyanpai_palette[(0x100 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);

			palette_set_color(Machine, ((offs_h << 8) + (offs_l << 1) + 0), MAKE_RGB(r, g, b));
		}

		if (ACCESSING_LSB16)
		{
			r  = ((niyanpai_palette[(0x000 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			g  = ((niyanpai_palette[(0x080 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			b  = ((niyanpai_palette[(0x100 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);

			palette_set_color(Machine, ((offs_h << 8) + (offs_l << 1) + 1), MAKE_RGB(r, g, b));
		}
	}
}

/******************************************************************************


******************************************************************************/
static int niyanpai_blitter_r(int vram, int offset)
{
	int ret;
	UINT8 *GFXROM = memory_region(REGION_GFX1);

	switch (offset)
	{
		case 0x00:	ret = 0xfe | ((nb19010_busyflag & 0x01) ^ 0x01); break;	// NB19010 Busy Flag
		case 0x01:	ret = GFXROM[blitter_src_addr[vram]]; break;			// NB19010 GFX-ROM Read
		default:	ret = 0xff; break;
	}

	return ret;
}

static void niyanpai_blitter_w(int vram, int offset, int data)
{
	switch (offset)
	{
		case 0x00:	blitter_direction_x[vram] = (data & 0x01) ? 1 : 0;
					blitter_direction_y[vram] = (data & 0x02) ? 1 : 0;
					niyanpai_clutmode[vram] = (data & 0x04) ? 1 : 0;
				//  if (data & 0x08) popmessage("Unknown GFX Flag!! (0x08)");
					niyanpai_transparency[vram] = (data & 0x10) ? 1 : 0;
				//  if (data & 0x20) popmessage("Unknown GFX Flag!! (0x20)");
					niyanpai_flipscreen[vram] = (data & 0x40) ? 0 : 1;
					niyanpai_dispflag[vram] = (data & 0x80) ? 1 : 0;
					niyanpai_vramflip(vram);
					break;
		case 0x01:	niyanpai_scrollx[vram] = (niyanpai_scrollx[vram] & 0x0100) | data; break;
		case 0x02:	niyanpai_scrollx[vram] = (niyanpai_scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x03:	niyanpai_scrolly[vram] = (niyanpai_scrolly[vram] & 0x0100) | data; break;
		case 0x04:	niyanpai_scrolly[vram] = (niyanpai_scrolly[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x05:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0xffff00) | data; break;
		case 0x06:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0xff00ff) | (data << 8); break;
		case 0x07:	blitter_src_addr[vram] = (blitter_src_addr[vram] & 0x00ffff) | (data << 16); break;
		case 0x08:	blitter_sizex[vram] = data; break;
		case 0x09:	blitter_sizey[vram] = data; break;
		case 0x0a:	blitter_destx[vram] = (blitter_destx[vram]  & 0xff00) | data; break;
		case 0x0b:	blitter_destx[vram] = (blitter_destx[vram]  & 0x00ff) | (data << 8); break;
		case 0x0c:	blitter_desty[vram] = (blitter_desty[vram]  & 0xff00) | data; break;
		case 0x0d:	blitter_desty[vram] = (blitter_desty[vram]  & 0x00ff) | (data << 8);
					niyanpai_gfxdraw(vram);
					break;
		default:	break;
	}
}

static void niyanpai_clutsel_w(int vram, int data)
{
	niyanpai_clutsel[vram] = data;
}

static void niyanpai_clut_w(int vram, int offset, int data)
{
	niyanpai_clut[vram][((niyanpai_clutsel[vram] & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
static void niyanpai_vramflip(int vram)
{
	static int niyanpai_flipscreen_old[VRAM_MAX] = { 0, 0, 0 };
	int x, y;
	UINT16 color1, color2;

	if (niyanpai_flipscreen[vram] == niyanpai_flipscreen_old[vram]) return;

	for (y = 0; y < (Machine->screen[0].height / 2); y++)
	{
		for (x = 0; x < Machine->screen[0].width; x++)
		{
			color1 = niyanpai_videoram[vram][(y * Machine->screen[0].width) + x];
			color2 = niyanpai_videoram[vram][((y ^ 0x1ff) * Machine->screen[0].width) + (x ^ 0x3ff)];
			niyanpai_videoram[vram][(y * Machine->screen[0].width) + x] = color2;
			niyanpai_videoram[vram][((y ^ 0x1ff) * Machine->screen[0].width) + (x ^ 0x3ff)] = color1;
		}
	}

	for (y = 0; y < (Machine->screen[0].height / 2); y++)
	{
		for (x = 0; x < Machine->screen[0].width; x++)
		{
			color1 = niyanpai_videoworkram[vram][(y * Machine->screen[0].width) + x];
			color2 = niyanpai_videoworkram[vram][((y ^ 0x1ff) * Machine->screen[0].width) + (x ^ 0x3ff)];
			niyanpai_videoworkram[vram][(y * Machine->screen[0].width) + x] = color2;
			niyanpai_videoworkram[vram][((y ^ 0x1ff) * Machine->screen[0].width) + (x ^ 0x3ff)] = color1;
		}
	}

	niyanpai_flipscreen_old[vram] = niyanpai_flipscreen[vram];
	niyanpai_screen_refresh = 1;
}

static void update_pixel(int vram, int x, int y)
{
	UINT16 color = niyanpai_videoram[vram][(y * Machine->screen[0].width) + x];
	*BITMAP_ADDR16(niyanpai_tmpbitmap[vram], y, x) = Machine->pens[color];
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb19010_busyflag = 1;
}

static void niyanpai_gfxdraw(int vram)
{
	UINT8 *GFX = memory_region(REGION_GFX1);

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT16 color, color1, color2;
	int gfxaddr;

	nb19010_busyctr = 0;

	if (niyanpai_clutmode[vram])
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

	gfxaddr = ((blitter_src_addr[vram] + 2) & 0x00ffffff);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (memory_region_length(REGION_GFX1) - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d", gfxaddr, startx, starty, sizex,sizey);
				logerror("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d\n", gfxaddr, startx, starty, sizex,sizey);
#endif
				gfxaddr &= (memory_region_length(REGION_GFX1) - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!niyanpai_flipscreen[vram])
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

			if (niyanpai_clutmode[vram])
			{
				// clut256 mode

				if (niyanpai_clutsel[vram] & 0x80)
				{
					// clut256 mode 1st(low)
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1] &= 0x00f0;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1] |= color1 & 0x0f;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2] &= 0x00f0;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1] &= 0x000f;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1] |= (color1 & 0x0f) << 4;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2] &= 0x000f;
					niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2] |= (color2 & 0x0f) << 4;

		//          niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1] += niyanpai_clut[vram][(niyanpai_clutsel[vram] * 0x10)];
		//          niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2] += niyanpai_clut[vram][(niyanpai_clutsel[vram] * 0x10)];
				}

				color1 = niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx1];
				color2 = niyanpai_videoworkram[vram][(dy * Machine->screen[0].width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = niyanpai_clut[vram][(niyanpai_clutsel[vram] * 0x10) + color1];
				color2 = niyanpai_clut[vram][(niyanpai_clutsel[vram] * 0x10) + color2];
			}

			color1 |= (0x0100 * vram);
			color2 |= (0x0100 * vram);

			if (((color1 & 0x00ff) != 0x00ff) || (!niyanpai_transparency[vram]))
			{
				niyanpai_videoram[vram][(dy * Machine->screen[0].width) + dx1] = color1;
				update_pixel(vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!niyanpai_transparency[vram]))
			{
				niyanpai_videoram[vram][(dy * Machine->screen[0].width) + dx2] = color2;
				update_pixel(vram, dx2, dy);
			}

			nb19010_busyctr++;
		}
	}

	if (niyanpai_clutmode[vram])
	{
		// NB22090 clut256 mode
		blitter_src_addr[vram] = gfxaddr;
	}

	nb19010_busyflag = 0;
	timer_set(ATTOTIME_IN_NSEC(1650 * nb19010_busyctr), NULL, 0, blitter_timer_callback);
}

/******************************************************************************


******************************************************************************/
WRITE16_HANDLER( niyanpai_blitter_0_w )	{ niyanpai_blitter_w(0, offset, data); }
WRITE16_HANDLER( niyanpai_blitter_1_w )	{ niyanpai_blitter_w(1, offset, data); }
WRITE16_HANDLER( niyanpai_blitter_2_w )	{ niyanpai_blitter_w(2, offset, data); }

READ16_HANDLER( niyanpai_blitter_0_r )	{ return niyanpai_blitter_r(0, offset); }
READ16_HANDLER( niyanpai_blitter_1_r )	{ return niyanpai_blitter_r(1, offset); }
READ16_HANDLER( niyanpai_blitter_2_r )	{ return niyanpai_blitter_r(2, offset); }

WRITE16_HANDLER( niyanpai_clut_0_w )	{ niyanpai_clut_w(0, offset, data); }
WRITE16_HANDLER( niyanpai_clut_1_w )	{ niyanpai_clut_w(1, offset, data); }
WRITE16_HANDLER( niyanpai_clut_2_w )	{ niyanpai_clut_w(2, offset, data); }

WRITE16_HANDLER( niyanpai_clutsel_0_w )	{ niyanpai_clutsel_w(0, data); }
WRITE16_HANDLER( niyanpai_clutsel_1_w )	{ niyanpai_clutsel_w(1, data); }
WRITE16_HANDLER( niyanpai_clutsel_2_w )	{ niyanpai_clutsel_w(2, data); }

/******************************************************************************


******************************************************************************/
VIDEO_START( niyanpai )
{
	niyanpai_tmpbitmap[0] = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	niyanpai_tmpbitmap[1] = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	niyanpai_tmpbitmap[2] = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	niyanpai_videoram[0] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_videoram[1] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_videoram[2] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_videoworkram[0] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_videoworkram[1] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_videoworkram[2] = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(short));
	niyanpai_palette = auto_malloc(0x480 * sizeof(short));
	niyanpai_clut[0] = auto_malloc(0x1000 * sizeof(char));
	niyanpai_clut[1] = auto_malloc(0x1000 * sizeof(char));
	niyanpai_clut[2] = auto_malloc(0x1000 * sizeof(char));
	memset(niyanpai_videoram[0], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	memset(niyanpai_videoram[1], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	memset(niyanpai_videoram[2], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	memset(niyanpai_videoworkram[0], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	memset(niyanpai_videoworkram[1], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	memset(niyanpai_videoworkram[2], 0x0000, (machine->screen[0].width * machine->screen[0].height * sizeof(short)));
	nb19010_busyflag = 1;
}

/******************************************************************************


******************************************************************************/
VIDEO_UPDATE( niyanpai )
{
	int i;
	int x, y;
	int scrollx[3], scrolly[3];

	if (niyanpai_screen_refresh)
	{
		niyanpai_screen_refresh = 0;

		for (y = 0; y < machine->screen[0].height; y++)
		{
			for (x = 0; x < machine->screen[0].width; x++)
			{
				update_pixel(0, x, y);
				update_pixel(1, x, y);
				update_pixel(2, x, y);
			}
		}
	}

	for (i = 0; i < 3; i++)
	{
		if (niyanpai_flipscreen[i])
		{
			scrollx[i] = (((-niyanpai_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = (-niyanpai_scrolly[i]) & 0x1ff;
		}
		else
		{
			scrollx[i] = (((-niyanpai_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = niyanpai_scrolly[i] & 0x1ff;
		}
	}

	if (niyanpai_dispflag[0])
	{
		copyscrollbitmap(bitmap, niyanpai_tmpbitmap[0], 1, &scrollx[0], 1, &scrolly[0], &machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
	}
	else
	{
		fillbitmap(bitmap, machine->pens[0x00ff], 0);
	}

	if (niyanpai_dispflag[1])
	{
		copyscrollbitmap(bitmap, niyanpai_tmpbitmap[1], 1, &scrollx[1], 1, &scrolly[1], &machine->screen[0].visarea, TRANSPARENCY_PEN, machine->pens[0x01ff]);
	}

	if (niyanpai_dispflag[2])
	{
		copyscrollbitmap(bitmap, niyanpai_tmpbitmap[2], 1, &scrollx[2], 1, &scrolly[2], &machine->screen[0].visarea, TRANSPARENCY_PEN, machine->pens[0x02ff]);
	}
	return 0;
}
