// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
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
	bitmap.fill(m_palette->pen(0x10), cliprect);

	/* define X display area through BPP mode register */
	calc_dxs = (m_vidc_regs[VIDC_HDSR]*2)+x_step[m_vidc_bpp_mode & 3];
	calc_dxe = (m_vidc_regs[VIDC_HDER]*2)+x_step[m_vidc_bpp_mode & 3];

	/* now calculate display clip rectangle start/end areas */
	xstart = (calc_dxs)-m_vidc_regs[VIDC_HBSR];
	ystart = (m_vidc_regs[VIDC_VDSR]-m_vidc_regs[VIDC_VBSR]);
	xend = (calc_dxe)+xstart;
	yend = (m_vidc_regs[VIDC_VDER] * (m_vidc_interlace+1))+ystart;

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	xsize = calc_dxe-calc_dxs;
	ysize = m_vidc_regs[VIDC_VDER]-m_vidc_regs[VIDC_VDSR];

	{
		int count;
		int x,y,xi;
		UINT8 pen;
		static UINT8 *vram = memregion("vram")->base();

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
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi))&0x1);
								if (cliprect.contains(res_x, res_y+1) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = m_palette->pen((pen>>(xi))&0x1);
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi))&0x1);
							}
						}

						count++;
					}
				}
			}
			break;
			case 1: //2 bpp
			{
				for(y=0;y<ysize;y++)
				{
					for(x=0;x<xsize;x+=4)
					{
						pen = vram[count];

						for(xi=0;xi<4;xi++)
						{
							res_x = x+xi+xstart;
							res_y = (y+ystart)*(m_vidc_interlace+1);

							if(m_vidc_interlace)
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi*2))&0x3);
								if (cliprect.contains(res_x, res_y+1) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = m_palette->pen((pen>>(xi*2))&0x3);
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi*2))&0x3);
							}
						}

						count++;
					}
				}
			}
			break;
			case 2: //4 bpp
			{
				for(y=0;y<ysize;y++)
				{
					for(x=0;x<xsize;x+=2)
					{
						pen = vram[count];

						for(xi=0;xi<2;xi++)
						{
							res_x = x+xi+xstart;
							res_y = (y+ystart)*(m_vidc_interlace+1);

							if(m_vidc_interlace)
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi*4))&0xf);
								if (cliprect.contains(res_x, res_y+1) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = m_palette->pen((pen>>(xi*4))&0xf);
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen((pen>>(xi*4))&0xf);
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
								bitmap.pix32(res_y, res_x) = m_palette->pen((pen&0xff)+0x100);
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y+1) <= yend)
								bitmap.pix32(res_y+1, res_x) = m_palette->pen((pen&0xff)+0x100);
						}
						else
						{
							if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
								bitmap.pix32(res_y, res_x) = m_palette->pen((pen&0xff)+0x100);
						}

						count++;
					}
				}
			}
			break;
			default:
				popmessage("Unemulated bpp mode %02x, contact MAME/MESSdev",m_vidc_bpp_mode);
				break;
		}


		if(m_cursor_enabled == true)
		{
			count = 0;
			for(y=0;y<16;y++)
			{
				for(x=0;x<32;x+=4)
				{
					for(xi=0;xi<4;xi++)
					{
						UINT8 cursor_dot;
						pen = m_cursor_vram[count];

						res_x = x+xi+xstart;
						res_y = (y+ystart)*(m_vidc_interlace+1);

						cursor_dot = ((pen>>(xi*2))&0x3);

						if(cursor_dot)
						{
							if(m_vidc_interlace)
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen(cursor_dot+0x10);
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y+1) <= yend)
									bitmap.pix32(res_y+1, res_x) = m_palette->pen(cursor_dot+0x10);
							}
							else
							{
								if (cliprect.contains(res_x, res_y) && (res_x) <= xend && (res_y) <= yend)
									bitmap.pix32(res_y, res_x) = m_palette->pen(cursor_dot+0x10);
							}
						}
					}

					count++;
				}
			}
		}
	}



	return 0;
}
