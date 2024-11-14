// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl

#include "emu.h"
#include "x68k_crtc.h"
#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(VINAS, vinas_device, "vinas", "IX0902/IX0903 VINAS CRTC")
DEFINE_DEVICE_TYPE(VICON, vicon_device, "vicon", "IX1093 VICON CRTC")

x68k_crtc_device::x68k_crtc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_vdisp_callback(*this)
	, m_rint_callback(*this)
	, m_hsync_callback(*this)
	, m_tvram_read_callback(*this, 0)
	, m_gvram_read_callback(*this, 0)
	, m_tvram_write_callback(*this)
	, m_gvram_write_callback(*this)
	, m_clock_69m(0)
	, m_clock_50m(0)
	, m_operation(0)
	, m_vblank(false)
	, m_hblank(false)
	, m_htotal(0)
	, m_vtotal(0)
	, m_hend(0)
	, m_vend(0)
	, m_hsync_end(0)
	, m_vsync_end(0)
	, m_hsyncadjust(0)
	, m_vmultiple(1)
	, m_height(0)
	, m_width(0)
	, m_visible_height(0)
	, m_visible_width(0)
	, m_interlace(false)
	, m_oddscanline(false)
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
}

vinas_device::vinas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: x68k_crtc_device(mconfig, VINAS, tag, owner, clock)
{
}

vicon_device::vicon_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: x68k_crtc_device(mconfig, VICON, tag, owner, clock)
{
}

void x68k_crtc_device::device_start()
{
	m_scanline_timer = timer_alloc(FUNC(x68k_crtc_device::hsync), this);
	m_operation_end_timer = timer_alloc(FUNC(x68k_crtc_device::operation_end), this);
	m_raster_end_timer = timer_alloc(FUNC(x68k_crtc_device::raster_end), this);
	m_raster_irq_timer = timer_alloc(FUNC(x68k_crtc_device::raster_irq), this);
	m_vblank_irq_timer = timer_alloc(FUNC(x68k_crtc_device::vblank_irq), this);

	// save state
	save_item(NAME(m_reg));
	save_item(NAME(m_operation));
	save_item(NAME(m_vblank));
	save_item(NAME(m_hblank));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_hend));
	save_item(NAME(m_vend));
	save_item(NAME(m_hsync_end));
	save_item(NAME(m_vsync_end));
	save_item(NAME(m_hsyncadjust));
	save_item(NAME(m_vmultiple));
	save_item(NAME(m_height));
	save_item(NAME(m_width));
	save_item(NAME(m_visible_height));
	save_item(NAME(m_visible_width));
	save_item(NAME(m_interlace));
	save_item(NAME(m_oddscanline));
}

void x68k_crtc_device::device_reset()
{
	// initialise CRTC, set registers to defaults for the standard text mode (768x512)
	m_reg[0] = 137;  // Horizontal total  (in characters)
	m_reg[1] = 14;   // Horizontal sync end
	m_reg[2] = 28;   // Horizontal start
	m_reg[3] = 124;  // Horizontal end
	m_reg[4] = 567;  // Vertical total
	m_reg[5] = 5;    // Vertical sync end
	m_reg[6] = 40;   // Vertical start
	m_reg[7] = 552;  // Vertical end
	m_reg[8] = 27;   // Horizontal adjust

	//m_scanline = screen().vpos();// = m_reg[6];  // Vertical start

	// start VBlank timer
	m_vblank = true;
	attotime const irq_time = screen().time_until_pos(m_reg[6],2);
	m_vblank_irq_timer->adjust(irq_time);

	// start HBlank timer
	m_scanline_timer->adjust(screen().scan_period(), 1);

	m_vdisp_callback(1);
	m_rint_callback(1);
	m_hsync_callback(1);
}

void x68k_crtc_device::text_copy(unsigned src, unsigned dest, u8 planes)
{
	// copys one raster in T-VRAM to another raster
	offs_t src_ram = src * 256;  // 128 bytes per scanline
	offs_t dest_ram = dest * 256;

	// update RAM in each plane
	for (int words = 256; words > 0; words--, src_ram++, dest_ram++)
	{
		for (u8 plane = 0; plane <= 3; plane++)
			if (BIT(planes, plane))
				m_tvram_write_callback(dest_ram + 0x10000 * plane, m_tvram_read_callback(src_ram + 0x10000 * plane, 0xffff), 0xffff);
	}
}

TIMER_CALLBACK_MEMBER(x68k_crtc_device::operation_end)
{
	if(!(m_operation & param))
	{
		m_operation |= param;
		m_operation_end_timer->adjust(attotime::from_msec(5), param);
	}
	else
		m_operation &= ~param;
}

void x68k_crtc_device::refresh_mode()
{
	// Calculate data from register values
	m_vmultiple = 1;
	if (m_interlace)
		m_vmultiple = 0.5f;  // 31.5kHz + 1024 lines or 15kHz + 512 lines = interlaced
	m_htotal = (m_reg[0] + 1) * 8;
	m_vtotal = (m_reg[4] + 1) / m_vmultiple; // default is 567 (568 scanlines)
	m_hbegin = (m_reg[2] * 8) + 1;
	m_hend = m_reg[3] * 8;
	m_vbegin = (m_reg[6] + 1) / m_vmultiple;
	m_vend = m_reg[7] / m_vmultiple;
	m_hsync_end = (m_reg[1]) * 8;
	m_vsync_end = (m_reg[5]) / m_vmultiple;
	m_hsyncadjust = m_reg[8];

	rectangle scr(0, m_htotal - 8, 0, m_vtotal);
	if (scr.max_y <= m_vend)
		scr.max_y = m_vend + 2;
	if (scr.max_x <= m_hend)
		scr.max_x = m_hend + 2;
	rectangle visiblescr(m_hbegin, m_hend, m_vbegin, m_vend);

	if ((visiblescr.max_y > m_height) || (visiblescr.max_x > m_width))
		logerror("visarea larger then reg[20]: %dx%d, %dx%d\n", visiblescr.max_x, visiblescr.max_y, m_width, m_height);

	// bounds check
	if (visiblescr.min_x < 0)
		visiblescr.min_x = 0;
	if (visiblescr.min_y < 0)
		visiblescr.min_y = 0;
	if (visiblescr.max_x >= scr.max_x)
		visiblescr.max_x = scr.max_x - 2;
	if (visiblescr.max_y >= scr.max_y - 1)
		visiblescr.max_y = scr.max_y - 2;

//  LOG("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i\n", m_reg[0], m_reg[1], m_reg[2], m_reg[3],
//      m_reg[4], m_reg[5], m_reg[6], m_reg[7], m_reg[8], m_reg[9]);
	double div;
	switch (m_reg[20] & 0x1f)
	{
		case 0:
			div = 8;
			break;
		default:
			logerror("Invalid mode %d", m_reg[20] & 0x1f); [[fallthrough]];
		case 1:
		case 5:
		case 0x11:
			div = 4;
			break;
		case 0x16:
			div = 2;
			break;
		case 0x10:
			div = 6;
			break;
		case 0x15:
			div = 3;
			break;
		case 0x19:
			div = 1.5;
			break;
	}
	attotime refresh = attotime::from_hz((BIT(m_reg[20], 4) ? clock_69m() : clock_39m()) / div) * (scr.max_x * scr.max_y);
	LOG("screen().configure(%i,%i,[%i,%i,%i,%i],%f)\n", scr.max_x, scr.max_y, visiblescr.min_x, visiblescr.min_y, visiblescr.max_x, visiblescr.max_y, refresh.as_hz());
	screen().configure(scr.max_x, scr.max_y, visiblescr, refresh.as_attoseconds());
}

TIMER_CALLBACK_MEMBER(x68k_crtc_device::hsync)
{
	int hstate = param;
	attotime hsync_time;

	m_hblank = hstate;
	m_hsync_callback(!m_hblank);

	if (m_operation & 8)
		text_copy((m_reg[22] & 0xff00) >> 8, (m_reg[22] & 0x00ff), (m_reg[21] & 0xf));

	int scan = screen().vpos();
	if (hstate == 1)
	{
		hsync_time = screen().time_until_pos(scan, m_hend);
		m_scanline_timer->adjust(hsync_time);
		if ((scan != 0) && (scan < m_vend))
			screen().update_partial(scan - 1);
	}
	if (hstate == 0)
	{
		if (scan == (m_vtotal - 1))
			scan = 0;
		else
			scan++;
		hsync_time = screen().time_until_pos(scan, m_hbegin);
		m_scanline_timer->adjust(hsync_time, 1);
	}
}

TIMER_CALLBACK_MEMBER(x68k_crtc_device::raster_end)
{
	m_rint_callback(1);
}

TIMER_CALLBACK_MEMBER(x68k_crtc_device::raster_irq)
{
	int scan = param;
	attotime irq_time;
	attotime end_time;

	if (scan <= m_vtotal)
	{
		m_rint_callback(0);
		screen().update_partial(scan - 1);
		irq_time = screen().time_until_pos(scan, m_hbegin);
		// end of HBlank period clears GPIP6 also?
		end_time = screen().time_until_pos(scan, m_hend);
		m_raster_irq_timer->adjust(irq_time, scan);
		m_raster_end_timer->adjust(end_time);
		LOG("GPIP6: Raster triggered at line %i (%i)\n", scan,screen().vpos());
	}
}

TIMER_CALLBACK_MEMBER(x68k_crtc_device::vblank_irq)
{
	int val = param;
	attotime irq_time;
	int vblank_line;

	if (val == 1)  // V-DISP on
	{
		m_vblank = 1;
		vblank_line = m_vbegin;
		irq_time = screen().time_until_pos(vblank_line, 2);
		m_vblank_irq_timer->adjust(irq_time);
		LOG("CRTC: VBlank on\n");
	}
	if (val == 0)  // V-DISP off
	{
		m_vblank = 0;
		vblank_line = m_vend + (gfx_double_scan() ? 2 : 1);
		if (vblank_line > m_vtotal)
			vblank_line = m_vtotal;
		irq_time = screen().time_until_pos(vblank_line, 2);
		m_vblank_irq_timer->adjust(irq_time, 1);
		LOG("CRTC: VBlank off\n");
	}

	m_vdisp_callback(!m_vblank);
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
void x68k_crtc_device::crtc_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset < 0x24)
		COMBINE_DATA(&m_reg[offset]);
	switch (offset)
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
		refresh_mode();
		break;
	case 9:  // CRTC raster IRQ (GPIP6)
		{
			data = m_reg[9];
			attotime irq_time = attotime::zero;
			if ((data / m_vmultiple) != screen().vpos())
			{
				irq_time = screen().time_until_pos(((data != 0 ? data : screen().height()) - 1) / m_vmultiple,2);
				m_rint_callback(1);
			}
			m_raster_irq_timer->adjust(irq_time, (data) / m_vmultiple);
			LOG("CRTC: Write to raster IRQ register - %i %i %f\n",data, screen().vpos(), irq_time.as_double());
		}
		break;
	case 20:
		if (ACCESSING_BITS_0_7)
		{
			m_interlace = false;
			switch (data & 0x0c)
			{
			case 0x00:
				m_height = 256;
				break;
			case 0x08:
			case 0x0c:  // TODO: 1024 vertical, if horizontal freq = 31kHz
				m_height = 512;
				m_interlace = true;  // if 31kHz, 1024 lines = interlaced
				break;
			case 0x04:
				m_height = 512;
				if (!(m_reg[20] & 0x0010))  // if 15kHz, 512 lines = interlaced
					m_interlace = true;
				break;
			}
			switch (data & 0x03)
			{
			case 0x00:
				m_width = 256;
				break;
			case 0x01:
				m_width = 512;
				break;
			case 0x02:
			case 0x03:  // 0x03 = 50MHz clock mode (XVI only)
				m_width = 768;
				break;
			}
		}
/*      if (ACCESSING_BITS_8_15)
        {
            m_interlace = false;
            if (data & 0x0400)
                m_interlace = true;
        }*/
		logerror("CRTC: Register 20 = %04x\n", m_reg[20]);
		refresh_mode();
		break;
	case 576:  // operation register
		m_operation = data & ~2;
		if ((data & 0x02) && (m_reg[21] & 0xf))  // high-speed graphic screen clear
		{
			// this is based on the docs except for the higher color depth modes which isn't
			// explicitly described this way but is likely based on how the plane scroll works
			// XXX: not sufficiently tested especially in hires modes
			// it seems that it only uses the 0 page scroll registers for where to clear see atomrobo
			uint16_t xscr = xscr_gfx(0) & 0x1ff;
			uint16_t yscr = yscr_gfx(0) & 0x1ff;
			uint16_t mask = 0;
			for (int page = 0; page < 4; page++)
			{
				if (!(m_reg[21] & (1 << page)))
					mask |= (0xf << (page * 4));
			}
			for (int y = yscr; y < (m_height + yscr); y++)
			{
				if (is_1024x1024())
				{
					if (m_width > 256)
					{
						for (int x = 0; x < 512; x++)
						{
							uint16_t data = m_gvram_read_callback(((y * 512) + x) & 0x3ffff, 0xffff);
							m_gvram_write_callback(((y * 512) + x) & 0x3ffff, data & mask, 0xffff);
						}
					}
					else
					{
						for (int x = 0; x < 256; x++)
						{
							uint16_t data = m_gvram_read_callback(((y * 512) + x + xscr) & 0x3ffff, 0xffff);
							m_gvram_write_callback(((y * 512) + x + xscr) & 0x3ffff, data & mask, 0xffff);
							data = m_gvram_read_callback(((y * 512) + x + xscr + 256) & 0x3ffff, 0xffff);
							m_gvram_write_callback(((y * 512) + x + xscr + 256) & 0x3ffff, data & mask, mask);
						}
					}
				}
				else
				{
					for (int x = 0; x < m_width; x++)
					{
						uint16_t data = m_gvram_read_callback(((y * 512) + x + xscr) & 0x3ffff, 0xffff);
						m_gvram_write_callback(((y * 512) + x + xscr) & 0x3ffff, data & mask, 0xffff);
					}
				}
			}
		}
		if (data & 0x02) m_operation_end_timer->adjust(attotime::from_msec(5), 0x02);  // time taken to do operation is a complete guess.
		break;
	}
//  LOG("%s CRTC: Wrote %04x to CRTC register %i\n",machine().describe_context(), data, offset);
}

u16 x68k_crtc_device::crtc_r(offs_t offset)
{
	if (offset < 24)
	{
//      LOG("%s CRTC: Read %04x from CRTC register %i\n",machine().describe_context(), m_reg[offset], offset);
		switch (offset)
		{
		case 9:
			if (machine().side_effects_disabled())
				return m_reg[9];
			return 0;
		case 10:  // Text X/Y scroll
		case 11:
		case 12:  // Graphic layer 0 scroll
		case 13:
			return m_reg[offset] & 0x3ff;
		case 14:  // Graphic layer 1 scroll
		case 15:
		case 16:  // Graphic layer 2 scroll
		case 17:
		case 18:  // Graphic layer 3 scroll
		case 19:
			return m_reg[offset] & 0x1ff;
		default:
			return m_reg[offset];
		}
	}
	if (offset == 576) // operation port, operation bits are set to 0 when operation is complete
		return m_operation;
//  LOG("CRTC: [%08x] Read from unknown CRTC register %i\n",activecpu_get_pc(),offset);
	return 0xffff;
}

void x68k_crtc_device::gvram_w(offs_t offset, u16 data, u16 mem_mask)
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
	if (m_reg[20] & 0x0800)  // G-VRAM set to buffer
	{
		if (offset < 0x40000)
			m_gvram_write_callback(offset, data, mem_mask);
	}
	else
	{
		switch (m_reg[20] & 0x0300)
		{
			case 0x0300:
				if (offset < 0x40000)
					m_gvram_write_callback(offset, data, mem_mask);
				break;
			case 0x0100:
				if (offset < 0x40000)
				{
					m_gvram_write_callback(offset, data & 0x00ff, 0x00ff);
				}
				else if (offset >= 0x40000 && offset < 0x80000)
				{
					m_gvram_write_callback(offset - 0x40000, (data & 0x00ff) << 8, 0xff00);
				}
				break;
			case 0x0000:
				if (offset < 0x40000)
				{
					m_gvram_write_callback(offset, data & 0x000f, 0x000f);
				}
				else if (offset >= 0x40000 && offset < 0x80000)
				{
					m_gvram_write_callback(offset - 0x40000, (data & 0x000f) << 4, 0x00f0);
				}
				else if (offset >= 0x80000 && offset < 0xc0000)
				{
					m_gvram_write_callback(offset - 0x80000, (data & 0x000f) << 8, 0x0f00);
				}
				else if (offset >= 0xc0000 && offset < 0x100000)
				{
					m_gvram_write_callback(offset - 0xc0000, (data & 0x000f) << 12, 0xf000);
				}
				break;
			default:
				logerror("G-VRAM written while layer setup is undefined.\n");
		}
	}
}

void x68k_crtc_device::tvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 text_mask = ~(m_reg[23]) & mem_mask;

	if (!(m_reg[21] & 0x0200)) // text access mask enable
		text_mask = 0xffff & mem_mask;

	mem_mask = text_mask;

	if (m_reg[21] & 0x0100)
	{
		// simultaneous T-VRAM plane access (I think ;))
		offset &= 0x00ffff;
		u8 wr = (m_reg[21] & 0x00f0) >> 4;
		for (int plane = 0; plane < 4; plane++)
		{
			if (BIT(wr, plane))
			{
				m_tvram_write_callback(offset + 0x10000 * plane, data, mem_mask);
			}
		}
	}
	else
	{
		m_tvram_write_callback(offset, data, mem_mask);
	}
}

u16 x68k_crtc_device::gvram_r(offs_t offset)
{
	u16 ret = 0;

	if (m_reg[20] & 0x0800)  // G-VRAM set to buffer
		return m_gvram_read_callback(offset);

	switch (m_reg[20] & 0x0300)  // colour setup determines G-VRAM use
	{
		case 0x0300: // 65,536 colour (RGB) - 16-bits per word
			if (offset < 0x40000)
				ret = m_gvram_read_callback(offset);
			else
				ret = 0xffff;
			break;
		case 0x0100:  // 256 colour (paletted) - 8 bits per word
			if (offset < 0x40000)
				ret = m_gvram_read_callback(offset) & 0x00ff;
			else if (offset >= 0x40000 && offset < 0x80000)
				ret = (m_gvram_read_callback(offset - 0x40000) & 0xff00) >> 8;
			else if (offset >= 0x80000)
				ret = 0xffff;
			break;
		case 0x0000:  // 16 colour (paletted) - 4 bits per word
			if (offset < 0x40000)
				ret = m_gvram_read_callback(offset) & 0x000f;
			else if (offset >= 0x40000 && offset < 0x80000)
				ret = (m_gvram_read_callback(offset - 0x40000) & 0x00f0) >> 4;
			else if (offset >= 0x80000 && offset < 0xc0000)
				ret = (m_gvram_read_callback(offset - 0x80000) & 0x0f00) >> 8;
			else if (offset >= 0xc0000 && offset < 0x100000)
				ret = (m_gvram_read_callback(offset - 0xc0000) & 0xf000) >> 12;
			break;
		default:
			logerror("G-VRAM read while layer setup is undefined.\n");
			ret = 0xffff;
	}

	return ret;
}

u16 x68k_crtc_device::tvram_r(offs_t offset)
{
	return m_tvram_read_callback(offset);
}
