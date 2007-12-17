/***************************************************************************

    Atari Cops'n Robbers hardware

***************************************************************************/

#include "driver.h"
#include "copsnrob.h"


UINT8 *copsnrob_bulletsram;
UINT8 *copsnrob_carimage;
UINT8 *copsnrob_cary;
UINT8 *copsnrob_trucky;
UINT8 *copsnrob_truckram;


VIDEO_UPDATE( copsnrob )
{
	int offs, x, y;

    /* redrawing the entire display is faster in this case */

    for (offs = videoram_size;offs >= 0;offs--)
    {
		int sx,sy;

		sx = 31 - (offs % 32);
		sy = offs / 32;

		drawgfx(bitmap,machine->gfx[0],
				videoram[offs] & 0x3f,0,
				0,0,
				8*sx,8*sy,
				cliprect,TRANSPARENCY_NONE,0);
    }


    /* Draw the cars. Positioning was based on a screen shot */
    if (copsnrob_cary[0])
    {
        drawgfx(bitmap,machine->gfx[1],
                copsnrob_carimage[0],0,
                1,0,
                0xe4,256-copsnrob_cary[0],
                cliprect,TRANSPARENCY_PEN,0);
    }

    if (copsnrob_cary[1])
    {
        drawgfx(bitmap,machine->gfx[1],
                copsnrob_carimage[1],0,
                1,0,
                0xc4,256-copsnrob_cary[1],
                cliprect,TRANSPARENCY_PEN,0);
    }

    if (copsnrob_cary[2])
    {
        drawgfx(bitmap,machine->gfx[1],
                copsnrob_carimage[2],0,
                0,0,
                0x24,256-copsnrob_cary[2],
                cliprect,TRANSPARENCY_PEN,0);
    }

    if (copsnrob_cary[3])
    {
        drawgfx(bitmap,machine->gfx[1],
                copsnrob_carimage[3],0,
                0,0,
                0x04,256-copsnrob_cary[3],
                cliprect,TRANSPARENCY_PEN,0);
    }


    /* Draw the beer truck. Positioning was based on a screen shot.
        We scan the truck's window RAM for a location whose bit is set and
        which corresponds either to the truck's front end or the truck's back
        end (based on the value of the truck image line sync register). We
        then draw a truck image in the proper place and continue scanning.
        This is not a perfect emulation of the game hardware, but it should
        suffice for the way the game software uses the hardware.  It does take
        care of the problem of displaying multiple beer trucks and of scrolling
        truck images smoothly off the top of the screen. */

     for (y = 0; y < 256; y++)
     {
		/* y is going up the screen, but the truck window RAM locations
        go down the screen. */

		if (copsnrob_truckram[255-y])
		{
			/* The hardware only uses the low 5 bits of the truck image line
            sync register. */
			if ((copsnrob_trucky[0] & 0x1f) == ((y+31) & 0x1f))
			{
				/* We've hit a truck's back end, so draw the truck.  The front
                   end may be off the top of the screen, but we don't care. */
				drawgfx(bitmap,machine->gfx[2],
						0,0,
						0,0,
						0x80,256-(y+31),
						cliprect,TRANSPARENCY_PEN,0);
				/* Skip past this truck's front end so we don't draw this
                truck twice. */
				y += 31;
			}
			else if ((copsnrob_trucky[0] & 0x1f) == (y & 0x1f))
			{
				/* We missed a truck's back end (it was off the bottom of the
                   screen) but have hit its front end, so draw the truck. */
				drawgfx(bitmap,machine->gfx[2],
						0,0,
						0,0,
						0x80,256-y,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
    }


    /* Draw the bullets.
       They are flickered on/off every frame by the software, so don't
       play it with frameskip 1 or 3, as they could become invisible */

    for (x = 0; x < 256; x++)
    {
	    int bullet, mask1, mask2, val;


        val = copsnrob_bulletsram[x];

        // Check for the most common case
        if (!(val & 0x0f)) continue;

        mask1 = 0x01;
        mask2 = 0x10;

        // Check each bullet
        for (bullet = 0; bullet < 4; bullet++)
        {
            if (val & mask1)
            {
                for (y = 0; y <= machine->screen[0].visarea.max_y; y++)
                {
                    if (copsnrob_bulletsram[y] & mask2)
                    {
						*BITMAP_ADDR16(bitmap, y, 256-x) = machine->pens[1];
                    }
                }
            }

            mask1 <<= 1;
            mask2 <<= 1;
        }
    }
	return 0;
}
