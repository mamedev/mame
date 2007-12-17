#include "driver.h"


static int ra_charbank[2] = { 0,0 };
static int ra_bkgpage = 0;
static int ra_bkgflip = 0;
static int ra_chrbank = 0;
static int ra_bkgpen = 0;
static int ra_bkgcol = 0;
static int ra_flipy = 0;
static int ra_flipx = 0;
static int ra_spritebank =0 ;

#define	RA_FGCHAR_BASE 	0
#define RA_BGCHAR_BASE 	4
#define RA_SP_BASE	5

WRITE8_HANDLER( rollrace_charbank_w)
{

	ra_charbank[offset&1] = data;
	ra_chrbank = ra_charbank[0] | (ra_charbank[1] << 1) ;
}


WRITE8_HANDLER( rollrace_bkgpen_w)
{
	ra_bkgpen = data;
}

WRITE8_HANDLER(rollrace_spritebank_w)
{
	ra_spritebank = data;
}

WRITE8_HANDLER(rollrace_backgroundpage_w)
{

	ra_bkgpage = data & 0x1f;
	ra_bkgflip = ( data & 0x80 ) >> 7;

	/* 0x80 flip vertical */
}

WRITE8_HANDLER( rollrace_backgroundcolor_w )
{
	ra_bkgcol = data;
}

WRITE8_HANDLER( rollrace_flipy_w )
{
	ra_flipy = data & 0x01;
}

WRITE8_HANDLER( rollrace_flipx_w )
{
	ra_flipx = data & 0x01;
}

VIDEO_UPDATE( rollrace )
{

	int offs;
	int sx, sy;
	int scroll;
	int col;

	/* fill in background colour*/
	fillbitmap(bitmap,machine->pens[ra_bkgpen],&machine->screen[0].visarea);

	/* draw road */
	for (offs = videoram_size - 1;offs >= 0;offs--)
		{
			if(!(ra_bkgflip))
				{
				sy = ( 31 - offs / 32 ) ;
				}
			else
				sy = ( offs / 32 ) ;

			sx = ( offs%32 ) ;

			if(ra_flipx)
				sx = 31-sx ;

			if(ra_flipy)
				sy = 31-sy ;

			drawgfx(bitmap,
				machine->gfx[RA_BGCHAR_BASE],
				memory_region(REGION_USER1)[offs + ( ra_bkgpage * 1024 )] \
				+ ((( memory_region(REGION_USER1)[offs + 0x4000 + ( ra_bkgpage * 1024 )] & 0xc0 ) >> 6 ) * 256 ) ,
				ra_bkgcol,
				ra_flipx,(ra_bkgflip^ra_flipy),
				sx*8,sy*8,
				&machine->screen[0].visarea,TRANSPARENCY_PEN,0);


		}




	/* sprites */
	for ( offs = 0x80-4 ; offs >=0x0 ; offs -= 4)
	{
		int s_flipy = 0;
		int bank = 0;

		sy=spriteram[offs] - 16;
		sx=spriteram[offs+3] - 16;

		if(sx && sy)
		{

		if(ra_flipx)
			sx = 224 - sx;
		if(ra_flipy)
			sy = 224 - sy;

		if(spriteram[offs+1] & 0x80)
			s_flipy = 1;

		bank = (( spriteram[offs+1] & 0x40 ) >> 6 ) ;

		if(bank)
			bank += ra_spritebank;

		drawgfx(bitmap, machine->gfx[ RA_SP_BASE + bank ],
			spriteram[offs+1] & 0x3f ,
			spriteram[offs+2] & 0x1f,
			ra_flipx,!(s_flipy^ra_flipy),
			sx,sy,
			&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
		}
	}




	/* draw foreground characters */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{

		sx =  offs % 32;
		sy =  offs / 32;

		scroll = ( 8 * sy + colorram[2 * sx] ) % 256;
		col = colorram[ sx * 2 + 1 ]&0x1f;

		if (!ra_flipy)
		{
		   scroll = (248 - scroll) % 256;
		}

		if (ra_flipx) sx = 31 - sx;

		drawgfx(bitmap,machine->gfx[RA_FGCHAR_BASE + ra_chrbank]  ,
			videoram[ offs ]  ,
			col,
			ra_flipx,ra_flipy,
			8*sx,scroll,
			&machine->screen[0].visarea,TRANSPARENCY_PEN,0);

	}



	return 0;
}
