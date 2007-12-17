/***************************************************************************

    Video emulation for Astro Invader, Space Intruder et al

***************************************************************************/

#include "driver.h"
#include "includes/astinvad.h"


static UINT8 spaceint_color;
static UINT8 screen_red;


void astinvad_set_screen_red(int data)
{
	screen_red = data;
}


WRITE8_HANDLER( spaceint_color_w )
{
	spaceint_color = data & 0x0f;
}


WRITE8_HANDLER( spaceint_videoram_w )
{
	videoram[offset] = data;
	colorram[offset] = spaceint_color;
}


VIDEO_START( spaceint )
{
	colorram = auto_malloc(videoram_size);
}



static void plot_byte(mame_bitmap *bitmap, UINT8 y, UINT8 x, UINT8 data, UINT8 color)
{
	int i;

	pen_t fore_pen = MAKE_RGB(pal1bit(color >> 0), pal1bit(color >> 2), pal1bit(color >> 1));

	for (i = 0; i < 8; i++)
	{
		pen_t pen = (data & 0x01) ? fore_pen : RGB_BLACK;

		if (flip_screen)
			*BITMAP_ADDR32(bitmap, 255 - y, 255 - x) = pen;
		else
			*BITMAP_ADDR32(bitmap, y, x) = pen;

		x = x + 1;
		data = data >> 1;
	}
}


VIDEO_UPDATE( astinvad )
{
	if (screen_red)
	{
		fillbitmap(bitmap, MAKE_RGB(pal1bit(1), pal1bit(0), pal1bit(0)), cliprect);
	}
	else
	{
		offs_t offs;

		for (offs = 0; offs < videoram_size; offs++)
		{
			UINT8 color;
			UINT8 data = videoram[offs];

			UINT8 y = offs >> 5;
			UINT8 x = offs << 3;

			offs_t n = ((offs >> 3) & ~0x1f) | (offs & 0x1f);

			if (flip_screen)
				color = (memory_region(REGION_PROMS)[n] >> 4) & 0x07;
			else
				color = (memory_region(REGION_PROMS)[(~n + 0x80) & 0x3ff]) & 0x07;

			plot_byte(bitmap, y, x, data, color);
		}
	}

	return 0;
}


VIDEO_UPDATE( spcking2 )
{
	if (screen_red)
	{
		fillbitmap(bitmap, MAKE_RGB(pal1bit(1), pal1bit(0), pal1bit(0)), cliprect);
	}
	else
	{
		offs_t offs;

		for (offs = 0; offs < videoram_size; offs++)
		{
			UINT8 color;
			UINT8 data = videoram[offs];

			UINT8 y = offs >> 5;
			UINT8 x = offs << 3;

			offs_t n = ((offs >> 3) & ~0x1f) | (offs & 0x1f);

			if (flip_screen)
				color = (memory_region(REGION_PROMS)[n] >> 4) & 0x07;
			else
				color = (memory_region(REGION_PROMS)[n ^ 0x03ff]) & 0x07;

			plot_byte(bitmap, y, x, data, color);
		}
	}

	return 0;
}


VIDEO_UPDATE( spaceint )
{
	offs_t offs;

	for (offs = 0; offs < videoram_size; offs++)
	{
		UINT8 data = videoram[offs];
		UINT8 color = colorram[offs];

		UINT8 y = ~offs;
		UINT8 x = offs >> 8 << 3;

		/* this is almost certainly wrong */
		offs_t n = ((offs >> 5) & 0xf0) | color;
		color = memory_region(REGION_PROMS)[n] & 0x07;

		plot_byte(bitmap, y, x, data, color);
	}

	return 0;
}
