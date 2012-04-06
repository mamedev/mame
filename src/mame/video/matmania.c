/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/

#include "emu.h"
#include "includes/matmania.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mat Mania is unusual in that it has both PROMs and RAM to control the
  palette. PROMs are used for characters and background tiles, RAM for
  sprites.
  I don't know for sure how the PROMs are connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( matmania )
{
	int i;

	for (i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[64], 0);
		bit1 = BIT(color_prom[64], 1);
		bit2 = BIT(color_prom[64], 2);
		bit3 = BIT(color_prom[64], 3);
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}



WRITE8_MEMBER(matmania_state::matmania_paletteram_w)
{
	int bit0, bit1, bit2, bit3, val;
	int r, g, b;
	int offs2;

	m_paletteram[offset] = data;
	offs2 = offset & 0x0f;

	val = m_paletteram[offs2];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x10];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x20];
	bit0 = BIT(val, 0);
	bit1 = BIT(val, 1);
	bit2 = BIT(val, 2);
	bit3 = BIT(val, 3);
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(machine(),offs2 + 64,MAKE_RGB(r,g,b));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( matmania )
{
	matmania_state *state = machine.driver_data<matmania_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	/* Mat Mania has a virtual screen twice as large as the visible screen */
	state->m_tmpbitmap  = auto_bitmap_ind16_alloc(machine, width, 2 * height);
	state->m_tmpbitmap2 = auto_bitmap_ind16_alloc(machine, width, 2 * height);
}



SCREEN_UPDATE_IND16( matmania )
{
	matmania_state *state = screen.machine().driver_data<matmania_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;


	/* Update the tiles in the left tile ram bank */
	for (offs = state->m_videoram_size - 1; offs >= 0; offs--)
	{
		int sx = 15 - offs / 32;
		int sy = offs % 32;

		drawgfx_opaque(*state->m_tmpbitmap, state->m_tmpbitmap->cliprect(), screen.machine().gfx[1],
				state->m_videoram[offs] + ((state->m_colorram[offs] & 0x08) << 5),
				(state->m_colorram[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16 * sx, 16 * sy);
	}

	/* Update the tiles in the right tile ram bank */
	for (offs = state->m_videoram3_size - 1; offs >= 0; offs--)
	{
		int sx = 15 - offs / 32;
		int sy = offs % 32;

		drawgfx_opaque(*state->m_tmpbitmap2, state->m_tmpbitmap2->cliprect(), screen.machine().gfx[1],
				state->m_videoram3[offs] + ((state->m_colorram3[offs] & 0x08) << 5),
				(state->m_colorram3[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}

	/* copy the temporary bitmap to the screen */
	{
		int scrolly = -*state->m_scroll;
		if (state->m_pageselect[0] & 0x01) // maniach sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap, *state->m_tmpbitmap2, 0, 0, 1, &scrolly, cliprect);
		else
			copyscrollbitmap(bitmap, *state->m_tmpbitmap, 0, 0, 1, &scrolly, cliprect);
	}


	/* Draw the sprites */
	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2],
					spriteram[offs + 1] + ((spriteram[offs] & 0xf0) << 4),
					(spriteram[offs] & 0x08) >> 3,
					spriteram[offs] & 0x04, spriteram[offs] & 0x02,
					239 - spriteram[offs + 3],(240 - spriteram[offs + 2]) & 0xff,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = state->m_videoram2_size - 1; offs >= 0; offs--)
	{
		int sx = 31 - offs / 32;
		int sy = offs % 32;

		drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0],
				state->m_videoram2[offs] + 256 * (state->m_colorram2[offs] & 0x07),
				(state->m_colorram2[offs] & 0x30) >> 4,
				0,0,
				8*sx,8*sy,0);
	}
	return 0;
}

SCREEN_UPDATE_IND16( maniach )
{
	matmania_state *state = screen.machine().driver_data<matmania_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;


	/* Update the tiles in the left tile ram bank */
	for (offs = state->m_videoram_size - 1; offs >= 0; offs--)
	{
		int sx = 15 - offs / 32;
		int sy = offs % 32;

		drawgfx_opaque(*state->m_tmpbitmap, state->m_tmpbitmap->cliprect(), screen.machine().gfx[1],
				state->m_videoram[offs] + ((state->m_colorram[offs] & 0x03) << 8),
				(state->m_colorram[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}

	/* Update the tiles in the right tile ram bank */
	for (offs = state->m_videoram3_size - 1; offs >= 0; offs--)
	{
		int sx = 15 - offs / 32;
		int sy = offs % 32;

		drawgfx_opaque(*state->m_tmpbitmap2, state->m_tmpbitmap2->cliprect(), screen.machine().gfx[1],
				state->m_videoram3[offs] + ((state->m_colorram3[offs] & 0x03) << 8),
				(state->m_colorram3[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scrolly = -*state->m_scroll;

		if (state->m_pageselect[0] & 0x01) // this sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap, *state->m_tmpbitmap2, 0, 0, 1, &scrolly, cliprect);
		else
			copyscrollbitmap(bitmap, *state->m_tmpbitmap, 0, 0, 1, &scrolly, cliprect);
	}


	/* Draw the sprites */
	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[2],
					spriteram[offs+1] + ((spriteram[offs] & 0xf0) << 4),
					(spriteram[offs] & 0x08) >> 3,
					spriteram[offs] & 0x04,spriteram[offs] & 0x02,
					239 - spriteram[offs+3],(240 - spriteram[offs+2]) & 0xff,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = state->m_videoram2_size - 1; offs >= 0; offs--)
	{
		int sx = 31 - offs / 32;
		int sy = offs % 32;

		drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0],
				state->m_videoram2[offs] + 256 * (state->m_colorram2[offs] & 0x07),
				(state->m_colorram2[offs] & 0x30) >> 4,
				0,0,
				8*sx,8*sy,0);
	}
	return 0;
}
