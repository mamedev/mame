/******************************************************************************

Ikki (c) 1985 Sun Electronics

Video hardware driver by Uki

    20/Jun/2001 -

******************************************************************************/

#include "driver.h"

UINT8 *ikki_scroll;

static bitmap_t *sprite_bitmap;
static UINT8 ikki_flipscreen;
static int punch_through_pen;

PALETTE_INIT( ikki )
{
	int i;

	/* allocate the colortable - extra pen for the punch through pen */
	machine->colortable = colortable_alloc(machine, 0x101);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* sprites lookup table */
	for (i = 0; i < 0x200; i++)
	{
		UINT16 ctabentry = color_prom[i] ^ 0xff;

		if (((i & 0x07) == 0x07) && (ctabentry == 0))
		{
			/* punch through */
			punch_through_pen = i;
			ctabentry = 0x100;
		}

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* bg lookup table */
	for (i = 0x200; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i];
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( ikki_scrn_ctrl_w )
{
	ikki_flipscreen = (data >> 2) & 1;
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int y;
	offs_t offs;

	bitmap_fill(sprite_bitmap, cliprect, punch_through_pen);

	for (offs = 0; offs < 0x800; offs += 4)
	{
		int code = (spriteram[offs + 2] & 0x80) | (spriteram[offs + 1] >> 1);
		int color = spriteram[offs + 2] & 0x3f;

		int x = spriteram[offs + 3];
		    y = spriteram[offs + 0];

		if (ikki_flipscreen)
			x = 240 - x;
		else
			y = 224 - y;

		x = x & 0xff;
		y = y & 0xff;

		if (x > 248)
			x = x - 256;

		if (y > 240)
			y = y - 256;

		drawgfx_transmask(sprite_bitmap,cliprect, machine->gfx[1],
				code, color,
				ikki_flipscreen,ikki_flipscreen,
				x,y,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
	}

	/* copy the sprite bitmap into the main bitmap, skipping the transparent pixels */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT16 pen = *BITMAP_ADDR16(sprite_bitmap, y, x);

			if (colortable_entry_get_value(machine->colortable, pen) != 0x100)
				*BITMAP_ADDR16(bitmap, y, x) = pen;
		}
	}
}


VIDEO_START( ikki )
{
	sprite_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
}


VIDEO_UPDATE( ikki )
{
	offs_t offs;
	UINT8 *VIDEOATTR = memory_region( screen->machine, "user1" );

	/* draw bg layer */

	for (offs=0; offs<(videoram_size/2); offs++)
	{
		int color, bank;

		int sx = offs / 32;
		int sy = offs % 32;
		int y = sy*8;
		int x = sx*8;

		int d = VIDEOATTR[ sx ];

		switch (d)
		{
			case 0x02: /* scroll area */
				x = sx*8 - ikki_scroll[1];
				if (x<0)
					x=x+8*22;
				y = (sy*8 + ~ikki_scroll[0]) & 0xff;
				break;

			case 0x03: /* non-scroll area */
				break;

			case 0x00: /* sprite disable? */
				break;

			case 0x0d: /* sprite disable? */
				break;

			case 0x0b: /* non-scroll area (?) */
				break;

			case 0x0e: /* unknown */
				break;
		}

		if (ikki_flipscreen)
		{
			x = 248-x;
			y = 248-y;
		}

		color = videoram[offs*2];
		bank = (color & 0xe0) << 3;
		color = ((color & 0x1f)<<0) | ((color & 0x80) >> 2);

		drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],
			videoram[offs*2+1] + bank,
			color,
			ikki_flipscreen,ikki_flipscreen,
			x,y);
	}

	draw_sprites(screen->machine, bitmap, cliprect);

	/* mask sprites */

	for (offs=0; offs<(videoram_size/2); offs++)
	{
		int sx = offs / 32;
		int sy = offs % 32;

		int d = VIDEOATTR[ sx ];

		if ( (d == 0) || (d == 0x0d) )
		{
			int color, bank;

			int y = sy*8;
			int x = sx*8;

			if (ikki_flipscreen)
			{
				x = 248-x;
				y = 248-y;
			}

			color = videoram[offs*2];
			bank = (color & 0xe0) << 3;
			color = ((color & 0x1f)<<0) | ((color & 0x80) >> 2);

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],
				videoram[offs*2+1] + bank,
				color,
				ikki_flipscreen,ikki_flipscreen,
				x,y);
		}
	}

	return 0;
}
