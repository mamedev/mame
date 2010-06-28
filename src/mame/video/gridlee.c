/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

    Based on the Bally/Sente SAC system

***************************************************************************/

#include "emu.h"
#include "includes/gridlee.h"
#include "includes/balsente.h"


/*************************************
 *
 *  Globals
 *
 *************************************/

UINT8 gridlee_cocktail_flip;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 *local_videoram;

static UINT8 palettebank_vis;



/*************************************
 *
 *  Color PROM conversion
 *
 *************************************/

PALETTE_INIT( gridlee )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0x0000]),pal4bit(color_prom[0x0800]),pal4bit(color_prom[0x1000]));
		color_prom++;
	}
}



/*************************************
 *
 *  Video system restart
 *
 *************************************/

static STATE_POSTLOAD( expand_pixels )
{
    int offset = 0;

    for(offset = 0; offset < 0x77ff; offset++)
    {
        local_videoram[offset * 2 + 0] = machine->generic.videoram.u8[offset] >> 4;
        local_videoram[offset * 2 + 1] = machine->generic.videoram.u8[offset] & 15;
    }
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( gridlee )
{
	/* allocate a local copy of video RAM */
	local_videoram = auto_alloc_array_clear(machine, UINT8, 256 * 256);

	/* reset the palette */
	palettebank_vis = 0;

    state_save_register_global(machine, gridlee_cocktail_flip);
    state_save_register_global(machine, palettebank_vis);
    state_save_register_postload(machine, expand_pixels, NULL);
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

WRITE8_HANDLER( gridlee_cocktail_flip_w )
{
	gridlee_cocktail_flip = data & 1;
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_HANDLER( gridlee_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;

	/* expand the two pixel values into two bytes */
	local_videoram[offset * 2 + 0] = data >> 4;
	local_videoram[offset * 2 + 1] = data & 15;
}



/*************************************
 *
 *  Palette banking
 *
 *************************************/

WRITE8_HANDLER( gridlee_palette_select_w )
{
	/* update the scanline palette */
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos() - 1 + BALSENTE_VBEND);
	palettebank_vis = data & 0x3f;
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

/* all the BALSENTE_VBEND adjustments are needed because the hardware has a seperate counting chain
   to address the video memory instead of using the video chain directly */

VIDEO_UPDATE( gridlee )
{
	const pen_t *pens = &screen->machine->pens[palettebank_vis * 32];
	UINT8 *gfx;
	int x, y, i;

	/* draw scanlines from the VRAM directly */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		/* non-flipped: draw directly from the bitmap */
		if (!gridlee_cocktail_flip)
			draw_scanline8(bitmap, 0, y, 256, &local_videoram[(y - BALSENTE_VBEND) * 256], pens + 16);

		/* flipped: x-flip the scanline into a temp buffer and draw that */
		else
		{
			int srcy = BALSENTE_VBSTART - 1 - y;
			UINT8 temp[256];
			int xx;

			for (xx = 0; xx < 256; xx++)
				temp[xx] = local_videoram[srcy * 256 + 255 - xx];
			draw_scanline8(bitmap, 0, y, 256, temp, pens + 16);
		}
	}

	/* draw the sprite images */
	gfx = memory_region(screen->machine, "gfx1");
	for (i = 0; i < 32; i++)
	{
		UINT8 *sprite = screen->machine->generic.spriteram.u8 + i * 4;
		UINT8 *src;
		int image = sprite[0];
		int ypos = sprite[2] + 17 + BALSENTE_VBEND;
		int xpos = sprite[3];

		/* get a pointer to the source image */
		src = &gfx[64 * image];

		/* loop over y */
		for (y = 0; y < 16; y++, ypos = (ypos + 1) & 255)
		{
			int currxor = 0;

			/* adjust for flip */
			if (gridlee_cocktail_flip)
			{
				ypos = 271 - ypos;
				currxor = 0xff;
			}

			if (ypos >= (16 + BALSENTE_VBEND) && ypos >= cliprect->min_y && ypos <= cliprect->max_y)
			{
				int currx = xpos;

				/* loop over x */
				for (x = 0; x < 4; x++)
				{
					int ipixel = *src++;
					int left = ipixel >> 4;
					int right = ipixel & 0x0f;

					/* left pixel */
					if (left && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx ^ currxor) = pens[left];
					currx++;

					/* right pixel */
					if (right && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx ^ currxor) = pens[right];
					currx++;
				}
			}
			else
				src += 4;

			/* de-adjust for flip */
			if (gridlee_cocktail_flip)
				ypos = 271 - ypos;
		}
	}
	return 0;
}
