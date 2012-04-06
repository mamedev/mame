/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

    Based on the Bally/Sente SAC system

***************************************************************************/

#include "emu.h"
#include "includes/gridlee.h"


/*************************************
 *
 *  Color PROM conversion
 *
 *************************************/

PALETTE_INIT( gridlee )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
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

static void expand_pixels(running_machine &machine)
{
	gridlee_state *state = machine.driver_data<gridlee_state>();
	UINT8 *videoram = state->m_videoram;
    int offset = 0;

    for(offset = 0; offset < 0x77ff; offset++)
    {
        state->m_local_videoram[offset * 2 + 0] = videoram[offset] >> 4;
        state->m_local_videoram[offset * 2 + 1] = videoram[offset] & 15;
    }
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( gridlee )
{
	gridlee_state *state = machine.driver_data<gridlee_state>();
	/* allocate a local copy of video RAM */
	state->m_local_videoram = auto_alloc_array_clear(machine, UINT8, 256 * 256);

	/* reset the palette */
	state->m_palettebank_vis = 0;

    state_save_register_global(machine, state->m_cocktail_flip);
    state_save_register_global(machine, state->m_palettebank_vis);
    machine.save().register_postload(save_prepost_delegate(FUNC(expand_pixels), &machine));
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

WRITE8_MEMBER(gridlee_state::gridlee_cocktail_flip_w)
{
	m_cocktail_flip = data & 1;
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_MEMBER(gridlee_state::gridlee_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;

	/* expand the two pixel values into two bytes */
	m_local_videoram[offset * 2 + 0] = data >> 4;
	m_local_videoram[offset * 2 + 1] = data & 15;
}



/*************************************
 *
 *  Palette banking
 *
 *************************************/

WRITE8_MEMBER(gridlee_state::gridlee_palette_select_w)
{
	/* update the scanline palette */
	machine().primary_screen->update_partial(machine().primary_screen->vpos() - 1 + GRIDLEE_VBEND);
	m_palettebank_vis = data & 0x3f;
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

/* all the GRIDLEE_VBEND adjustments are needed because the hardware has a separate counting chain
   to address the video memory instead of using the video chain directly */

SCREEN_UPDATE_IND16( gridlee )
{
	gridlee_state *state = screen.machine().driver_data<gridlee_state>();
	const pen_t *pens = &screen.machine().pens[state->m_palettebank_vis * 32];
	UINT8 *gfx;
	int x, y, i;

	/* draw scanlines from the VRAM directly */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		/* non-flipped: draw directly from the bitmap */
		if (!state->m_cocktail_flip)
			draw_scanline8(bitmap, 0, y, 256, &state->m_local_videoram[(y - GRIDLEE_VBEND) * 256], pens + 16);

		/* flipped: x-flip the scanline into a temp buffer and draw that */
		else
		{
			int srcy = GRIDLEE_VBSTART - 1 - y;
			UINT8 temp[256];
			int xx;

			for (xx = 0; xx < 256; xx++)
				temp[xx] = state->m_local_videoram[srcy * 256 + 255 - xx];
			draw_scanline8(bitmap, 0, y, 256, temp, pens + 16);
		}
	}

	/* draw the sprite images */
	gfx = screen.machine().region("gfx1")->base();
	for (i = 0; i < 32; i++)
	{
		UINT8 *sprite = state->m_spriteram + i * 4;
		UINT8 *src;
		int image = sprite[0];
		int ypos = sprite[2] + 17 + GRIDLEE_VBEND;
		int xpos = sprite[3];

		/* get a pointer to the source image */
		src = &gfx[64 * image];

		/* loop over y */
		for (y = 0; y < 16; y++, ypos = (ypos + 1) & 255)
		{
			int currxor = 0;

			/* adjust for flip */
			if (state->m_cocktail_flip)
			{
				ypos = 271 - ypos;
				currxor = 0xff;
			}

			if (ypos >= (16 + GRIDLEE_VBEND) && ypos >= cliprect.min_y && ypos <= cliprect.max_y)
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
						bitmap.pix16(ypos, currx ^ currxor) = pens[left];
					currx++;

					/* right pixel */
					if (right && currx >= 0 && currx < 256)
						bitmap.pix16(ypos, currx ^ currxor) = pens[right];
					currx++;
				}
			}
			else
				src += 4;

			/* de-adjust for flip */
			if (state->m_cocktail_flip)
				ypos = 271 - ypos;
		}
	}
	return 0;
}
