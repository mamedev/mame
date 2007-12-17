/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *redalert_backram;
UINT8 *redalert_spriteram1;
UINT8 *redalert_spriteram2;
UINT8 *redalert_spriteram3;
UINT8 *redalert_characterram;
UINT8 *redalert_characterram2;

static UINT8 redalert_dirtyback[0x400];
static UINT8 redalert_dirtycharacter[0x100];
static UINT8 redalert_dirtycharacter2[0x100];
static UINT8 redalert_backcolor[0x400];


/* There might be a color PROM that dictates this? */
/* These guesses are based on comparing the color bars on the test
   screen with the picture in the manual */
static UINT8 color_lookup[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,3,
	1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

	1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};

static int backcolor, flip=0;

WRITE8_HANDLER( redalert_c040_w )
{
	/* Only seems to load D0-D3 into a flip-flop. */
	/* D0/D1 seem to head off to unconnected circuits */
	/* D2 connects to a "NL" line, and NOTted to a "NH" line */
	/* D3 connects to a "YI" line */

	/*
        D0 == 1             -> 1 player
        D1 == 1 and D0 == 1 -> 2 players
    */
	flip = !(data & 0x04);
}

WRITE8_HANDLER( redalert_backcolor_w )
{
	/* Only seems to load D0-D2 into a flip-flop. */
	/* Outputs feed into RAM which seems to feed to RGB lines. */
	backcolor = data & 0x07;
}

WRITE8_HANDLER( demoneye_c040_w )
{
	/*
        D0 == 1             -> 1 player
        D1 == 1 and D0 == 1 -> 2 players
    */
	flip = data & 0x04;
}

/***************************************************************************
redalert_backram_w
***************************************************************************/

WRITE8_HANDLER( redalert_backram_w )
{
	int charnum;

	charnum = offset / 8 % 0x400;

	if ((redalert_backram[offset] != data) ||
		(redalert_backcolor[charnum] != backcolor))
	{
		redalert_dirtyback[charnum] = 1;
		dirtybuffer[charnum] = 1;
		redalert_backcolor[charnum] = backcolor;

		redalert_backram[offset] = data;
	}
}

/***************************************************************************
redalert_spriteram1_w
***************************************************************************/

WRITE8_HANDLER( redalert_spriteram1_w )
{
	if (redalert_spriteram1[offset] != data)
	{
		redalert_dirtycharacter[((offset / 8) % 0x80) + 0x80] = 1;

		redalert_spriteram1[offset] = data;
	}
}

/***************************************************************************
redalert_spriteram2_w
***************************************************************************/

WRITE8_HANDLER( redalert_spriteram2_w )
{
	if (redalert_spriteram2[offset] != data)
	{

		redalert_dirtycharacter[((offset / 8) % 0x80) + 0x80] = 1;

		redalert_spriteram2[offset] = data;
	}
}

/***************************************************************************
redalert_characterram_w
***************************************************************************/

WRITE8_HANDLER( redalert_characterram_w )
{
	if (redalert_characterram[offset] != data)
	{
		redalert_dirtycharacter[((offset / 8) % 0x80)] = 1;

		redalert_characterram[offset] = data;
	}
}

WRITE8_HANDLER( redalert_characterram2_w )
{
	if (redalert_characterram2[offset] != data)
	{
		redalert_dirtycharacter[((offset / 8) % 0x80)] = 1;

		redalert_characterram2[offset] = data;
	}
}

WRITE8_HANDLER( redalert_spriteram3_w )
{
	if (redalert_spriteram3[offset] != data)
	{
		redalert_dirtycharacter2[((offset / 8) % 0x80) + 0x80] = 1;

		redalert_spriteram3[offset] = data;
	}

}


VIDEO_UPDATE( redalert )
{
	int offs,i;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int charcode;
		int stat_transparent;


		charcode = videoram[offs];

		if (dirtybuffer[offs] || redalert_dirtycharacter[charcode] || redalert_dirtycharacter2[charcode])
		{
			int sx,sy,color;


			/* decode modified background */
			if (redalert_dirtyback[offs] == 1)
			{
				decodechar(machine->gfx[0],offs,redalert_backram,
							machine->drv->gfxdecodeinfo[0].gfxlayout);
				redalert_dirtyback[offs] = 2;
			}

			/* decode modified characters */
			if (redalert_dirtycharacter[charcode] == 1)
			{
				if (charcode < 0x80)
					decodechar(machine->gfx[1],charcode,redalert_characterram,
								machine->drv->gfxdecodeinfo[1].gfxlayout);
				else
					decodechar(machine->gfx[2],charcode-0x80,redalert_spriteram1,
								machine->drv->gfxdecodeinfo[2].gfxlayout);
				redalert_dirtycharacter[charcode] = 2;
			}

			if (redalert_dirtycharacter2[charcode] == 1)
			{
				decodechar(machine->gfx[3],charcode-0x80,redalert_spriteram3,
							machine->drv->gfxdecodeinfo[3].gfxlayout);
				redalert_dirtycharacter2[charcode] = 2;
			}

			dirtybuffer[offs] = 0;

			sx = 31 - offs / 32;
			sy = offs % 32;

			stat_transparent = TRANSPARENCY_NONE;

			/* First layer of color */
			if (charcode >= 0xC0)
			{
				stat_transparent = TRANSPARENCY_COLOR;

				color = color_lookup[charcode];

				drawgfx(tmpbitmap,machine->gfx[2],
						charcode-0x80,
						color,
						0,0,
						8*sx,8*sy,
						&machine->screen[0].visarea,TRANSPARENCY_NONE,0);

				if( redalert_dirtycharacter2[charcode] != 0 )
					drawgfx(tmpbitmap,machine->gfx[3],
							charcode-0x80,
							color,
							0,0,
							8*sx,8*sy,
							&machine->screen[0].visarea,TRANSPARENCY_COLOR,0);

			}

			/* Second layer - background */
			color = redalert_backcolor[offs];
			drawgfx(tmpbitmap,machine->gfx[0],
					offs,
					color,
					0,0,
					8*sx,8*sy,
					&machine->screen[0].visarea,stat_transparent,0);

			/* Third layer - alphanumerics & sprites */
			if (charcode < 0x80)
			{
				color = color_lookup[charcode];
				drawgfx(tmpbitmap,machine->gfx[1],
						charcode,
						color,
						0,0,
						8*sx,8*sy,
						&machine->screen[0].visarea,TRANSPARENCY_COLOR,0);
			}
			else if (charcode < 0xC0)
			{
				color = color_lookup[charcode];
				drawgfx(tmpbitmap,machine->gfx[2],
						charcode-0x80,
						color,
						0,0,
						8*sx,8*sy,
						&machine->screen[0].visarea,TRANSPARENCY_COLOR,0);

				if( redalert_dirtycharacter2[charcode] != 0 )
					drawgfx(tmpbitmap,machine->gfx[3],
							charcode-0x80,
							color,
							0,0,
							8*sx,8*sy,
							&machine->screen[0].visarea,TRANSPARENCY_COLOR,0);

			}

		}
	}

	for (i = 0;i < 256;i++)
	{
		if (redalert_dirtycharacter[i] == 2)
			redalert_dirtycharacter[i] = 0;

		if (redalert_dirtycharacter2[i] == 2)
			redalert_dirtycharacter2[i] = 0;
	}

	for (i = 0;i < 0x400;i++)
	{
		if (redalert_dirtyback[i] == 2)
			redalert_dirtyback[i] = 0;
	}

	/* copy the character mapped graphics */
	copybitmap(bitmap,tmpbitmap,flip,flip,0,0,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);

	return 0;
}
