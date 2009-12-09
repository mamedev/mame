/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/timeplt.h"


#define NUM_PENS	(0x10)


/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE8_HANDLER( tutankhm_flip_screen_x_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;
	state->flip_x = data & 0x01;
}


WRITE8_HANDLER( tutankhm_flip_screen_y_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;
	state->flip_y = data & 0x01;
}


/*************************************
 *
 *  Palette management
 *
 *************************************/

static void get_pens( running_machine *machine, pen_t *pens )
{
	timeplt_state *state = (timeplt_state *)machine->driver_data;
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 data = state->paletteram[i];

		pens[i] = MAKE_RGB(pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( tutankhm )
{
	timeplt_state *state = (timeplt_state *)screen->machine->driver_data;
	int xorx = state->flip_x ? 255 : 0;
	int xory = state->flip_y ? 255 : 0;
	pen_t pens[NUM_PENS];
	int x, y;

	get_pens(screen->machine, pens);

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT32 *dst = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT8 effx = x ^ xorx;
			UINT8 yscroll = (effx < 192) ? *state->scroll : 0;
			UINT8 effy = (y ^ xory) + yscroll;
			UINT8 vrambyte = state->videoram[effy * 128 + effx / 2];
			UINT8 shifted = vrambyte >> (4 * (effx % 2));
			dst[x] = pens[shifted & 0x0f];
		}
	}

	return 0;
}



/* Juno First Blitter Hardware emulation

    Juno First can blit a 16x16 graphics which comes from un-memory mapped graphics roms

    $8070->$8071 specifies the destination NIBBLE address
    $8072->$8073 specifies the source NIBBLE address

    Depending on bit 0 of the source address either the source pixels will be copied to
    the destination address, or a zero will be written.

    Only source pixels which aren't 0 are copied or cleared.

    This allows the game to quickly clear the sprites from the screen

    TODO: Does bit 1 of the source address mean something?
          We have to mask it off otherwise the "Juno First" logo on the title screen is wrong.
*/

WRITE8_HANDLER( junofrst_blitter_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;
	state->blitterdata[offset] = data;

	/* blitter is triggered by $8073 */
	if (offset == 3)
	{
		int i;
		UINT8 *gfx_rom = memory_region(space->machine, "gfx1");

		offs_t src = ((state->blitterdata[2] << 8) | state->blitterdata[3]) & 0xfffc;
		offs_t dest = (state->blitterdata[0] << 8) | state->blitterdata[1];

		int copy = state->blitterdata[3] & 0x01;

		/* 16x16 graphics */
		for (i = 0; i < 16; i++)
		{
			int j;

			for (j = 0; j < 16; j++)
			{
				UINT8 data;

				if (src & 1)
					data = gfx_rom[src >> 1] & 0x0f;
				else
					data = gfx_rom[src >> 1] >> 4;

				src += 1;

				/* if there is a source pixel either copy the pixel or clear the pixel depending on the copy flag */

				if (data)
				{
					if (copy == 0)
						data = 0;

					if (dest & 1)
						state->videoram[dest >> 1] = (state->videoram[dest >> 1] & 0x0f) | (data << 4);
					else
						state->videoram[dest >> 1] = (state->videoram[dest >> 1] & 0xf0) | data;
				}

				dest += 1;
			}

			dest += 240;
		}
	}
}
