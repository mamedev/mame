/***************************************************************************

    Kitco Crowns Golf hardware

***************************************************************************/

#include "driver.h"
#include "crgolf.h"


/* globals */
UINT8 *crgolf_color_select;
UINT8 *crgolf_screen_flip;
UINT8 *crgolf_screen_select;
UINT8 *crgolf_screenb_enable;
UINT8 *crgolf_screena_enable;


/* local variables */
static mame_bitmap *screena;
static mame_bitmap *screenb;
static mame_bitmap *highbit;



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_HANDLER( crgolf_videoram_bit0_w )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *dest = (UINT16 *)screen->base + screen->rowpixels * y + x;

	dest[0] = (dest[0] & ~0x01) | ((data >> 7) & 0x01);
	dest[1] = (dest[1] & ~0x01) | ((data >> 6) & 0x01);
	dest[2] = (dest[2] & ~0x01) | ((data >> 5) & 0x01);
	dest[3] = (dest[3] & ~0x01) | ((data >> 4) & 0x01);
	dest[4] = (dest[4] & ~0x01) | ((data >> 3) & 0x01);
	dest[5] = (dest[5] & ~0x01) | ((data >> 2) & 0x01);
	dest[6] = (dest[6] & ~0x01) | ((data >> 1) & 0x01);
	dest[7] = (dest[7] & ~0x01) | ((data >> 0) & 0x01);
}


WRITE8_HANDLER( crgolf_videoram_bit1_w )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *dest = (UINT16 *)screen->base + screen->rowpixels * y + x;

	dest[0] = (dest[0] & ~0x02) | ((data >> 6) & 0x02);
	dest[1] = (dest[1] & ~0x02) | ((data >> 5) & 0x02);
	dest[2] = (dest[2] & ~0x02) | ((data >> 4) & 0x02);
	dest[3] = (dest[3] & ~0x02) | ((data >> 3) & 0x02);
	dest[4] = (dest[4] & ~0x02) | ((data >> 2) & 0x02);
	dest[5] = (dest[5] & ~0x02) | ((data >> 1) & 0x02);
	dest[6] = (dest[6] & ~0x02) | ((data >> 0) & 0x02);
	dest[7] = (dest[7] & ~0x02) | ((data << 1) & 0x02);
}


WRITE8_HANDLER( crgolf_videoram_bit2_w )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *dest = (UINT16 *)screen->base + screen->rowpixels * y + x;

	dest[0] = (dest[0] & ~0x04) | ((data >> 5) & 0x04);
	dest[1] = (dest[1] & ~0x04) | ((data >> 4) & 0x04);
	dest[2] = (dest[2] & ~0x04) | ((data >> 3) & 0x04);
	dest[3] = (dest[3] & ~0x04) | ((data >> 2) & 0x04);
	dest[4] = (dest[4] & ~0x04) | ((data >> 1) & 0x04);
	dest[5] = (dest[5] & ~0x04) | ((data >> 0) & 0x04);
	dest[6] = (dest[6] & ~0x04) | ((data << 1) & 0x04);
	dest[7] = (dest[7] & ~0x04) | ((data << 2) & 0x04);
}



/*************************************
 *
 *  Video RAM reads
 *
 *************************************/

READ8_HANDLER( crgolf_videoram_bit0_r )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *source = (UINT16 *)screen->base + screen->rowpixels * y + x;

	return	((source[0] & 0x01) << 7) |
			((source[1] & 0x01) << 6) |
			((source[2] & 0x01) << 5) |
			((source[3] & 0x01) << 4) |
			((source[4] & 0x01) << 3) |
			((source[5] & 0x01) << 2) |
			((source[6] & 0x01) << 1) |
			((source[7] & 0x01) << 0);
}


READ8_HANDLER( crgolf_videoram_bit1_r )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *source = (UINT16 *)screen->base + screen->rowpixels * y + x;

	return	((source[0] & 0x02) << 6) |
			((source[1] & 0x02) << 5) |
			((source[2] & 0x02) << 4) |
			((source[3] & 0x02) << 3) |
			((source[4] & 0x02) << 2) |
			((source[5] & 0x02) << 1) |
			((source[6] & 0x02) << 0) |
			((source[7] & 0x02) >> 1);
}


READ8_HANDLER( crgolf_videoram_bit2_r )
{
	mame_bitmap *screen = (*crgolf_screen_select & 1) ? screenb : screena;
	int x = (offset % 32) * 8;
	int y = offset / 32;
	UINT16 *source = (UINT16 *)screen->base + screen->rowpixels * y + x;

	return	((source[0] & 0x04) << 5) |
			((source[1] & 0x04) << 4) |
			((source[2] & 0x04) << 3) |
			((source[3] & 0x04) << 2) |
			((source[4] & 0x04) << 1) |
			((source[5] & 0x04) << 0) |
			((source[6] & 0x04) >> 1) |
			((source[7] & 0x04) >> 2);
}



/*************************************
 *
 *  Color PROM decoding
 *
 *************************************/

PALETTE_INIT( crgolf )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

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
		bit0 = (*color_prom >> 6) & 0x01;
		bit1 = (*color_prom >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( crgolf )
{
	/* allocate temporary bitmaps */
	screena = auto_bitmap_alloc(256, 256, machine->screen[0].format);
	screenb = auto_bitmap_alloc(256, 256, machine->screen[0].format);
	highbit = auto_bitmap_alloc(256, 256, machine->screen[0].format);

	/* initialize the "high bit" bitmap */
	fillbitmap(screena, 0, NULL);
	fillbitmap(screenb, 8, NULL);
	fillbitmap(highbit, 16, NULL);

	/* register for save states */
	state_save_register_bitmap("video", 0, "screena", screena);
	state_save_register_bitmap("video", 0, "screenb", screenb);
	state_save_register_bitmap("video", 0, "highbit", highbit);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( crgolf )
{
	int flip = *crgolf_screen_flip & 1;

	/* draw screen b if enabled */
	if (~*crgolf_screenb_enable & 1)
		copybitmap(bitmap, screenb, flip, flip, 0, 0, cliprect, TRANSPARENCY_NONE, 0);
	else
		fillbitmap(bitmap, 8, cliprect);

	/* draw screen a if enabled */
	if (~*crgolf_screena_enable & 1)
		copybitmap(bitmap, screena, flip, flip, 0, 0, cliprect, TRANSPARENCY_PEN, 0);

	/* apply the color select bit */
	if (*crgolf_color_select)
		copybitmap(bitmap, highbit, 0, 0, 0, 0, cliprect, TRANSPARENCY_BLEND, 0);
	return 0;
}
