/***************************************************************************

    Acorn Archimedes VIDC (VIDeo Controller) emulation

***************************************************************************/

#include "emu.h"
#include "includes/archimds.h"

VIDEO_START( archimds_vidc )
{
}

SCREEN_UPDATE_RGB32( archimds_vidc )
{
	int xstart,ystart,xend,yend;
	int res_x,res_y;
	int xsize,ysize;
	int calc_dxs = 0,calc_dxe = 0;
	const UINT8 x_step[4] = { 5, 7, 11, 19 };

	/* border color */
	bitmap.fill(screen.machine().pens[0x10], cliprect);

	/* define X display area through BPP mode register */
	calc_dxs = (vidc_regs[VIDC_HDSR]*2)+x_step[vidc_bpp_mode & 3];
	calc_dxe = (vidc_regs[VIDC_HDER]*2)+x_step[vidc_bpp_mode & 3];

	/* now calculate display clip rectangle start/end areas */
	xstart = (calc_dxs)-vidc_regs[VIDC_HBSR];
	ystart = (vidc_regs[VIDC_VDSR])-vidc_regs[VIDC_VBSR];
	xend = (calc_dxe)+xstart;
	yend = vidc_regs[VIDC_VDER]+ystart;

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	xsize = calc_dxe-calc_dxs;
	ysize = vidc_regs[VIDC_VDER]-vidc_regs[VIDC_VDSR];

	{
		int count;
		int x,y,xi;
		UINT8 pen;
		static UINT8 *vram = screen.machine().root_device().memregion("vram")->base();

		count = (0);

		switch(vidc_bpp_mode)
		{
			case 0: //1 bpp
			{
				for(y=0;y<ysize;y++)
				{
					for(x=0;x<xsize;x+=8)
					{
						pen = vram[count];

						for(xi=0;xi<8;xi++)
						{
							res_x = x+xi+xstart;
							res_y = (y+ystart)*(vidc_interlace+1);

							if(vidc_interlace)
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = screen.machine().pens[(pen>>(xi))&0x1];
								if (cliprect.contains(res_x, res_y+1) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = screen.machine().pens[(pen>>(xi))&0x1];
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = screen.machine().pens[(pen>>(xi))&0x1];
							}
						}

						count++;
					}
				}
			}
			break;
			case 3: //8 bpp
			{
				for(y=0;y<ysize;y++)
				{
					for(x=0;x<xsize;x++)
					{
						pen = vram[count];

						res_x = x+xstart;
						res_y = (y+ystart)*(vidc_interlace+1);

						if(vidc_interlace)
						{
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
								bitmap.pix32(res_y, res_x) = screen.machine().pens[(pen&0xff)+0x100];
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y+1) <= yend)
								bitmap.pix32(res_y+1, res_x) = screen.machine().pens[(pen&0xff)+0x100];
						}
						else
						{
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
								bitmap.pix32(res_y, res_x) = screen.machine().pens[(pen&0xff)+0x100];
						}

						count++;
					}
				}
			}
			break;
		}
	}

	return 0;
}
