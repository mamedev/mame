/***************************************************************************

                      -= Electronic Devices / International Games =-

                    driver by   Luca Elia (l.elia@tin.it)

    This game has sprites only:

    tiles are 16 x 16 x 6. There are 0x400 sprites, each one is allotted
    8 bytes of memory in spriteram (but only 5 are used) :

    Offset:     Bits:           Value:

        0                       X (low bits)

        1       7--- ----       X (high bit)
                -6-- ----       Y (high bit)
                --5- ----       Flip X
                ---4 ----       Flip Y
                ---- 32--
                ---- --10       Color

        2                       Code (high bits)

        3                       Code (low bits)

        4                       Y (low bits)

    Then 2 tables follow, 0x400 bytes each:

    - the first table contains 1 byte per sprite: an index in the second table
    - the second table is either an x,y offset or an index in spriteram_2:

        0                       X offset (low bits)

        1                       Y offset (low bits)

        2       7--- ----       If 1, the following bits are an index in spriteram_2 for the real X&Y & Code offsets
                -654 321-
                ---- ---0       X offset (high bit)

        3       7654 321-
                ---- ---0       Y offset (high bit)


    Spriteram_2 contains 0x4000 X&Y & Code offsets:

        0                       Y offset (low bits)

        1       7654 321-       Code offset
                ---- ---0       Y offset (high bit)

        2                       X offset (low bits)

        3                       X offset (high bit)

***************************************************************************/

#include "driver.h"

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	UINT8	*indx_ram	=	spriteram + 0x2000,	// this ram contains indexes into offs_ram
			*offs_ram	=	spriteram + 0x2400,	// this ram contains x,y offsets or indexes into spriteram_2
			*ram		=	spriteram,			// current sprite pointer in spriteram
			*ram2		=	indx_ram;			// current sprite pointer in indx_ram

	for ( ; ram < indx_ram; ram += 8,ram2++)
	{
		int attr,code,color, x,y,xoffs,yoffs,flipx,flipy, idx;

		attr	=	ram[1];

		x		=	ram[0];
		code	=	ram[3] + (ram[2] << 8);
		y		=	ram[4];

		color	=	(attr & 0x03);
		flipy	=	(attr & 0x10);
		flipx	=	(attr & 0x20);

		y		+=	(attr & 0x40) << 2;
		x		+=	(attr & 0x80) << 1;

		// Index in the table of offsets

		idx		=	ram2[0] * 4;

		// Fetch the offsets

		if (offs_ram[idx + 2] & 0x80)
		{
			// x,y & code offset is in spriteram_2, this is its index

			idx		=	(((offs_ram[idx + 2] << 8) + offs_ram[idx + 3]) & 0x3fff) * 4;

			yoffs	=	spriteram_2[idx + 0] + (spriteram_2[idx + 1] << 8);
			xoffs	=	spriteram_2[idx + 2] + (spriteram_2[idx + 3] << 8);

			code	+=	yoffs >> 9;
		}
		else
		{
			// this is an x,y offset

			yoffs	=	((offs_ram[idx + 3] & 0x01) << 8) + offs_ram[idx + 1];
			xoffs	=	((offs_ram[idx + 2] & 0x01) << 8) + offs_ram[idx + 0];
		}

		yoffs		=	(yoffs & 0xff) - (yoffs & 0x100);
		xoffs		=	(xoffs & 0x1ff);

		if (xoffs >= 0x180)		xoffs -= 0x200;

		y		+=	yoffs;
		x		+=	xoffs;

		y		=	(y & 0xff) - (y & 0x100);
		x		=	(x & 0x1ff);

		if (x >= 0x180)		x -= 0x200;

		drawgfx(bitmap,machine->gfx[0], code,color, flipx,flipy, x,y, cliprect,TRANSPARENCY_PEN,0);
	}
}

VIDEO_UPDATE( fantland )
{
	fillbitmap(bitmap,machine->pens[0],cliprect);
	draw_sprites(machine,bitmap,cliprect);

	return 0;
}
