/**********************************************************************

    RCA CDP1861 Video Display Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "cdp1861.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1861_CYCLES_DMA_START    2*8
#define CDP1861_CYCLES_DMA_ACTIVE   8*8
#define CDP1861_CYCLES_DMA_WAIT     6*8



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type CDP1861 = &device_creator<cdp1861_device>;

//-------------------------------------------------
//  cdp1861_device - constructor
//-------------------------------------------------

cdp1861_device::cdp1861_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDP1861, "CDP1861", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1861_device::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1861_interface *intf = reinterpret_cast<const cdp1861_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1861_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_dmao_cb, 0, sizeof(m_out_dmao_cb));
		memset(&m_out_efx_cb, 0, sizeof(m_out_efx_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1861_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_dmao_func.resolve(m_out_dmao_cb, *this);
	m_out_efx_func.resolve(m_out_efx_cb, *this);

	// allocate timers
	m_int_timer = timer_alloc(TIMER_INT);
	m_efx_timer = timer_alloc(TIMER_EFX);
	m_dma_timer = timer_alloc(TIMER_DMA);

	// find devices
	m_cpu = machine().device<cpu_device>(m_cpu_tag);
	m_screen =  machine().device<screen_device>(m_screen_tag);
	m_screen->register_screen_bitmap(m_bitmap);

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
	m_int_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_INT_START, 0));
	m_efx_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_EFX_TOP_START, 0));
	m_dma_timer->adjust(m_cpu->cycles_to_attotime(CDP1861_CYCLES_DMA_START));

	m_disp = 0;
	m_dmaout = 0;

	m_out_int_func(CLEAR_LINE);
	m_out_dmao_func(CLEAR_LINE);
	m_out_efx_func(CLEAR_LINE);
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void cdp1861_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int scanline = m_screen->vpos();

	switch (id)
	{
	case TIMER_INT:
		if (scanline == CDP1861_SCANLINE_INT_START)
		{
			if (m_disp)
			{
				m_out_int_func(ASSERT_LINE);
			}

			m_int_timer->adjust(m_screen->time_until_pos( CDP1861_SCANLINE_INT_END, 0));
		}
		else
		{
			if (m_disp)
			{
				m_out_int_func(CLEAR_LINE);
			}

			m_int_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_INT_START, 0));
		}
		break;

	case TIMER_EFX:
		switch (scanline)
		{
		case CDP1861_SCANLINE_EFX_TOP_START:
			m_out_efx_func(ASSERT_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_EFX_TOP_END, 0));
			break;

		case CDP1861_SCANLINE_EFX_TOP_END:
			m_out_efx_func(CLEAR_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_EFX_BOTTOM_START, 0));
			break;

		case CDP1861_SCANLINE_EFX_BOTTOM_START:
			m_out_efx_func(ASSERT_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_EFX_BOTTOM_END, 0));
			break;

		case CDP1861_SCANLINE_EFX_BOTTOM_END:
			m_out_efx_func(CLEAR_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1861_SCANLINE_EFX_TOP_START, 0));
			break;
		}
		break;

	case TIMER_DMA:
		if (m_dmaout)
		{
			if (m_disp)
			{
				if (scanline >= CDP1861_SCANLINE_DISPLAY_START && scanline < CDP1861_SCANLINE_DISPLAY_END)
				{
					m_out_dmao_func(CLEAR_LINE);
				}
			}

			m_dma_timer->adjust(m_cpu->cycles_to_attotime(CDP1861_CYCLES_DMA_WAIT));

			m_dmaout = 0;
		}
		else
		{
			if (m_disp)
			{
				if (scanline >= CDP1861_SCANLINE_DISPLAY_START && scanline < CDP1861_SCANLINE_DISPLAY_END)
				{
					m_out_dmao_func(ASSERT_LINE);
				}
			}

			m_dma_timer->adjust(m_cpu->cycles_to_attotime(CDP1861_CYCLES_DMA_ACTIVE));

			m_dmaout = 1;
		}
		break;
	}
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( cdp1861_device::dma_w )
{
	int sx = m_screen->hpos() + 4;
	int y = m_screen->vpos();
	int x;

	for (x = 0; x < 8; x++)
	{
		int color = BIT(data, 7);
		m_bitmap.pix32(y, sx + x) = RGB_MONOCHROME_WHITE[color];
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

	m_out_int_func(CLEAR_LINE);
	m_out_dmao_func(CLEAR_LINE);
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 cdp1861_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_disp)
	{
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		bitmap.fill(RGB_BLACK, cliprect);
	}
	return 0;
}
