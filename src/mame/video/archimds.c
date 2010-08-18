/***************************************************************************

	Acorn Archimedes VIDC (VIDeo Controller) emulation

***************************************************************************/

#include "emu.h"
#include "includes/archimds.h"

VIDEO_START( archimds_vidc )
{
}

VIDEO_UPDATE( archimds_vidc )
{
	int xstart,ystart,xend,yend;

	/* border color */
	bitmap_fill(bitmap, cliprect, screen->machine->pens[0x10]);

	/* display area x/y */
	xstart = vidc_regs[VIDC_HDSR];
	ystart = vidc_regs[VIDC_VDSR];
	xend = vidc_regs[VIDC_HDER];
	yend = vidc_regs[VIDC_VDER];

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	{
		int count;
		int x,y,xi;
		UINT8 pen;
		static UINT8 *vram = memory_region(screen->machine,"vram");

		count = 0;

		switch(vidc_bpp_mode)
		{
			case 0: //1 bpp
			{
				for(y=0;y<400;y++)
				{
					for(x=0;x<640;x+=8)
					{
						pen = vram[count];

						for(xi=0;xi<8;xi++)
						{
							if ((x+xi+xstart) <= screen->visible_area().max_x && (y+ystart) <= screen->visible_area().max_y &&
							    (x+xi+xstart) <= xend && (y+ystart) <= yend)
								*BITMAP_ADDR32(bitmap, y+ystart, x+xi+xstart) = screen->machine->pens[(pen>>(xi))&0x1];
						}

						count++;
					}
				}
			}
			break;
			case 3: //8 bpp
			{
				for(y=0;y<400;y++)
				{
					for(x=0;x<640;x++)
					{
						pen = vram[count];

						if ((x+xstart) <= screen->visible_area().max_x && (y+ystart) <= screen->visible_area().max_y &&
						    (x+xstart) <= xend && (y+ystart) <= yend)
								*BITMAP_ADDR32(bitmap, y+ystart, x+xstart) = screen->machine->pens[(pen&0xff)+0x100];

						count++;
					}
				}
			}
			break;
		}
	}

	return 0;
}