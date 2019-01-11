// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1861 Video Display Controller emulation

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
		screen().set_screen_update(screen_update_rgb32_delegate(FUNC(cdp1861_device::screen_update), this));
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

void cdp1861_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

WRITE8_MEMBER( cdp1861_device::dma_w )
{
	int sx = screen().hpos() + 4;
	int y = screen().vpos();
	int x;

	for (x = 0; x < 8; x++)
	{
		pen_t color = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
		m_bitmap.pix32(y, sx + x) = color;
		data <<= 1;
	}
}


//-------------------------------------------------
//  disp_on_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1861_device::disp_on_w )
{
	if (!m_dispon && state) m_disp = 1;

	m_dispon = state;
}


//-------------------------------------------------
//  disp_off_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1861_device::disp_off_w )
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
