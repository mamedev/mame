/***************************************************************************

  video/balsente.c

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "driver.h"
#include "balsente.h"


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 *local_videoram;
static UINT8 *sprite_data;
static UINT32 sprite_mask;
static UINT8 *sprite_bank[2];

static UINT8 palettebank_vis;



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( balsente )
{
	/* reset the system */
	palettebank_vis = 0;
	sprite_bank[0] = memory_region(REGION_GFX1);
	sprite_bank[1] = memory_region(REGION_GFX1) + 0x10000;

	/* allocate a local copy of video RAM */
	local_videoram = auto_malloc(256 * 256);

	/* determine sprite size */
	sprite_data = memory_region(REGION_GFX1);
	sprite_mask = memory_region_length(REGION_GFX1) - 1;

	/* register for saving */
	state_save_register_global_pointer(local_videoram, 256 * 256);
	state_save_register_global(palettebank_vis);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_HANDLER( balsente_videoram_w )
{
	videoram[offset] = data;

	/* expand the two pixel values into two bytes */
	local_videoram[offset * 2 + 0] = data >> 4;
	local_videoram[offset * 2 + 1] = data & 15;
}



/*************************************
 *
 *  Palette banking
 *
 *************************************/

WRITE8_HANDLER( balsente_palette_select_w )
{
	/* only update if changed */
	if (palettebank_vis != (data & 3))
	{
		/* update the scanline palette */
		video_screen_update_partial(0, video_screen_get_vpos(0) - 1 + BALSENTE_VBEND);
		palettebank_vis = data & 3;
	}

	logerror("balsente_palette_select_w(%d) scanline=%d\n", data & 3, video_screen_get_vpos(0));
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE8_HANDLER( balsente_paletteram_w )
{
	int r, g, b;

	paletteram[offset] = data & 0x0f;

	r = paletteram[(offset & ~3) + 0];
	g = paletteram[(offset & ~3) + 1];
	b = paletteram[(offset & ~3) + 2];

	palette_set_color_rgb(Machine, offset / 4, pal4bit(r), pal4bit(g), pal4bit(b));
}



/*************************************
 *
 *  Sprite banking
 *
 *************************************/

WRITE8_HANDLER( shrike_sprite_select_w )
{
	if( sprite_data != sprite_bank[(data & 0x80 >> 7) ^ 1 ])
	{
		logerror( "shrike_sprite_select_w( 0x%02x )\n", data );
		video_screen_update_partial(0, video_screen_get_vpos(0) - 1 + BALSENTE_VBEND);
		sprite_data = sprite_bank[(data & 0x80 >> 7) ^ 1];
	}

	shrike_shared_6809_w( 1, data );
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_one_sprite(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8 *sprite)
{
	int flags = sprite[0];
	int image = sprite[1] | ((flags & 7) << 8);
	int ypos = sprite[2] + 17 + BALSENTE_VBEND;
	int xpos = sprite[3];
	UINT8 *src;
	int x, y;

	/* get a pointer to the source image */
	src = &sprite_data[(64 * image) & sprite_mask];
	if (flags & 0x80) src += 4 * 15;

	/* loop over y */
	for (y = 0; y < 16; y++, ypos = (ypos + 1) & 255)
	{
		if (ypos >= (16 + BALSENTE_VBEND) && ypos >= cliprect->min_y && ypos <= cliprect->max_y)
		{
			const pen_t *pens = &machine->pens[palettebank_vis * 256];
			UINT8 *old = &local_videoram[(ypos - BALSENTE_VBEND) * 256 + xpos];
			int currx = xpos;

			/* standard case */
			if (!(flags & 0x40))
			{
				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *src++;
					int left = ipixel & 0xf0;
					int right = (ipixel << 4) & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[right | old[1]];
					currx++;
				}
			}

			/* hflip case */
			else
			{
				src += 4;

				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *--src;
					int left = (ipixel << 4) & 0xf0;
					int right = ipixel & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[right | old[1]];
					currx++;
				}
				src += 4;
			}
		}
		else
			src += 4;
		if (flags & 0x80) src -= 2 * 4;
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

VIDEO_UPDATE( balsente )
{
	const pen_t *pens = &machine->pens[palettebank_vis * 256];
	int y, i;

	/* draw scanlines from the VRAM directly */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		draw_scanline8(bitmap, 0, y, 256, &local_videoram[(y - BALSENTE_VBEND) * 256], pens, -1);
	}

	/* draw the sprite images */
	for (i = 0; i < 40; i++)
		draw_one_sprite(machine, bitmap, cliprect, &spriteram[(0xe0 + i * 4) & 0xff]);

	return 0;
}
