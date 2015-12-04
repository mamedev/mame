// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl
/*

    Sharp X68000 video functions
    driver by Barry Rodewald

    X68000 video hardware (there are some minor revisions to these custom chips across various X680x0 models):
        Custom sprite controller "Cynthia"
        Custom CRT controller "Vinas / Vicon"
        Custom video controller "VSOP / VIPS"
        Custom video data selector "Cathy"

    In general terms:
        1 "Text" layer - effectively a 4bpp bitmap split into 4 planes at 1bpp each
                         512kB "text" VRAM
                         can write to multiple planes at once
                         can copy one character line to another character line
                         is 1024x1024 in size
        Up to 4 graphic layers - can be 4 layers with a 16 colour palette, 2 layers with a 256 colour palette,
                                 or 1 layer at 16-bit RGB.
                                 512k graphic VRAM
                                 all layers are 512x512, but at 16 colours, the 4 layers can be combined into 1 1024x1024 layer
                                 one or more layers can be cleared at once quickly with a simple hardware function
         2 tilemapped layers - can be 8x8 or 16x16, 16 colours per tile, max 256 colours overall
         1 sprite layer - up to 128 16x16 sprites, 16 colours per sprite, maximum 16 sprites per scanline (not yet implemented).

         Questions: What do the other bits in m_video.reg[2] do?
                    What is "special priority mode"?
                    How is the intensity applied during blending if at all?
                    Black appears to be opaque only at priority 2 but not 3, is that right?
                    Are the gfx layers blended from the bottom up or all at once?

*/

#include "emu.h"

#include "machine/mc68901.h"
#include "includes/x68k.h"
#include "machine/ram.h"


PALETTE_DECODER_MEMBER(x68k_state, GGGGGRRRRRBBBBBI)
{
	UINT8 i = raw & 1;
	UINT8 r = pal6bit(((raw >> 5) & 0x3e) | i);
	UINT8 g = pal6bit(((raw >> 10) & 0x3e) | i);
	UINT8 b = pal6bit(((raw >> 0) & 0x3e) | i);
	return rgb_t(r, g, b);
}

inline void x68k_state::x68k_plot_pixel(bitmap_rgb32 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix32(y, x) = (UINT16)color;
}
/*
bitmap_rgb32* ::x68k_get_gfx_page(int pri,int type)
{
    if(type == GFX16)
    {
        switch(pri)
        {
        case 0:
            return x68k_gfx_0_bitmap_16;
        case 1:
            return x68k_gfx_1_bitmap_16;
        case 2:
            return x68k_gfx_2_bitmap_16;
        case 3:
            return x68k_gfx_3_bitmap_16;
        default:
            return x68k_gfx_0_bitmap_16;  // should never reach here.
        }
    }
    if(type == GFX256)
    {
        switch(pri)
        {
        case 0:
        case 1:
            return x68k_gfx_0_bitmap_256;
        case 2:
        case 3:
            return x68k_gfx_1_bitmap_256;
        default:
            return x68k_gfx_0_bitmap_256;  // should never reach here.
        }
    }
    if(type == GFX65536)
        return x68k_gfx_0_bitmap_65536;

    return NULL;  // should never reach here either.
}
*/
void x68k_state::x68k_crtc_text_copy(int src, int dest, UINT8 planes)
{
	// copys one raster in T-VRAM to another raster
	int src_ram = src * 256;  // 128 bytes per scanline
	int dest_ram = dest * 256;

	// update RAM in each plane
	if(planes & 1)
		memcpy(&m_tvram[dest_ram],&m_tvram[src_ram],512);
	if(planes & 2)
		memcpy(&m_tvram[dest_ram+0x10000],&m_tvram[src_ram+0x10000],512);
	if(planes & 4)
		memcpy(&m_tvram[dest_ram+0x20000],&m_tvram[src_ram+0x20000],512);
	if(planes & 8)
		memcpy(&m_tvram[dest_ram+0x30000],&m_tvram[src_ram+0x30000],512);
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_operation_end)
{
	int bit = param;
	m_crtc.operation &= ~bit;
}

void x68k_state::x68k_crtc_refresh_mode()
{
//  rectangle rect;
//  double scantime;
	rectangle scr,visiblescr;
	int length;

	// Calculate data from register values
	m_crtc.vmultiple = 1;
	if((m_crtc.reg[20] & 0x10) != 0 && (m_crtc.reg[20] & 0x0c) == 0)
		m_crtc.vmultiple = 2;  // 31.5kHz + 256 lines = doublescan
	if(m_crtc.interlace != 0)
		m_crtc.vmultiple = 0.5f;  // 31.5kHz + 1024 lines or 15kHz + 512 lines = interlaced
	m_crtc.htotal = (m_crtc.reg[0] + 1) * 8;
	m_crtc.vtotal = (m_crtc.reg[4] + 1) / m_crtc.vmultiple; // default is 567 (568 scanlines)
	m_crtc.hbegin = (m_crtc.reg[2] * 8) + 1;
	m_crtc.hend = (m_crtc.reg[3] * 8);
	m_crtc.vbegin = (m_crtc.reg[6]) / m_crtc.vmultiple;
	m_crtc.vend = (m_crtc.reg[7] - 1) / m_crtc.vmultiple;
	if((m_crtc.vmultiple == 2) && !(m_crtc.reg[7] & 1)) // otherwise if the raster irq line == vblank line, the raster irq fires too late
		m_crtc.vend++;
	m_crtc.hsync_end = (m_crtc.reg[1]) * 8;
	m_crtc.vsync_end = (m_crtc.reg[5]) / m_crtc.vmultiple;
	m_crtc.hsyncadjust = m_crtc.reg[8];
	scr.set(0, m_crtc.htotal - 8, 0, m_crtc.vtotal);
	if(scr.max_y <= m_crtc.vend)
		scr.max_y = m_crtc.vend + 2;
	if(scr.max_x <= m_crtc.hend)
		scr.max_x = m_crtc.hend + 2;
	visiblescr.set(m_crtc.hbegin, m_crtc.hend, m_crtc.vbegin, m_crtc.vend);

	// expand visible area to the size indicated by CRTC reg 20
	length = m_crtc.hend - m_crtc.hbegin;
	if (length < m_crtc.width)
	{
		visiblescr.min_x = m_crtc.hbegin - ((m_crtc.width - length)/2);
		visiblescr.max_x = m_crtc.hend + ((m_crtc.width - length)/2);
	}
	length = m_crtc.vend - m_crtc.vbegin;
	if (length < m_crtc.height)
	{
		visiblescr.min_y = m_crtc.vbegin - ((m_crtc.height - length)/2);
		visiblescr.max_y = m_crtc.vend + ((m_crtc.height - length)/2);
	}
	// bounds check
	if(visiblescr.min_x < 0)
		visiblescr.min_x = 0;
	if(visiblescr.min_y < 0)
		visiblescr.min_y = 0;
	if(visiblescr.max_x >= scr.max_x)
		visiblescr.max_x = scr.max_x - 2;
	if(visiblescr.max_y >= scr.max_y - 1)
		visiblescr.max_y = scr.max_y - 2;

//  logerror("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i\n",m_crtc.reg[0],m_crtc.reg[1],m_crtc.reg[2],m_crtc.reg[3],
//      m_crtc.reg[4],m_crtc.reg[5],m_crtc.reg[6],m_crtc.reg[7],m_crtc.reg[8],m_crtc.reg[9]);
	logerror("video_screen_configure(machine.first_screen(),%i,%i,[%i,%i,%i,%i],55.45)\n",scr.max_x,scr.max_y,visiblescr.min_x,visiblescr.min_y,visiblescr.max_x,visiblescr.max_y);
	m_screen->configure(scr.max_x,scr.max_y,visiblescr,HZ_TO_ATTOSECONDS(55.45));
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_hsync)
{
	int hstate = param;
	attotime hsync_time;

	m_crtc.hblank = hstate;
	m_mfpdev->i7_w(!m_crtc.hblank);

	if(m_crtc.operation & 8)
		x68k_crtc_text_copy((m_crtc.reg[22] & 0xff00) >> 8,(m_crtc.reg[22] & 0x00ff),(m_crtc.reg[21] & 0xf));

	if(m_crtc.vmultiple == 2) // 256-line (doublescan)
	{
		if(hstate == 1)
		{
			if(m_oddscanline == 1)
			{
				int scan = m_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				hsync_time = m_screen->time_until_pos(scan,(m_crtc.htotal + m_crtc.hend) / 2);
				m_scanline_timer->adjust(hsync_time);
				if(scan != 0)
				{
					if((ioport("options")->read() & 0x04))
					{
						m_screen->update_partial(scan);
					}
				}
			}
			else
			{
				int scan = m_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				hsync_time = m_screen->time_until_pos(scan,m_crtc.hend / 2);
				m_scanline_timer->adjust(hsync_time);
				if(scan != 0)
				{
					if((ioport("options")->read() & 0x04))
					{
						m_screen->update_partial(scan);
					}
				}
			}
		}
		if(hstate == 0)
		{
			if(m_oddscanline == 1)
			{
				int scan = m_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				else
					scan++;
				hsync_time = m_screen->time_until_pos(scan,m_crtc.hbegin / 2);
				m_scanline_timer->adjust(hsync_time, 1);
				m_oddscanline = 0;
			}
			else
			{
				hsync_time = m_screen->time_until_pos(m_screen->vpos(),(m_crtc.htotal + m_crtc.hbegin) / 2);
				m_scanline_timer->adjust(hsync_time, 1);
				m_oddscanline = 1;
			}
		}
	}
	else  // 512-line
	{
		if(hstate == 1)
		{
			int scan = m_screen->vpos();
			if(scan > m_crtc.vend)
				scan = 0;
			hsync_time = m_screen->time_until_pos(scan,m_crtc.hend);
			m_scanline_timer->adjust(hsync_time);
			if(scan != 0)
			{
				if((ioport("options")->read() & 0x04))
				{
					m_screen->update_partial(scan);
				}
			}
		}
		if(hstate == 0)
		{
			hsync_time = m_screen->time_until_pos(m_screen->vpos()+1,m_crtc.hbegin);
			m_scanline_timer->adjust(hsync_time, 1);
		}
	}
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_raster_end)
{
	m_mfpdev->i6_w(1);
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_raster_irq)
{
	int scan = param;
	attotime irq_time;
	attotime end_time;

	if(scan <= m_crtc.vtotal)
	{
		m_mfpdev->i6_w(0);
		m_screen->update_partial(scan);
		irq_time = m_screen->time_until_pos(scan,m_crtc.hbegin);
		// end of HBlank period clears GPIP6 also?
		end_time = m_screen->time_until_pos(scan,m_crtc.hend);
		m_raster_irq->adjust(irq_time, scan);
		timer_set(end_time, TIMER_X68K_CRTC_RASTER_END);
		logerror("GPIP6: Raster triggered at line %i (%i)\n",scan,m_screen->vpos());
	}
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_vblank_irq)
{
	int val = param;
	attotime irq_time;
	int vblank_line;

	if(val == 1)  // V-DISP on
	{
		m_crtc.vblank = 1;
		vblank_line = m_crtc.vbegin;
		irq_time = m_screen->time_until_pos(vblank_line,2);
		m_vblank_irq->adjust(irq_time);
		logerror("CRTC: VBlank on\n");
	}
	if(val == 0)  // V-DISP off
	{
		m_crtc.vblank = 0;
		vblank_line = m_crtc.vend;
		if(vblank_line > m_crtc.vtotal)
			vblank_line = m_crtc.vtotal;
		irq_time = m_screen->time_until_pos(vblank_line,2);
		m_vblank_irq->adjust(irq_time, 1);
		logerror("CRTC: VBlank off\n");
	}

	m_mfpdev->tai_w(!m_crtc.vblank);
	m_mfpdev->i4_w(!m_crtc.vblank);
}


// CRTC "VINAS 1+2 / VICON" at 0xe80000
/* 0xe80000 - Registers (all are 16-bit):
 * 0 - Horizontal Total (in characters)
 * 1 - Horizontal Sync End
 * 2 - Horizontal Display Begin
 * 3 - Horizontal Display End
 * 4 - Vertical Total (in scanlines)
 * 5 - Vertical Sync End
 * 6 - Vertical Display Begin
 * 7 - Vertical Display End
 * 8 - Fine Horizontal Sync Adjustment
 * 9 - Raster Line (for Raster IRQ mapped to MFP GPIP6)
 * 10/11 - Text Layer X and Y Scroll
 * 12/13 - Graphic Layer 0 X and Y Scroll
 * 14/15 - Graphic Layer 1 X and Y Scroll
 * 16/17 - Graphic Layer 2 X and Y Scroll
 * 18/19 - Graphic Layer 3 X and Y Scroll
 * 20 - bit 12 - Text VRAM mode : 0 = display, 1 = buffer
 *      bit 11 - Graphic VRAM mode : 0 = display, 1 = buffer
 *      bit 10 - "Real" screen size : 0 = 512x512, 1 = 1024x1024
 *      bits 8,9 - Colour mode :
 *                 00 = 16 colour      01 = 256 colour
 *                 10 = Undefined      11 = 65,536 colour
 *      bit 4 - Horizontal Frequency : 0 = 15.98kHz, 1 = 31.50kHz
 *      bits 2,3 - Vertical dots :
 *                 00 = 256            01 = 512
 *                 10 or 11 = 1024 (interlaced)
 *      bits 0,1 - Horizontal dots :
 *                 00 = 256            01 = 512
 *                 10 = 768            11 = 50MHz clock mode (Compact XVI or later)
 * 21 - bit 9 - Text Screen Access Mask Enable
 *      bit 8 - Text Screen Simultaneous Plane Access Enable
 *      bits 4-7 - Text Screen Simultaneous Plane Access Select
 *      bits 0-3 - Text Screen Line Copy Plane Select
 *                 Graphic Screen High-speed Clear Page Select
 * 22 - Text Screen Line Copy
 *      bits 15-8 - Source Line
 *      bits 7-0 - Destination Line
 * 23 - Text Screen Mask Pattern
 *
 * 0xe80481 - Operation Port (8-bit):
 *      bit 3 - Text Screen Line Copy Begin
 *      bit 1 - Graphic Screen High-speed Clear Begin
 *      bit 0 - Image Taking Begin (?)
 *    Operation Port bits are cleared automatically when the requested
 *    operation is completed.
 */
WRITE16_MEMBER(x68k_state::x68k_crtc_w )
{
	if (offset < 0x24)
		COMBINE_DATA(m_crtc.reg+offset);
	switch(offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		x68k_crtc_refresh_mode();
		break;
	case 9:  // CRTC raster IRQ (GPIP6)
		{
			attotime irq_time;
			data = m_crtc.reg[9];
			irq_time = m_screen->time_until_pos((data) / m_crtc.vmultiple,2);

			if(irq_time.as_double() > 0)
				m_raster_irq->adjust(irq_time, (data) / m_crtc.vmultiple);
		}
		logerror("CRTC: Write to raster IRQ register - %i\n",data);
		break;
	case 20:
		if(ACCESSING_BITS_0_7)
		{
			m_crtc.interlace = 0;
			switch(data & 0x0c)
			{
			case 0x00:
				m_crtc.height = 256;
				break;
			case 0x08:
			case 0x0c:  // TODO: 1024 vertical, if horizontal freq = 31kHz
				m_crtc.height = 512;
				m_crtc.interlace = 1;  // if 31kHz, 1024 lines = interlaced
				break;
			case 0x04:
				m_crtc.height = 512;
				if(!(m_crtc.reg[20] & 0x0010))  // if 15kHz, 512 lines = interlaced
					m_crtc.interlace = 1;
				break;
			}
			switch(data & 0x03)
			{
			case 0x00:
				m_crtc.width = 256;
				break;
			case 0x01:
				m_crtc.width = 512;
				break;
			case 0x02:
			case 0x03:  // 0x03 = 50MHz clock mode (XVI only)
				m_crtc.width = 768;
				break;
			}
		}
/*      if(ACCESSING_BITS_8_15)
        {
            m_crtc.interlace = 0;
            if(data & 0x0400)
                m_crtc.interlace = 1;
        }*/
		x68k_crtc_refresh_mode();
		break;
	case 576:  // operation register
		m_crtc.operation = data;
		if(data & 0x02)  // high-speed graphic screen clear
		{
			memset(&m_gvram[0],0,0x40000);
			timer_set(attotime::from_msec(10), TIMER_X68K_CRTC_OPERATION_END, 0x02);  // time taken to do operation is a complete guess.
		}
		break;
	}
//  logerror("CRTC: [%08x] Wrote %04x to CRTC register %i\n",m_maincpu->safe_pc(),data,offset);
}

READ16_MEMBER(x68k_state::x68k_crtc_r )
{
	if(offset < 24)
	{
//      logerror("CRTC: [%08x] Read %04x from CRTC register %i\n",m_maincpu->safe_pc(),m_crtc.reg[offset],offset);
		switch(offset)
		{
		case 9:
			return 0;
		case 10:  // Text X/Y scroll
		case 11:
		case 12:  // Graphic layer 0 scroll
		case 13:
			return m_crtc.reg[offset] & 0x3ff;
		case 14:  // Graphic layer 1 scroll
		case 15:
		case 16:  // Graphic layer 2 scroll
		case 17:
		case 18:  // Graphic layer 3 scroll
		case 19:
			return m_crtc.reg[offset] & 0x1ff;
		default:
			return m_crtc.reg[offset];
		}
	}
	if(offset == 576) // operation port, operation bits are set to 0 when operation is complete
		return m_crtc.operation;
//  logerror("CRTC: [%08x] Read from unknown CRTC register %i\n",activecpu_get_pc(),offset);
	return 0xffff;
}

WRITE16_MEMBER(x68k_state::x68k_gvram_w )
{
//  int xloc,yloc,pageoffset;
	/*
	   G-VRAM usage is determined by colour depth and "real" screen size.

	   For screen size of 1024x1024, all G-VRAM space is used, in one big page.
	   At 1024x1024 real screen size, colour depth is always 4bpp, and ranges from
	   0xc00000-0xdfffff.

	   For screen size of 512x512, the colour depth determines the page usage.
	   16 colours = 4 pages
	   256 colours = 2 pages
	   65,536 colours = 1 page
	   Page 1 - 0xc00000-0xc7ffff    Page 2 - 0xc80000-0xcfffff
	   Page 3 - 0xd00000-0xd7ffff    Page 4 - 0xd80000-0xdfffff
	*/

	// handle different G-VRAM page setups
	if(m_crtc.reg[20] & 0x0800)  // G-VRAM set to buffer
	{
		if(offset < 0x40000)
			COMBINE_DATA(&m_gvram[offset]);
	}
	else
	{
		switch(m_crtc.reg[20] & 0x0300)
		{
			case 0x0300:
				if(offset < 0x40000)
					COMBINE_DATA(&m_gvram[offset]);
				break;
			case 0x0100:
				if(offset < 0x40000)
				{
					m_gvram[offset] = (m_gvram[offset] & 0xff00) | (data & 0x00ff);
				}
				if(offset >= 0x40000 && offset < 0x80000)
				{
					m_gvram[offset-0x40000] = (m_gvram[offset-0x40000] & 0x00ff) | ((data & 0x00ff) << 8);
				}
				break;
			case 0x0000:
				if(offset < 0x40000)
				{
					m_gvram[offset] = (m_gvram[offset] & 0xfff0) | (data & 0x000f);
				}
				if(offset >= 0x40000 && offset < 0x80000)
				{
					m_gvram[offset-0x40000] = (m_gvram[offset-0x40000] & 0xff0f) | ((data & 0x000f) << 4);
				}
				if(offset >= 0x80000 && offset < 0xc0000)
				{
					m_gvram[offset-0x80000] = (m_gvram[offset-0x80000] & 0xf0ff) | ((data & 0x000f) << 8);
				}
				if(offset >= 0xc0000 && offset < 0x100000)
				{
					m_gvram[offset-0xc0000] = (m_gvram[offset-0xc0000] & 0x0fff) | ((data & 0x000f) << 12);
				}
				break;
			default:
				logerror("G-VRAM written while layer setup is undefined.\n");
		}
	}
}

WRITE16_MEMBER(x68k_state::x68k_tvram_w )
{
	UINT16 text_mask;

	text_mask = ~(m_crtc.reg[23]) & mem_mask;

	if(!(m_crtc.reg[21] & 0x0200)) // text access mask enable
		text_mask = 0xffff & mem_mask;

	mem_mask = text_mask;

	if(m_crtc.reg[21] & 0x0100)
	{  // simultaneous T-VRAM plane access (I think ;))
		int plane,wr;
		offset = offset & 0x00ffff;
		wr = (m_crtc.reg[21] & 0x00f0) >> 4;
		for(plane=0;plane<4;plane++)
		{
			if(wr & (1 << plane))
			{
				COMBINE_DATA(&m_tvram[offset+0x10000*plane]);
			}
		}
	}
	else
	{
		COMBINE_DATA(&m_tvram[offset]);
	}
}

READ16_MEMBER(x68k_state::x68k_gvram_r )
{
	UINT16 ret = 0;

	if(m_crtc.reg[20] & 0x0800)  // G-VRAM set to buffer
		return m_gvram[offset];

	switch(m_crtc.reg[20] & 0x0300)  // colour setup determines G-VRAM use
	{
		case 0x0300: // 65,536 colour (RGB) - 16-bits per word
			if(offset < 0x40000)
				ret = m_gvram[offset];
			else
				ret = 0xffff;
			break;
		case 0x0100:  // 256 colour (paletted) - 8 bits per word
			if(offset < 0x40000)
				ret = m_gvram[offset] & 0x00ff;
			if(offset >= 0x40000 && offset < 0x80000)
				ret = (m_gvram[offset-0x40000] & 0xff00) >> 8;
			if(offset >= 0x80000)
				ret = 0xffff;
			break;
		case 0x0000:  // 16 colour (paletted) - 4 bits per word
			if(offset < 0x40000)
				ret = m_gvram[offset] & 0x000f;
			if(offset >= 0x40000 && offset < 0x80000)
				ret = (m_gvram[offset-0x40000] & 0x00f0) >> 4;
			if(offset >= 0x80000 && offset < 0xc0000)
				ret = (m_gvram[offset-0x80000] & 0x0f00) >> 8;
			if(offset >= 0xc0000 && offset < 0x100000)
				ret = (m_gvram[offset-0xc0000] & 0xf000) >> 12;
			break;
		default:
			logerror("G-VRAM read while layer setup is undefined.\n");
			ret = 0xffff;
	}

	return ret;
}

READ16_MEMBER(x68k_state::x68k_tvram_r )
{
	return m_tvram[offset];
}

WRITE16_MEMBER(x68k_state::x68k_spritereg_w )
{
	COMBINE_DATA(&m_spritereg[offset]);
	switch(offset)
	{
	case 0x400:
		m_bg0_8->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		m_bg0_16->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		break;
	case 0x401:
		m_bg0_8->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		m_bg0_16->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		break;
	case 0x402:
		m_bg1_8->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		m_bg1_16->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		break;
	case 0x403:
		m_bg1_8->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		m_bg1_16->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		break;
	case 0x406:  // BG H-DISP (normally equals CRTC reg 2 value + 4)
		if(data != 0x00ff)
		{
			m_crtc.bg_visible_width = (m_crtc.reg[3] - ((data & 0x003f) - 4)) * 8;
			m_crtc.bg_hshift = ((data - (m_crtc.reg[2]+4)) * 8);
			if(m_crtc.bg_hshift > 0)
				m_crtc.bg_hshift = 0;
		}
		break;
	case 0x407:  // BG V-DISP (like CRTC reg 6)
		m_crtc.bg_vshift = m_crtc.vshift;
		break;
	case 0x408:  // BG H/V-Res
		m_crtc.bg_hvres = data & 0x1f;
		if(data != 0xff)
		{  // Handle when the PCG is using 256 and the CRTC is using 512
			if((m_crtc.bg_hvres & 0x0c) == 0x00 && (m_crtc.reg[20] & 0x0c) == 0x04)
				m_crtc.bg_double = 2;
			else
				m_crtc.bg_double = 1;
		}
		else
			m_crtc.bg_double = 1;
		break;
	}
}

READ16_MEMBER(x68k_state::x68k_spritereg_r )
{
	if(offset >= 0x400 && offset < 0x404)
		return m_spritereg[offset] & 0x3ff;
	return m_spritereg[offset];
}

WRITE16_MEMBER(x68k_state::x68k_spriteram_w )
{
	COMBINE_DATA(m_spriteram+offset);
	m_video.tile8_dirty[offset / 16] = 1;
	m_video.tile16_dirty[offset / 64] = 1;
	if(offset < 0x2000)
	{
		m_bg1_8->mark_all_dirty();
		m_bg1_16->mark_all_dirty();
		m_bg0_8->mark_all_dirty();
		m_bg0_16->mark_all_dirty();
	}
	if(offset >= 0x2000 && offset < 0x3000)
	{
		m_bg1_8->mark_tile_dirty(offset & 0x0fff);
		m_bg1_16->mark_tile_dirty(offset & 0x0fff);
	}
	if(offset >= 0x3000)
	{
		m_bg0_8->mark_tile_dirty(offset & 0x0fff);
		m_bg0_16->mark_tile_dirty(offset & 0x0fff);
	}
}

READ16_MEMBER(x68k_state::x68k_spriteram_r )
{
	return m_spriteram[offset];
}

void x68k_state::x68k_draw_text(bitmap_rgb32 &bitmap, int xscr, int yscr, rectangle rect)
{
	unsigned int line,pixel; // location on screen
	UINT32 loc;  // location in TVRAM
	UINT32 colour;
	int bit;

	for(line=rect.min_y;line<=rect.max_y;line++)  // per scanline
	{
		// adjust for scroll registers
		loc = (((line - m_crtc.vbegin) + yscr) & 0x3ff) * 64;
		loc += (xscr / 16) & 0x7f;
		loc &= 0xffff;
		bit = 15 - (xscr & 0x0f);
		for(pixel=rect.min_x;pixel<=rect.max_x;pixel++)  // per pixel
		{
			colour = (((m_tvram[loc] >> bit) & 0x01) ? 1 : 0)
				+ (((m_tvram[loc+0x10000] >> bit) & 0x01) ? 2 : 0)
				+ (((m_tvram[loc+0x20000] >> bit) & 0x01) ? 4 : 0)
				+ (((m_tvram[loc+0x30000] >> bit) & 0x01) ? 8 : 0);
			// Colour 0 is displayable if the text layer is at the priority level 2
			if((m_pcgpalette->pen(colour) & 0xffffff) || ((m_video.reg[1] & 0x0c00) == 0x0800))
				bitmap.pix32(line, pixel) = m_pcgpalette->pen(colour);
			bit--;
			if(bit < 0)
			{
				bit = 15;
				loc++;
				loc &= 0xffff;
			}
		}
	}
}

bool x68k_state::x68k_draw_gfx_scanline( bitmap_ind16 &bitmap, rectangle cliprect, UINT8 priority)
{
	int pixel;
	int page;
	UINT32 loc;  // location in GVRAM
	UINT32 lineoffset;
	UINT16 xscr,yscr;
	UINT16 colour = 0;
	int shift;
	int scanline;
	bool blend, ret = false;
	UINT16 *pal = (UINT16 *)m_gfxpalette->basemem().base();

	for(scanline=cliprect.min_y;scanline<=cliprect.max_y;scanline++)  // per scanline
	{
		if(m_crtc.reg[20] & 0x0400)  // 1024x1024 "real" screen size - use 1024x1024 16-colour gfx layer
		{
			// adjust for scroll registers
			if(m_video.reg[2] & 0x0010 && priority == m_video.gfxlayer_pri[0])
			{
				xscr = (m_crtc.reg[12] & 0x3ff);
				yscr = (m_crtc.reg[13] & 0x3ff);
				lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x3ff) * 1024;
				loc = xscr & 0x3ff;
				for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
				{
					switch(lineoffset & 0xc0000)
					{
					case 0x00000:
						colour = m_gvram[lineoffset + (loc & 0x3ff)] & 0x000f;
						break;
					case 0x40000:
						colour = (m_gvram[(lineoffset - 0x40000) + (loc & 0x3ff)] & 0x00f0) >> 4;
						break;
					case 0x80000:
						colour = (m_gvram[(lineoffset - 0x80000) + (loc & 0x3ff)] & 0x0f00) >> 8;
						break;
					case 0xc0000:
						colour = (m_gvram[(lineoffset - 0xc0000) + (loc & 0x3ff)] & 0xf000) >> 12;
						break;
					}
					if(colour || (priority == 3))
						bitmap.pix16(scanline, pixel) = colour;
					loc++;
					loc &= 0x3ff;
				}
			}
		}
		else  // else 512x512 "real" screen size
		{
			if(m_video.reg[2] & (1 << priority))
			{
				page = m_video.gfxlayer_pri[priority];
				// adjust for scroll registers
				switch(m_video.reg[0] & 0x03)
				{
				case 0x00: // 16 colours
					xscr = ((m_crtc.reg[12+(page*2)])) & 0x1ff;
					yscr = ((m_crtc.reg[13+(page*2)])) & 0x1ff;
					lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					shift = 4;
					if((m_video.reg[2] & 0x1a00) == 0x1a00)
						ret = true;
					for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
					{
						colour = ((m_gvram[lineoffset + loc] >> page*shift) & 0x000f);
						if(ret && (colour & 1))
						{
							blend = true;
							colour &= 0xfe;
						}
						else
							blend = false;
						if(colour || (priority == 3))
						{
							if(ret)
							{
								if(blend && bitmap.pix16(scanline, pixel))
									bitmap.pix16(scanline, pixel) = ((bitmap.pix16(scanline, pixel) >> 1) & 0x7bde) + ((pal[colour] >> 1) & 0x7bde) + 1;
								else
									bitmap.pix16(scanline, pixel) = (pal[colour] & 0xfffe) + blend;
							}
							else
								bitmap.pix16(scanline, pixel) = colour;
						}
						loc++;
						loc &= 0x1ff;
					}
					break;
				case 0x01: // 256 colours
					if(page == 0 || page == 2)
					{
						xscr = ((m_crtc.reg[12+(page*2)])) & 0x1ff;
						yscr = ((m_crtc.reg[13+(page*2)])) & 0x1ff;
						lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
						loc = xscr & 0x1ff;
						shift = 4;
						if((m_video.reg[2] & 0x1a00) == 0x1a00)
							ret = true;
						for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
						{
							colour = ((m_gvram[lineoffset + loc] >> page*shift) & 0x00ff);
							if(ret && (colour & 1))
							{
								blend = true;
								colour &= 0xfe;
							}
							else
								blend = false;
							if(colour || (priority == 3))
							{
								if(ret)
								{
									if(blend && bitmap.pix16(scanline, pixel))
										bitmap.pix16(scanline, pixel) = ((bitmap.pix16(scanline, pixel) >> 1) & 0x7bde) + ((pal[colour] >> 1) & 0x7bde) + 1;
									else
										bitmap.pix16(scanline, pixel) = (pal[colour] & 0xfffe) + blend;
								}
								else
									bitmap.pix16(scanline, pixel) = colour;
							}
							loc++;
							loc &= 0x1ff;
						}
					}
					break;
				case 0x03: // 65536 colours
					xscr = ((m_crtc.reg[12])) & 0x1ff;
					yscr = ((m_crtc.reg[13])) & 0x1ff;
					lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
					{
						colour = m_gvram[lineoffset + loc];
						if(colour || (priority == 3))
							bitmap.pix16(scanline, pixel) = colour;
						loc++;
						loc &= 0x1ff;
					}
					break;
				}
			}
		}
	}
	return ret;
}

void x68k_state::x68k_draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect)
{
	int priority, scanline, pixel;
	bool gfxblend=false;
	//rectangle rect;
	//int xscr,yscr;
	//int gpage;

	if(m_crtc.reg[20] & 0x0800)  // if graphic layers are set to buffer, then they aren't visible
		return;

	m_gfxbitmap->fill(0, cliprect);

	for(priority=3;priority>=0;priority--)
	{
		gfxblend = x68k_draw_gfx_scanline(*m_gfxbitmap,cliprect,priority);
	}

	for(scanline=cliprect.min_y;scanline<=cliprect.max_y;scanline++)
	{
		UINT16 colour;
		bool blend = false;
		for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
		{
			if((m_video.reg[0] & 0x03) == 3)
			{
				colour = m_gfxbitmap->pix16(scanline, pixel);
				if(colour || (m_video.gfx_pri == 2))
					bitmap.pix32(scanline, pixel) = GGGGGRRRRRBBBBBI_decoder(colour);
			}
			else if(gfxblend)
			{
				colour = m_gfxbitmap->pix16(scanline, pixel);
				if(((m_video.reg[2] & 0x1900) == 0x1900) && (m_video.gfx_pri != 2) && (colour & 1))
					blend = true;
				else
					blend = false;
				if(colour || (m_video.gfx_pri == 2))
				{
					if(blend)
						bitmap.pix32(scanline, pixel) = ((bitmap.pix32(scanline, pixel) >> 1) & 0xff7f7f7f) + ((pal555(colour, 6, 11, 1) >> 1) & 0x7f7f7f);
					else
						bitmap.pix32(scanline, pixel) = pal555(colour, 6, 11, 1);
				}
			}
			else
			{
				colour = m_gfxbitmap->pix16(scanline, pixel) & 0xff;
				if(((m_video.reg[2] & 0x1900) == 0x1900) && (m_video.gfx_pri != 2) && (colour & 1))
				{
					blend = true;
					colour &= 0xfe;
				}
				else
					blend = false;
				if((colour && (m_gfxpalette->pen(colour) & 0xffffff)) || (m_video.gfx_pri == 2))
				{
					if(blend)
						bitmap.pix32(scanline, pixel) = ((bitmap.pix32(scanline, pixel) >> 1) & 0xff7f7f7f) + ((m_gfxpalette->pen(colour) >> 1) & 0x7f7f7f);
					else
						bitmap.pix32(scanline, pixel) = m_gfxpalette->pen(colour);
				}
			}
		}
	}
}

// Sprite controller "Cynthia" at 0xeb0000
void x68k_state::x68k_draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect)
{
	/*
	   0xeb0000 - 0xeb07ff - Sprite registers (up to 128)
	       + 00 : b9-0,  Sprite X position
	       + 02 : b9-0,  Sprite Y position
	       + 04 : b15,   Vertical Reversing (flipping?)
	              b14,   Horizontal Reversing
	              b11-8, Sprite colour
	              b7-0,  Sprite tile code (in PCG)
	       + 06 : b1-0,  Priority
	                     00 = Sprite not displayed

	   0xeb0800 - BG0 X Scroll  (10-bit)
	   0xeb0802 - BG0 Y Scroll
	   0xeb0804 - BG1 X Scroll
	   0xeb0806 - BG1 Y Scroll
	   0xeb0808 - BG control
	              b9,    BG/Sprite display (RAM and register access is faster if 1)
	              b4,    PCG area 1 available
	              b3,    BG1 display enable
	              b1,    PCG area 0 available
	              b0,    BG0 display enable
	   0xeb080a - Horizontal total (like CRTC reg 0 - is 0xff if in 256x256?)
	   0xeb080c - Horizontal display position (like CRTC reg 2 - +4)
	   0xeb080e - Vertical display position (like CRTC reg 6)
	   0xeb0810 - Resolution setting
	              b4,    "L/H" (apparently 15kHz/31kHz switch for sprites/BG?)
	              b3-2,  V-Res
	              b1-0,  H-Res (0 = 8x8 tilemaps, 1 = 16x16 tilemaps, 2 or 3 = unknown)
	*/
	int ptr,pri;

	for(ptr=508;ptr>=0;ptr-=4)  // stepping through sprites
	{
		pri = m_spritereg[ptr+3] & 0x03;
#ifdef MAME_DEBUG
		if(!(machine().input().code_pressed(KEYCODE_I)))
#endif
		if(pri == priority)
		{  // if at the right priority level, draw the sprite
			rectangle rect;
			int code = m_spritereg[ptr+2] & 0x00ff;
			int colour = (m_spritereg[ptr+2] & 0x0f00) >> 8;
			int xflip = m_spritereg[ptr+2] & 0x4000;
			int yflip = m_spritereg[ptr+2] & 0x8000;
			int sx = (m_spritereg[ptr+0] & 0x3ff) - 16;
			int sy = (m_spritereg[ptr+1] & 0x3ff) - 16;

			rect.min_x=m_crtc.hshift;
			rect.min_y=m_crtc.vshift;
			rect.max_x=rect.min_x + m_crtc.visible_width-1;
			rect.max_y=rect.min_y + m_crtc.visible_height-1;

			sx += m_crtc.bg_hshift;
			sx += m_sprite_shift;

			m_gfxdecode->gfx(1)->zoom_transpen(bitmap,cliprect,code,colour,xflip,yflip,m_crtc.hbegin+sx,m_crtc.vbegin+(sy*m_crtc.bg_double),0x10000,0x10000*m_crtc.bg_double,0x00);
		}
	}
}

static const gfx_layout x68k_pcg_8 =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout x68k_pcg_16 =
{
	16,16,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20,8+64*8,12+64*8,64*8,4+64*8,24+64*8,28+64*8,16+64*8,20+64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg0_tile)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(0,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg1_tile)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(0,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg0_tile_16)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(1,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg1_tile_16)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(1,code,colour,flags);
}

VIDEO_START_MEMBER(x68k_state,x68000)
{
	int gfx_index;

	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(m_pcgpalette, x68k_pcg_8, memregion("user1")->base(), 0, 32, 0)));

	gfx_index++;

	m_gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(m_pcgpalette, x68k_pcg_16, memregion("user1")->base(), 0, 32, 0)));
	m_gfxdecode->gfx(gfx_index)->set_colors(32);

	/* Tilemaps */
	m_bg0_8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg0_tile),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg1_8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg1_tile),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg0_16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg0_tile_16),this),TILEMAP_SCAN_ROWS,16,16,64,64);
	m_bg1_16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg1_tile_16),this),TILEMAP_SCAN_ROWS,16,16,64,64);

	m_bg0_8->set_transparent_pen(0);
	m_bg1_8->set_transparent_pen(0);
	m_bg0_16->set_transparent_pen(0);
	m_bg1_16->set_transparent_pen(0);

	m_pcgbitmap = auto_bitmap_ind16_alloc(machine(), 1024, 1024);
	m_pcgbitmap->fill(0);

	m_gfxbitmap = auto_bitmap_ind16_alloc(machine(), 1024, 1024);
	m_gfxbitmap->fill(0);

//  m_scanline_timer->adjust(attotime::zero, 0, attotime::from_hz(55.45)/568);
}

UINT32 x68k_state::screen_update_x68000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle rect(0,0,0,0);
	int priority;
	int xscr,yscr;
	int x;
	tilemap_t* x68k_bg0;
	tilemap_t* x68k_bg1;
	int pixel, scanline;
	//UINT8 *rom;

	if((m_spritereg[0x408] & 0x03) == 0x00)  // Sprite/BG H-Res 0=8x8, 1=16x16, 2 or 3 = undefined.
	{
		x68k_bg0 = m_bg0_8;
		x68k_bg1 = m_bg1_8;
	}
	else
	{
		x68k_bg0 = m_bg0_16;
		x68k_bg1 = m_bg1_16;
	}
//  rect.max_x=m_crtc.width;
//  rect.max_y=m_crtc.height;
	bitmap.fill(0, cliprect);

	if(m_sysport.contrast == 0)  // if monitor contrast is 0, then don't bother displaying anything
		return 0;

	rect.min_x=m_crtc.hbegin;
	rect.min_y=m_crtc.vbegin;
//  rect.max_x=rect.min_x + m_crtc.visible_width-1;
//  rect.max_y=rect.min_y + m_crtc.visible_height-1;
	rect.max_x=m_crtc.hend;
	rect.max_y=m_crtc.vend;

	if(rect.min_y < cliprect.min_y)
		rect.min_y = cliprect.min_y;
	if(rect.max_y > cliprect.max_y)
		rect.max_y = cliprect.max_y;

	// update tiles
	//rom = memregion("user1")->base();
	for(x=0;x<256;x++)
	{
		if(m_video.tile16_dirty[x] != 0)
		{
			m_gfxdecode->gfx(1)->mark_dirty(x);
			m_video.tile16_dirty[x] = 0;
		}
		if(m_video.tile8_dirty[x] != 0)
		{
			m_gfxdecode->gfx(0)->mark_dirty(x);
			m_video.tile8_dirty[x] = 0;
		}
	}


	for(priority=2;priority>=0;priority--)
	{
		// Graphics screen(s)
		if(priority == m_video.gfx_pri)
			x68k_draw_gfx(bitmap,rect);

		// Sprite / BG Tiles
		if(priority == m_video.sprite_pri /*&& (m_spritereg[0x404] & 0x0200)*/ && (m_video.reg[2] & 0x0040))
		{
			m_pcgbitmap->fill(0, rect);
			x68k_draw_sprites(*m_pcgbitmap,1,rect);
			if((m_spritereg[0x404] & 0x0008))
			{
				if((m_spritereg[0x404] & 0x0030) == 0x10)  // BG1 TXSEL
				{
					x68k_bg0->set_scrollx(0,(m_spritereg[0x402] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg0->set_scrolly(0,(m_spritereg[0x403] - m_crtc.vbegin) & 0x3ff);
					x68k_bg0->draw(screen, *m_pcgbitmap,rect,0,0);
				}
				else
				{
					x68k_bg1->set_scrollx(0,(m_spritereg[0x402] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg1->set_scrolly(0,(m_spritereg[0x403] - m_crtc.vbegin) & 0x3ff);
					x68k_bg1->draw(screen, *m_pcgbitmap,rect,0,0);
				}
			}
			x68k_draw_sprites(*m_pcgbitmap,2,rect);
			if((m_spritereg[0x404] & 0x0001))
			{
				if((m_spritereg[0x404] & 0x0006) == 0x02)  // BG0 TXSEL
				{
					x68k_bg0->set_scrollx(0,(m_spritereg[0x400] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg0->set_scrolly(0,(m_spritereg[0x401] - m_crtc.vbegin) & 0x3ff);
					x68k_bg0->draw(screen, *m_pcgbitmap,rect,0,0);
				}
				else
				{
					x68k_bg1->set_scrollx(0,(m_spritereg[0x400] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg1->set_scrolly(0,(m_spritereg[0x401] - m_crtc.vbegin) & 0x3ff);
					x68k_bg1->draw(screen, *m_pcgbitmap,rect,0,0);
				}
			}
			x68k_draw_sprites(*m_pcgbitmap,3,rect);

			for(scanline=rect.min_y;scanline<=rect.max_y;scanline++)
			{
				for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
				{
					UINT8 colour = m_pcgbitmap->pix16(scanline, pixel) & 0xff;
					if((colour && (m_pcgpalette->pen(colour) & 0xffffff)) || ((m_video.reg[1] & 0x3000) == 0x2000))
						bitmap.pix32(scanline, pixel) = m_pcgpalette->pen(colour);
				}
			}
		}

		// Text screen
		if(m_video.reg[2] & 0x0020 && priority == m_video.text_pri)
		{
			xscr = (m_crtc.reg[10] & 0x3ff);
			yscr = (m_crtc.reg[11] & 0x3ff);
			if(!(m_crtc.reg[20] & 0x1000))  // if text layer is set to buffer, then it's not visible
				x68k_draw_text(bitmap,xscr,yscr,rect);
		}
	}

#ifdef MAME_DEBUG
	if(machine().input().code_pressed(KEYCODE_9))
	{
		m_sprite_shift--;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}
	if(machine().input().code_pressed(KEYCODE_0))
	{
		m_sprite_shift++;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}

#endif

#ifdef MAME_DEBUG
//  popmessage("Layer priorities [%04x] - Txt: %i  Spr: %i  Gfx: %i  Layer Pri0-3: %i %i %i %i",m_video.reg[1],m_video.text_pri,m_video.sprite_pri,
//      m_video.gfx_pri,m_video.gfxlayer_pri[0],m_video.gfxlayer_pri[1],m_video.gfxlayer_pri[2],m_video.gfxlayer_pri[3]);
//  popmessage("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i",m_crtc.reg[0],m_crtc.reg[1],m_crtc.reg[2],m_crtc.reg[3],
//      m_crtc.reg[4],m_crtc.reg[5],m_crtc.reg[6],m_crtc.reg[7],m_crtc.reg[8],m_crtc.reg[9]);
//  popmessage("Visible resolution = %ix%i (%s) Screen size = %ix%i",m_crtc.visible_width,m_crtc.visible_height,m_crtc.interlace ? "Interlaced" : "Non-interlaced",m_crtc.video_width,m_crtc.video_height);
//  popmessage("VBlank : scanline = %i",m_scanline);
//  popmessage("CRTC/BG compare H-TOTAL %i/%i H-DISP %i/%i V-DISP %i/%i BG Res %02x",m_crtc.reg[0],m_spritereg[0x405],m_crtc.reg[2],m_spritereg[0x406],
//      m_crtc.reg[6],m_spritereg[0x407],m_spritereg[0x408]);
//  popmessage("BG Scroll - BG0 X %i Y %i  BG1 X %i Y %i",m_spriteram[0x400],m_spriteram[0x401],m_spriteram[0x402],m_spriteram[0x403]);
//  popmessage("uPD72065 status = %02x",upd765_status_r(machine(), space, 0));
//  popmessage("Layer enable - 0x%02x",m_video.reg[2] & 0xff);
//  popmessage("Graphic layer scroll - %i, %i - %i, %i - %i, %i - %i, %i",
//      m_crtc.reg[12],m_crtc.reg[13],m_crtc.reg[14],m_crtc.reg[15],m_crtc.reg[16],m_crtc.reg[17],m_crtc.reg[18],m_crtc.reg[19]);
//  popmessage("IOC IRQ status - %02x",m_ioc.irqstatus);
//  popmessage("RAM: mouse data - %02x %02x %02x %02x",m_ram->pointer()[0x931],m_ram->pointer()[0x930],m_ram->pointer()[0x933],m_ram->pointer()[0x932]);
#endif
	return 0;
}
