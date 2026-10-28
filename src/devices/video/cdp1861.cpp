// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1861 Video Display Controller emulation

Timing: The 1861 interrupts the CPU to signal that DMA will be starting
in exactly 29 cycles. The CPU must set R0 as the pointer to the first of
8 sequential bytes to be transferred. After the DMA, exactly 6 cycles will
elapse before the next DMA. This process continues until the 1861 is within
4 lines of the end of the visible area, when it will assert EFx. When the
CPU sees this, it can finish up the interrupt routine. The 1861 will clear
EFx at the last visible line. The original IRQ request is cleared 28 cycles
after it was asserted. EFx is also asserted 4 lines before the first visible
scanline and ends 4 lines later, but this is usually ignored.

Timing as it applies to the Studio II:
- The usage of EFx before the visible area is not used.
- R1 is preset with the value 001C, the interrupt vector.
- When the interrupt from the 1861 occurs, R1 becomes the P register, so
  causing a jump to 001C.
- This is followed by 13 2-cycle instructions and one 3-cycle instruction,
  giving us the required 29 cycles.
- The first DMA therefore will occur just after the PLO at 002D.
- This is followed by 3 2-cycle instructions, giving the required 6 cycles.
- The 1861 will draw 128 scanlines, but due to memory constraints, the
  Studio II can only do 32 lines, and so each group of 4 scanlines is
  DMA'd from the same part of memory.
- Each DMA will automatically add 8 to R0, and so this needs to be reset
  for each of the 4 lines. After this, the new R0 value can be used for
  the next group of 4 scanlines.
- After the 4 scanlines are done, EF1 is checked to see if the bottom of
  the display is being reached. If not, more lines can be processed.
- At the end, the random number seed (not part of video drawing) gets
  updated and the interrupt routine ends.

**********************************************************************/

#include "emu.h"
#include "cdp1861.h"

#include "screen.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CDP1861, cdp1861_device, "cdp1861", "RCA CDP1861")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1861_device - constructor
//-------------------------------------------------

cdp1861_device::cdp1861_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDP1861, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_write_int(*this)
	, m_write_dma_out(*this)
	, m_write_efx(*this)
	, m_disp(0)
	, m_dispon(0), m_dispoff(0)
	, m_dmaout(CLEAR_LINE)
	, m_int(CLEAR_LINE)
	, m_efx(CLEAR_LINE)
	, m_tpa(0)
	, m_sc(0)
	, m_line(0), m_cycle(0)
	, m_next_line(0), m_next_cycle(0)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1861_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().has_been_setup())
		screen().set_raw(clock(), SCREEN_WIDTH, HBLANK_END, HBLANK_START, TOTAL_SCANLINES, SCANLINE_VBLANK_END, SCANLINE_VBLANK_START);

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(cdp1861_device::screen_update));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1861_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);

	// register for state saving
	save_item(NAME(m_disp));
	save_item(NAME(m_dispon));
	save_item(NAME(m_dispoff));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_int));
	save_item(NAME(m_efx));
	save_item(NAME(m_tpa));
	save_item(NAME(m_sc));
	save_item(NAME(m_line));
	save_item(NAME(m_cycle));
	save_item(NAME(m_next_line));
	save_item(NAME(m_next_cycle));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1861_device::device_reset()
{
	m_line = m_next_line = 0;
	m_cycle = m_next_cycle = 0;

	m_disp = 0;
	m_dispon = 0;

	set_int(CLEAR_LINE);
	set_dma_out(CLEAR_LINE);
	set_efx(CLEAR_LINE);
}


//-------------------------------------------------
//  output lines
//-------------------------------------------------

void cdp1861_device::set_int(int state)
{
	m_int = state;
	m_write_int(state);
}

void cdp1861_device::set_dma_out(int state)
{
	m_dmaout = state;
	m_write_dma_out(state);
}

void cdp1861_device::set_efx(int state)
{
	m_efx = state;
	m_write_efx(state);
}


//-------------------------------------------------
//  tpa_w - clock the horizontal line counter
//-------------------------------------------------

void cdp1861_device::tpa_w(int state)
{
	if (state && !m_tpa)
	{
		m_line = m_next_line;
		m_cycle = m_next_cycle;

		if (m_cycle == 0)
		{
			switch (m_line)
			{
			case 0:
				screen().reset_origin(0, 0);
				break;

			case SCANLINE_EFX_TOP_START:
			case SCANLINE_EFX_BOTTOM_START:
				set_efx(ASSERT_LINE);
				break;

			case SCANLINE_EFX_TOP_END:
			case SCANLINE_EFX_BOTTOM_END:
				set_efx(CLEAR_LINE);
				break;
			}

			if (m_line == SCANLINE_INT_START && m_disp)
				set_int(ASSERT_LINE);

			if (m_line == SCANLINE_INT_END)
				set_int(CLEAR_LINE);
		}

		if (m_line >= SCANLINE_DISPLAY_START && m_line < SCANLINE_DISPLAY_END)
		{
			if (m_cycle == CYCLE_DMA_START && m_disp)
				set_dma_out(ASSERT_LINE);
			else if (m_cycle == CYCLE_DMA_END && m_dmaout)
				set_dma_out(CLEAR_LINE);
		}

		// a line is one cycle short unless the CPU is in S1 at horizontal sync (the CDP1861 datasheet
		// swaps SC0 and SC1 here, the CDP1864 datasheet has it right)
		m_next_cycle = m_cycle + ((m_cycle == 0 && m_sc != 1) ? 2 : 1);

		if (m_next_cycle >= CYCLES_PER_LINE)
		{
			m_next_cycle = 0;
			m_next_line = (m_line + 1) % TOTAL_SCANLINES;
		}
	}

	m_tpa = state;
}


//-------------------------------------------------
//  sc_w - state code input
//-------------------------------------------------

void cdp1861_device::sc_w(uint8_t data)
{
	m_sc = data & 0x03;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void cdp1861_device::dma_w(uint8_t data)
{
	int sx = m_cycle * 8 + 4;

	for (int x = 0; x < 8; x++)
	{
		if (m_bitmap.cliprect().contains(sx + x, m_line))
			m_bitmap.pix(m_line, sx + x) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
		data <<= 1;
	}
}


//-------------------------------------------------
//  disp_on_w -
//-------------------------------------------------

void cdp1861_device::disp_on_w(int state)
{
	if (!m_dispon && state) m_disp = 1;

	m_dispon = state;
}


//-------------------------------------------------
//  disp_off_w -
//-------------------------------------------------

void cdp1861_device::disp_off_w(int state)
{
	if (!m_dispon && !m_dispoff && state) m_disp = 0;

	m_dispoff = state;

	set_int(CLEAR_LINE);
	set_dma_out(CLEAR_LINE);
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t cdp1861_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_disp)
	{
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}
	return 0;
}
