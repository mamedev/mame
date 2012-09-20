/***************************************************************************

    Acorn Archimedes VIDC (VIDeo Controller) emulation

***************************************************************************/

#include "emu.h"
#include "includes/archimds.h"

UINT32 archimedes_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xstart,ystart,xend,yend;
	int res_x,res_y;
	int xsize,ysize;
	int calc_dxs = 0,calc_dxe = 0;
	const UINT8 x_step[4] = { 5, 7, 11, 19 };

	/* border color */
	bitmap.fill(machine().pens[0x10], cliprect);

	/* define X display area through BPP mode register */
	calc_dxs = (m_vidc_regs[VIDC_HDSR]*2)+x_step[m_vidc_bpp_mode & 3];
	calc_dxe = (m_vidc_regs[VIDC_HDER]*2)+x_step[m_vidc_bpp_mode & 3];

	/* now calculate display clip rectangle start/end areas */
	xstart = (calc_dxs)-m_vidc_regs[VIDC_HBSR];
	ystart = (m_vidc_regs[VIDC_VDSR])-m_vidc_regs[VIDC_VBSR];
	xend = (calc_dxe)+xstart;
	yend = m_vidc_regs[VIDC_VDER]+ystart;

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	xsize = calc_dxe-calc_dxs;
	ysize = m_vidc_regs[VIDC_VDER]-m_vidc_regs[VIDC_VDSR];

	{
		int count;
		int x,y,xi;
		UINT8 pen;
		static UINT8 *vram = machine().root_device().memregion("vram")->base();

		count = (0);

		switch(m_vidc_bpp_mode)
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
							res_y = (y+ystart)*(m_vidc_interlace+1);

							if(m_vidc_interlace)
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = machine().pens[(pen>>(xi))&0x1];
								if (cliprect.contains(res_x, res_y+1) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = machine().pens[(pen>>(xi))&0x1];
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = machine().pens[(pen>>(xi))&0x1];
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
						res_y = (y+ystart)*(m_vidc_interlace+1);

						if(m_vidc_interlace)
						{
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
								bitmap.pix32(res_y, res_x) = machine().pens[(pen&0xff)+0x100];
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y+1) <= yend)
								bitmap.pix32(res_y+1, res_x) = machine().pens[(pen&0xff)+0x100];
						}
						else
						{
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
								bitmap.pix32(res_y, res_x) = machine().pens[(pen&0xff)+0x100];
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
