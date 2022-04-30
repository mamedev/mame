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
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1861_CYCLES_DMA_START    (2*8)
#define CDP1861_CYCLES_DMA_ACTIVE   (8*8)
#define CDP1861_CYCLES_DMA_WAIT     (6*8)



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
	, m_int_timer(nullptr), m_efx_timer(nullptr), m_dma_timer(nullptr)
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

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock(), SCREEN_WIDTH, HBLANK_END, HBLANK_START, TOTAL_SCANLINES, SCANLINE_VBLANK_END, SCANLINE_VBLANK_START);

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(cdp1861_device::screen_update));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1861_device::device_start()
{
	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_dma_out.resolve_safe();
	m_write_efx.resolve_safe();

	// allocate timers
	m_int_timer = timer_alloc(TIMER_INT);
	m_efx_timer = timer_alloc(TIMER_EFX);
	m_dma_timer = timer_alloc(TIMER_DMA);

	// find devices
	screen().register_screen_bitmap(m_bitmap);

	// register for state saving
	save_item(NAME(m_disp));
	save_item(NAME(m_dispon));
	save_item(NAME(m_dispoff));
	save_item(NAME(m_dmaout));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1861_device::device_reset()
{
	m_int_timer->adjust(screen().time_until_pos(SCANLINE_INT_START, 0));
	m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_TOP_START, 0));
	m_dma_timer->adjust(clocks_to_attotime(CDP1861_CYCLES_DMA_START));

	m_disp = 0;
	m_dmaout = 0;
	m_dispon = 0;

	m_write_int(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);
	m_write_efx(CLEAR_LINE);
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void cdp1861_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	int scanline = screen().vpos();

	switch (id)
	{
	case TIMER_INT:
		if (scanline == SCANLINE_INT_START)
		{
			if (m_disp)
			{
				m_write_int(ASSERT_LINE);
			}

			m_int_timer->adjust(screen().time_until_pos( SCANLINE_INT_END, 0));
		}
		else
		{
			if (m_disp)
			{
				m_write_int(CLEAR_LINE);
			}

			m_int_timer->adjust(screen().time_until_pos(SCANLINE_INT_START, 0));
		}
		break;

	case TIMER_EFX:
		switch (scanline)
		{
		case SCANLINE_EFX_TOP_START:
			m_write_efx(ASSERT_LINE);
			m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_TOP_END, 0));
			break;

		case SCANLINE_EFX_TOP_END:
			m_write_efx(CLEAR_LINE);
			m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_BOTTOM_START, 0));
			break;

		case SCANLINE_EFX_BOTTOM_START:
			m_write_efx(ASSERT_LINE);
			m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_BOTTOM_END, 0));
			break;

		case SCANLINE_EFX_BOTTOM_END:
			m_write_efx(CLEAR_LINE);
			m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_TOP_START, 0));
			break;
		}
		break;

	case TIMER_DMA:
		if (m_dmaout)
		{
			if (m_disp)
			{
				if (scanline >= SCANLINE_DISPLAY_START && scanline < SCANLINE_DISPLAY_END)
				{
					m_write_dma_out(CLEAR_LINE);
				}
			}

			m_dma_timer->adjust(clocks_to_attotime(CDP1861_CYCLES_DMA_WAIT));

			m_dmaout = CLEAR_LINE;
		}
		else
		{
			if (m_disp)
			{
				if (scanline >= SCANLINE_DISPLAY_START && scanline < SCANLINE_DISPLAY_END)
				{
					m_write_dma_out(ASSERT_LINE);
				}
			}

			m_dma_timer->adjust(clocks_to_attotime(CDP1861_CYCLES_DMA_ACTIVE));

			m_dmaout = ASSERT_LINE;
		}
		break;
	}
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void cdp1861_device::dma_w(uint8_t data)
{
	int sx = screen().hpos() + 4;
	int y = screen().vpos();

	for (int x = 0; x < 8; x++)
	{
		pen_t color = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
		m_bitmap.pix(y, sx + x) = color;
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

	m_write_int(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);
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
