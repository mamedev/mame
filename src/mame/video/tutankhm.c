/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/


#include "driver.h"
#include "tutankhm.h"


#define NUM_PENS	(0x10)


UINT8 *tutankhm_scroll;

static UINT8 junofrst_blitterdata[4];
static UINT8 tutankhm_flip_screen_x;
static UINT8 tutankhm_flip_screen_y;



WRITE8_HANDLER( tutankhm_flip_screen_x_w )
{
	tutankhm_flip_screen_x = data & 0x01;
}


WRITE8_HANDLER( tutankhm_flip_screen_y_w )
{
	tutankhm_flip_screen_y = data & 0x01;
}


static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 data = paletteram[i];

		pens[i] = MAKE_RGB(pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


VIDEO_START( tutankhm )
{
	state_save_register_global_array(junofrst_blitterdata);
	state_save_register_global(tutankhm_flip_screen_x);
	state_save_register_global(tutankhm_flip_screen_y);
}


VIDEO_UPDATE( tutankhm )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < videoram_size; offs++)
	{
		int i;

		UINT8 data = videoram[offs];

		UINT8 y = offs >> 7;
		UINT8 x = offs << 1;

		for (i = 0; i < 2; i++)
		{
			UINT8 sy = y;
			UINT8 sx = x;

			pen_t pen = pens[data & 0x0f];

			/* adjust for scrolling */
			if (x < 192)
			{
				sy = sy - *tutankhm_scroll;
			}

			if (tutankhm_flip_screen_y)	sy = 255 - sy;
			if (tutankhm_flip_screen_x)	sx = 255 - sx;

			*BITMAP_ADDR32(bitmap, sy, sx) = pen;

			x = x + 1;
			data = data >> 4;
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
	junofrst_blitterdata[offset] = data;

	/* blitter is triggered by $8073 */
	if (offset == 3)
	{
		int i;
		UINT8 *gfx_rom = memory_region(REGION_GFX1);

		offs_t src = ((junofrst_blitterdata[2] << 8) | junofrst_blitterdata[3]) & 0xfffc;
		offs_t dest = (junofrst_blitterdata[0] << 8) | junofrst_blitterdata[1];

		int copy = junofrst_blitterdata[3] & 0x01;

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

				src = src + 1;

				/* if there is a source pixel either copy the pixel or clear the pixel depending on the copy flag */

				if (data)
				{
					if (copy==0)
						data = 0;

					if (dest & 1)
						videoram[dest >> 1] = (videoram[dest >> 1] & 0x0f) | (data << 4);
					else
						videoram[dest >> 1] = (videoram[dest >> 1] & 0xf0) | data;
				}

				dest = dest + 1;
			}

			dest = dest + 240;
		}
	}
}
