/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi 2000/06/07 -

******************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "nb1413m3.h"


static int blitter_destx, blitter_desty;
static int blitter_sizex, blitter_sizey;
static int blitter_src_addr;
static int pastelg_gfxrom;
static int pastelg_dispflag;
static int pastelg_flipscreen;
static int blitter_direction_x, blitter_direction_y;
static int pastelg_palbank;

static UINT8 *pastelg_videoram;
static UINT8 *pastelg_clut;


static void pastelg_vramflip(void);
static void pastelg_gfxdraw(void);


/******************************************************************************


******************************************************************************/
PALETTE_INIT( pastelg )
{
	int i;
	int bit0, bit1, bit2, bit3, r, g, b;

	for (i = 0; i < machine->config->total_colors; i++)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->config->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( pastelg_clut_w )
{
	pastelg_clut[offset] = data;
}

/******************************************************************************


******************************************************************************/
int pastelg_blitter_src_addr_r(void)
{
	return blitter_src_addr;
}

WRITE8_HANDLER( pastelg_blitter_w )
{
	switch (offset)
	{
		case 0: blitter_src_addr = (blitter_src_addr & 0xff00) | data; break;
		case 1: blitter_src_addr = (blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2: blitter_destx = data; break;
		case 3: blitter_desty = data; break;
		case 4: blitter_sizex = data; break;
		case 5: blitter_sizey = data;
				/* writing here also starts the blit */
				pastelg_gfxdraw();
				break;
		case 6:	blitter_direction_x = (data & 0x01) ? 1 : 0;
				blitter_direction_y = (data & 0x02) ? 1 : 0;
				pastelg_flipscreen = (data & 0x04) ? 0 : 1;
				pastelg_dispflag = (data & 0x08) ? 0 : 1;
				pastelg_vramflip();
				break;
	}
}

WRITE8_HANDLER( pastelg_romsel_w )
{
	pastelg_gfxrom = ((data & 0xc0) >> 6);
	pastelg_palbank = ((data & 0x10) >> 4);
	nb1413m3_sndrombank1_w(machine, 0, data);

	if ((pastelg_gfxrom << 16) > (memory_region_length(REGION_GFX1) - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		pastelg_gfxrom &= (memory_region_length(REGION_GFX1) / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
static void pastelg_vramflip(void)
{
	static int pastelg_flipscreen_old = 0;
	int x, y;
	UINT8 color1, color2;

	if (pastelg_flipscreen == pastelg_flipscreen_old) return;

	for (y = 0; y < Machine->screen[0].height; y++)
	{
		for (x = 0; x < Machine->screen[0].width; x++)
		{
			color1 = pastelg_videoram[(y * Machine->screen[0].width) + x];
			color2 = pastelg_videoram[((y ^ 0xff) * Machine->screen[0].width) + (x ^ 0xff)];
			pastelg_videoram[(y * Machine->screen[0].width) + x] = color2;
			pastelg_videoram[((y ^ 0xff) * Machine->screen[0].width) + (x ^ 0xff)] = color1;
		}
	}

	pastelg_flipscreen_old = pastelg_flipscreen;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void pastelg_gfxdraw(void)
{
	UINT8 *GFX = memory_region(REGION_GFX1);

	int x, y;
	int dx, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	int readflag;
	int gfxaddr;
	UINT8 color;

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

	gfxaddr = (pastelg_gfxrom << 16) + blitter_src_addr;

	readflag = 0;

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (memory_region_length(REGION_GFX1) - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr = 0;
			}

			color = GFX[gfxaddr];

			dx = x & 0xff;
			dy = y & 0xff;

			if (pastelg_flipscreen)
			{
				dx ^= 0xff;
				dy ^= 0xff;
			}

			if (!readflag)
			{
				// 1st, 3rd, 5th, ... read
				color = (color & 0x0f);
			}
			else
			{
				// 2nd, 4th, 6th, ... read
				color = (color & 0xf0) >> 4;
				gfxaddr++;
			}

			readflag ^= 1;

			if (pastelg_clut[color] & 0xf0)
			{
				if (color)
				{
					color = ((pastelg_palbank * 0x10) + color);
					pastelg_videoram[(dy * Machine->screen[0].width) + dx] = color;
				}
			}
			else
			{
				color = ((pastelg_palbank * 0x10) + pastelg_clut[color]);
				pastelg_videoram[(dy * Machine->screen[0].width) + dx] = color;
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	timer_set(attotime_mul(ATTOTIME_IN_HZ(400000), nb1413m3_busyctr), NULL, 0, blitter_timer_callback);
}

/******************************************************************************


******************************************************************************/
VIDEO_START( pastelg )
{
	pastelg_videoram = auto_malloc(machine->screen[0].width * machine->screen[0].height * sizeof(UINT8));
	pastelg_clut = auto_malloc(0x10 * sizeof(UINT8));
	memset(pastelg_videoram, 0x00, (machine->screen[0].width * machine->screen[0].height * sizeof(UINT8)));
}

/******************************************************************************


******************************************************************************/
VIDEO_UPDATE( pastelg )
{
	if (pastelg_dispflag)
	{
		int x, y;

		for (y = 0; y < machine->screen[0].height; y++)
			for (x = 0; x < machine->screen[0].width; x++)
				*BITMAP_ADDR16(bitmap, y, x) = pastelg_videoram[(y * machine->screen[0].width) + x];
	}
	else
		fillbitmap(bitmap, 0, cliprect);

	return 0;
}
