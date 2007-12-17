#include "driver.h"
#include "taxidrvr.h"


UINT8 *taxidrvr_vram0,*taxidrvr_vram1,*taxidrvr_vram2,*taxidrvr_vram3;
UINT8 *taxidrvr_vram4,*taxidrvr_vram5,*taxidrvr_vram6,*taxidrvr_vram7;
UINT8 *taxidrvr_scroll;
int taxidrvr_bghide;
static int spritectrl[9];


WRITE8_HANDLER( taxidrvr_spritectrl_w )
{
	spritectrl[offset] = data;
}



VIDEO_UPDATE( taxidrvr )
{
	int offs;
	int sx,sy;


	if (taxidrvr_bghide)
	{
		fillbitmap(bitmap,machine->pens[0],cliprect);


		/* kludge to fix scroll after death */
		taxidrvr_scroll[0] = taxidrvr_scroll[1] = taxidrvr_scroll[2] = taxidrvr_scroll[3] = 0;
		spritectrl[2] = spritectrl[5] = spritectrl[8] = 0;
	}
	else
	{
		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx(bitmap,machine->gfx[3],
					taxidrvr_vram3[offs],
					0,
					0,0,
					(sx*8-taxidrvr_scroll[0])&0xff,(sy*8-taxidrvr_scroll[1])&0xff,
					cliprect,TRANSPARENCY_NONE,0);
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx(bitmap,machine->gfx[2],
					taxidrvr_vram2[offs]+256*taxidrvr_vram2[offs+0x400],
					0,
					0,0,
					(sx*8-taxidrvr_scroll[2])&0xff,(sy*8-taxidrvr_scroll[3])&0xff,
					cliprect,TRANSPARENCY_PEN,0);
		}

		if (spritectrl[2] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-spritectrl[0]-256*(spritectrl[2]&1))&0x1ff;
				sy = ((offs/2) / 64-spritectrl[1]-128*(spritectrl[2]&2))&0x1ff;

				color = (taxidrvr_vram5[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		if (spritectrl[5] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-spritectrl[3]-256*(spritectrl[5]&1))&0x1ff;
				sy = ((offs/2) / 64-spritectrl[4]-128*(spritectrl[5]&2))&0x1ff;

				color = (taxidrvr_vram6[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		if (spritectrl[8] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-spritectrl[6]-256*(spritectrl[8]&1))&0x1ff;
				sy = ((offs/2) / 64-spritectrl[7]-128*(spritectrl[8]&2))&0x1ff;

				color = (taxidrvr_vram7[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx(bitmap,machine->gfx[1],
					taxidrvr_vram1[offs],
					0,
					0,0,
					sx*8,sy*8,
					cliprect,TRANSPARENCY_PEN,0);
		}

		for (offs = 0;offs < 0x2000;offs++)
		{
			int color;

			sx = (offs/2) % 64;
			sy = (offs/2) / 64;

			color = (taxidrvr_vram4[offs/4]>>(2*(offs&3)))&0x03;
			if (color)
			{
				*BITMAP_ADDR16(bitmap, sy, sx) = 2 * color;
			}
		}
	}

	for (offs = 0;offs < 0x400;offs++)
	{
		sx = offs % 32;
		sy = offs / 32;

		drawgfx(bitmap,machine->gfx[0],
				taxidrvr_vram0[offs],
				0,
				0,0,
				sx*8,sy*8,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}
