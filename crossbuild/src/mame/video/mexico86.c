#include "driver.h"

UINT8 *mexico86_videoram,*mexico86_objectram;
size_t mexico86_objectram_size;
static int charbank;



WRITE8_HANDLER( mexico86_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	if ((data & 7) > 5)
		popmessage( "Switching to invalid bank!" );

	memory_set_bankptr(1, &RAM[0x10000 + 0x4000 * (data & 0x07)]);

	charbank = (data & 0x20) >> 5;
}



VIDEO_UPDATE( mexico86 )
{
	int offs;
	int sx,sy,xc,yc;
	int gfx_num,gfx_attr,gfx_offs;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored inthe area dd00-dd3f */

	/* This clears & redraws the entire screen each pass */
	fillbitmap(bitmap,machine->pens[255],&machine->screen[0].visarea);

	sx = 0;
/* the score display seems to be outside of the main objectram. */
	for (offs = 0;offs < mexico86_objectram_size+0x200;offs += 4)
	{
		int height;

if (offs >= mexico86_objectram_size && offs < mexico86_objectram_size+0x180) continue;
if (offs >= mexico86_objectram_size+0x1c0) continue;

		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&mexico86_objectram[offs]) == 0)
			continue;

		gfx_num = mexico86_objectram[offs + 1];
		gfx_attr = mexico86_objectram[offs + 3];

		if ((gfx_num & 0x80) == 0)  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)   /* next column */
			sx += 16;
		else
		{
			sx = mexico86_objectram[offs + 2];
//          if (gfx_attr & 0x40) sx -= 256;
		}
		sy = 256 - height*8 - (mexico86_objectram[offs + 0]);

		for (xc = 0;xc < 2;xc++)
		{
			for (yc = 0;yc < height;yc++)
			{
				int goffs,code,color,flipx,flipy,x,y;

				goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				code = mexico86_videoram[goffs] + ((mexico86_videoram[goffs + 1] & 0x07) << 8)
						+ ((mexico86_videoram[goffs + 1] & 0x80) << 4) + (charbank << 12);
				color = ((mexico86_videoram[goffs + 1] & 0x38) >> 3) + ((gfx_attr & 0x02) << 2);
				flipx = mexico86_videoram[goffs + 1] & 0x40;
				flipy = 0;
//              x = sx + xc * 8;
				x = (sx + xc * 8) & 0xff;
				y = (sy + yc * 8) & 0xff;

				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flipx,flipy,
						x,y,
						&machine->screen[0].visarea,TRANSPARENCY_PEN,15);
			}
		}
	}
	return 0;
}
//AT
#if 0 // old code
VIDEO_UPDATE( kikikai )
{
	int offs;
	int sx,sy,xc,yc;
	int gfx_num,gfx_attr,gfx_offs;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored inthe area dd00-dd3f */

	/* This clears & redraws the entire screen each pass */
	fillbitmap(bitmap,machine->pens[255],&machine->screen[0].visarea);

	sx = 0;
/* the score display seems to be outside of the main objectram. */
	for (offs = 0;offs < mexico86_objectram_size+0x200;offs += 4)
	{
		int height;

if (offs >= mexico86_objectram_size && offs < mexico86_objectram_size+0x180) continue;
if (offs >= mexico86_objectram_size+0x1c0) continue;

		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&mexico86_objectram[offs]) == 0)
			continue;

		gfx_num = mexico86_objectram[offs + 1];
		gfx_attr = mexico86_objectram[offs + 3];

		if ((gfx_num & 0x80) == 0)  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)   /* next column */
			sx += 16;
		else
		{
			sx = mexico86_objectram[offs + 2];
//          if (gfx_attr & 0x40) sx -= 256;
		}
		sy = 256 - height*8 - (mexico86_objectram[offs + 0]);

		for (xc = 0;xc < 2;xc++)
		{
			for (yc = 0;yc < height;yc++)
			{
				int goffs,code,color,flipx,flipy,x,y;

				goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				code = mexico86_videoram[goffs] + ((mexico86_videoram[goffs + 1] & 0x1f) << 8);
				color = (mexico86_videoram[goffs + 1] & 0xe0) >> 5;
				flipx = 0;
				flipy = 0;
//              x = sx + xc * 8;
				x = (sx + xc * 8) & 0xff;
				y = (sy + yc * 8) & 0xff;

				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flipx,flipy,
						x,y,
						&machine->screen[0].visarea,TRANSPARENCY_PEN,15);
			}
		}
	}
	return 0;
}
#endif

VIDEO_UPDATE( kikikai )
{
	int offs;
	int sx,sy,yc;
	int gfx_num,gfx_attr,gfx_offs;
	int height;
	int goffs,code,color,y;
	int tx, ty;

	fillbitmap(bitmap, get_black_pen(machine), &machine->screen[0].visarea);
	sx = 0;
	for (offs=0; offs<mexico86_objectram_size; offs+=4)
	{
		if (*(UINT32*)(mexico86_objectram + offs) == 0) continue;

		ty = mexico86_objectram[offs];
		gfx_num = mexico86_objectram[offs + 1];
		tx = mexico86_objectram[offs + 2];
		gfx_attr = mexico86_objectram[offs + 3];

		if (gfx_num & 0x80)
		{
			gfx_offs = ((gfx_num & 0x3f) << 7);
			height = 32;
			if (gfx_num & 0x40) sx += 16;
			else sx = tx;
		}
		else
		{
			if (!(ty && tx)) continue;
			gfx_offs = ((gfx_num & 0x1f) << 7) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
			sx = tx;
		}
		sy = 256 - (height << 3) - ty;

		height <<= 1;
		for (yc=0; yc<height; yc+=2)
		{
			y = (sy + (yc << 2)) & 0xff;
			goffs = gfx_offs + yc;
			code = mexico86_videoram[goffs] + ((mexico86_videoram[goffs + 1] & 0x1f) << 8);
			color = (mexico86_videoram[goffs + 1] & 0xe0) >> 5;
			goffs += 0x40;

			drawgfx(bitmap,machine->gfx[0],
					code,
					color,
					0,0,
					sx&0xff,y,
					&machine->screen[0].visarea,TRANSPARENCY_PEN,15);

			code = mexico86_videoram[goffs] + ((mexico86_videoram[goffs + 1] & 0x1f) << 8);
			color = (mexico86_videoram[goffs + 1] & 0xe0) >> 5;

			drawgfx(bitmap,machine->gfx[0],
					code,
					color,
					0,0,
					(sx+8)&0xff,y,
					&machine->screen[0].visarea,TRANSPARENCY_PEN,15);
		}
	}
	return 0;
}
//ZT
