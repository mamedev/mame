/*************************************************************************
 Universal Cheeky Mouse Driver
 (c)Lee Taylor May 1998, All rights reserved.

 For use only in offical Mame releases.
 Not to be distrabuted as part of any commerical work.
***************************************************************************
Functions to emulate the video hardware of the machine.
***************************************************************************/

#include "driver.h"
#include "sound/dac.h"


static int man_scroll = -1;
static UINT8 sprites[0x20];
static int char_palette = 0;


PALETTE_INIT( cheekyms )
{
	int i,j,bit,r,g,b;

	for (i = 0; i < 6; i++)
	{
		for (j = 0;j < 0x20;j++)
		{
			/* red component */
			bit = (color_prom[0x20*(i/2)+j] >> ((4*(i&1))+0)) & 0x01;
			r = 0xff * bit;
			/* green component */
			bit = (color_prom[0x20*(i/2)+j] >> ((4*(i&1))+1)) & 0x01;
			g = 0xff * bit;
			/* blue component */
			bit = (color_prom[0x20*(i/2)+j] >> ((4*(i&1))+2)) & 0x01;
			b = 0xff * bit;

			palette_set_color(machine,(i*0x20)+j,MAKE_RGB(r,g,b));
		}
	}
}


WRITE8_HANDLER( cheekyms_sprite_w )
{
	sprites[offset] = data;
}


WRITE8_HANDLER( cheekyms_port_40_w )
{
	static int last_dac = -1;

	/* The lower bits probably trigger sound samples */

	if (last_dac != (data & 0x80))
	{
		last_dac = data & 0x80;

		DAC_data_w(0, last_dac ? 0x80 : 0);
	}
}


WRITE8_HANDLER( cheekyms_port_80_w )
{
	/* Bits 0-1 Sound enables, not sure which bit is which */

	/* Bit 2 is interrupt enable */
	interrupt_enable_w(offset, data & 0x04);

	/* Bit 3-5 Man scroll amount */
	man_scroll = (data >> 3) & 0x07;

	/* Bit 6 is palette select (Selects either 0 = PROM M9, 1 = PROM M8) */
	char_palette = (data >> 2) & 0x10;

	/* Bit 7 is screen flip */
	flip_screen_set(data & 0x80);
}



VIDEO_UPDATE( cheekyms )
{
	int offs;


	fillbitmap(bitmap,machine->pens[0],cliprect);

	/* Draw the sprites first, because they're supposed to appear below
       the characters */
	for (offs = 0; offs < sizeof(sprites)/sizeof(sprites[0]); offs += 4)
	{
		int v1, sx, sy, col, code;

		v1  = sprites[offs + 0];
		sy  = sprites[offs + 1];
		sx  = 256 - sprites[offs + 2];
		col = (~sprites[offs + 3] & 0x07);

		if (!(sprites[offs + 3] & 0x08)) continue;

		code = (~v1 << 1) & 0x1f;

		if (v1 & 0x80)
		{
			if (!flip_screen)
			{
				code++;
			}

			drawgfx(bitmap,machine->gfx[1],
					code,
					col,
					0,0,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
		else
		{
			drawgfx(bitmap,machine->gfx[1],
					code + 0x20,
					col,
					0,0,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);

			drawgfx(bitmap,machine->gfx[1],
					code + 0x21,
					col,
					0,0,
					sx + 8*(v1 & 2),sy + 8*(~v1 & 2),
					cliprect,TRANSPARENCY_PEN,0);
		}
	}


	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,man_area,color;

		sx = offs % 32;
		sy = offs / 32;


		man_area = ((sy >=  6) && (sy <= 26) && (sx >=  8) && (sx <= 12));

		if (sx >= 30)
		{
			if (sy < 12)
				color = 0x15;
			else if (sy < 20)
				color = 0x16;
			else
				color = 0x14;
		}
		else
		{
			color = ((sx >> 1) & 0x0f) + char_palette;
			if (sy == 4 || sy == 27)
				color = 0xc + char_palette;
		}

		if (flip_screen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
		}

		drawgfx(bitmap,machine->gfx[0],
				videoram[offs],
				color,
				flip_screen,flip_screen,
				8*sx, 8*sy - (man_area ? man_scroll : 0),
				cliprect,TRANSPARENCY_PEN,0);
	}

	return 0;
}
