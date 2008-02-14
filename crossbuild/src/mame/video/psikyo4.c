/*

Psikyo PS6807 (PS4):
See src/drivers/psikyo4.c for more info

Each sprite has a flag denoting the screen to which it should be drawn.

*/

/*
Vid Regs:

0x3003fe4 -- ??xx???? vblank? 86??0000 always?
0x3003fe8 -- c0c0???? flipscreen for screen 1 and 2 resp.
             ????8080 Screen size select
0x3003fec -- a0000xxx always? is in two working games. 0x00000fff is bank select for gfx test
0x3003ff0 -- 000000ff brightness for screen 1, ffffff00 are probably seperate rgb brightness (not used)
0x3003ff4 -- ffffff00 screen 1 clear colour
0x3003ff8 -- 000000ff brightness for screen 2, ffffff00 are probably seperate rgb brightness (not used)
0x3003ffc -- ffffff00 screen 2 clear colour

HotDebut: 86010000 00009998 80000000 Small Screen
LodeRnDF: 86010000 00009998 a0000000 Small Screen

HotGmck:  86010000 1f201918 a0000000 Large Screen
HgKairak: 86010000 1f201918 a0000000 Large Screen
*/

#include "driver.h"

/* defined in drivers/psikyo4.c */
extern UINT32 *psikyo4_vidregs;

/* --- SPRITES --- */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT32 scr )
{
	/*- Sprite Format 0x0000 - 0x2bff -**

    0 hhhh --yy yyyy yyyy | wwww --xx xxxx xxxx  1  Ffpp pppp ---- -nnn | nnnn nnnn nnnn nnnn

    y = ypos
    x = xpos

    h = height
    w = width

    f = flip (x)
    F = flip (y) Unused?

    n = tile number

    p = palette

    **- End Sprite Format -*/

	const gfx_element *gfx = machine->gfx[0];
	UINT32 *source = spriteram32;
	UINT16 *list = (UINT16 *)spriteram32 + 0x2c00/2 + 0x04/2; /* 0x2c00/0x2c02 what are these for, pointers? one for each screen */
	UINT16 listlen=(0xc00/2 - 0x04/2), listcntr=0;
	int flipscreen1, flipscreen2;

	flipscreen1 = (((psikyo4_vidregs[1]>>30)&2) == 2) ? 1 : 0;
	flipscreen2 = (((psikyo4_vidregs[1]>>22)&2) == 2) ? 1 : 0;

	while( listcntr < listlen )
	{
		UINT16 listdat, sprnum, thisscreen;

		listdat = list[BYTE_XOR_BE(listcntr)];
		sprnum = (listdat & 0x03ff) * 2;

		thisscreen = 0;
		if ((listdat & 0x2000) == scr) thisscreen = 1;

		/* start drawing */
		if (!(listdat & 0x8000) && thisscreen) /* draw only selected screen */
		{
			int loopnum=0, i, j;
			UINT32 xpos, ypos, tnum, wide, high, colr, flipx, flipy;
			int xstart, ystart, xend, yend, xinc, yinc;

			ypos = (source[sprnum+0] & 0x03ff0000) >> 16;
			xpos = (source[sprnum+0] & 0x000003ff) >> 00;

			high = ((source[sprnum+0] & 0xf0000000) >> (12+16)) + 1;
			wide = ((source[sprnum+0] & 0x0000f000) >> 12) + 1;

			tnum = (source[sprnum+1] & 0x0007ffff) >> 00;

			colr = (source[sprnum+1] & 0x3f000000) >> 24;
   			if(scr) colr += 0x40; /* Use second copy of palette which is dimmed appropriately */

			flipx = (source[sprnum+1] & 0x40000000);
			flipy = (source[sprnum+1] & 0x80000000); /* Guess */

			if(ypos & 0x200) ypos -= 0x400;
			if(xpos & 0x200) xpos -= 0x400;

			if((!scr && flipscreen1) || (scr && flipscreen2))
			{
				ypos = machine->screen[0].visarea.max_y+1 - ypos - high*16; /* Screen Height depends on game */
				xpos = 40*8 - xpos - wide*16;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (flipx)	{ xstart = wide-1;  xend = -1;    xinc = -1; }
			else		{ xstart = 0;       xend = wide;  xinc = +1; }

			if (flipy)	{ ystart = high-1;  yend = -1;     yinc = -1; }
			else		{ ystart = 0;       yend = high;   yinc = +1; }

			for (j = ystart; j != yend; j += yinc) {
				for (i = xstart; i != xend; i += xinc) {
					drawgfx(bitmap,gfx,tnum+loopnum,colr,flipx,flipy,xpos+16*i,ypos+16*j,cliprect,TRANSPARENCY_PEN,0);
					loopnum++;
				}
			}
		}
		/* end drawing */
		listcntr++;
		if (listdat & 0x4000) break;
	}
}

VIDEO_UPDATE( psikyo4 )
{
	if (screen==0)
	{
		fillbitmap(bitmap, machine->pens[0x1000], cliprect);
		draw_sprites(machine, bitmap, cliprect, 0x0000);
	}
	else if (screen==1)
	{
		fillbitmap(bitmap, machine->pens[0x1001], cliprect);
		draw_sprites(machine, bitmap, cliprect, 0x2000);
	}
	return 0;
}

VIDEO_START( psikyo4 )
{
	machine->gfx[0]->color_granularity=32; /* 256 colour sprites with palette selectable on 32 colour boundaries */
}
