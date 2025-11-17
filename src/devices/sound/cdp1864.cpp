// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

**********************************************************************/

/*

    TODO:

    - interlace mode
    - PAL output, currently using RGB
    - cpu synchronization

        SC1 and SC0 are used to provide CDP1864C-to-CPU synchronization for a jitter-free display.
        During every horizontal sync the CDP1864C samples SC0 and SC1 for SC0 = 1 and SC1 = 0
        (CDP1800 execute state). Detection of a fetch cycle causes the CDP1864C to skip cycles to
        attain synchronization. (i.e. picture moves 8 pixels to the right)

*/

#include "emu.h"
#include "cdp1864.h"
#include "screen.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1864_DEFAULT_LATCH       0x35

#define CDP1864_CYCLES_DMA_START    2*8
#define CDP1864_CYCLES_DMA_ACTIVE   8*8
#define CDP1864_CYCLES_DMA_WAIT     6*8

constexpr int cdp1864_device::bckgnd[4];



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(CDP1864, cdp1864_device, "cdp1864", "RCA CDP1864")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1864_device - constructor
//-------------------------------------------------

cdp1864_device::cdp1864_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CDP1864, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_read_inlace(*this, 1),
	m_read_rdata(*this, 0),
	m_read_bdata(*this, 0),
	m_read_gdata(*this, 0),
	m_write_int(*this),
	m_write_dma_out(*this),
	m_write_efx(*this),
	m_write_hsync(*this),
	m_disp(0),
	m_dmaout(0),
	m_bgcolor(0),
	m_con(0),
	m_aoe(0),
	m_latch(CDP1864_DEFAULT_LATCH)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1864_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock(), SCREEN_WIDTH, HBLANK_END, HBLANK_START, TOTAL_SCANLINES, SCANLINE_VBLANK_END, SCANLINE_VBLANK_START);

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(cdp1864_device::screen_update));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1864_device::device_start()
{
	// initialize palette
	initialize_palette();

	// create sound stream
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);

	// allocate timers
	m_int_timer = timer_alloc(FUNC(cdp1864_device::int_tick), this);
	m_efx_timer = timer_alloc(FUNC(cdp1864_device::efx_tick), this);
	m_dma_timer = timer_alloc(FUNC(cdp1864_device::dma_tick), this);

	// find devices
	screen().register_screen_bitmap(m_bitmap);

	// register for state saving
	save_item(NAME(m_disp));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_bgcolor));
	save_item(NAME(m_con));
	save_item(NAME(m_aoe));
	save_item(NAME(m_latch));
	save_item(NAME(m_signal));
	save_item(NAME(m_incr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1864_device::device_reset()
{
	m_int_timer->adjust(screen().time_until_pos(SCANLINE_INT_START, 0));
	m_efx_timer->adjust(screen().time_until_pos(SCANLINE_EFX_TOP_START, 0));
	m_dma_timer->adjust(clocks_to_attotime(CDP1864_CYCLES_DMA_START));

	m_disp = 0;
	m_dmaout = 0;

	m_write_int(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);
	m_write_efx(CLEAR_LINE);
	m_write_hsync(CLEAR_LINE);
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cdp1864_device::int_tick)
{
	if (screen().vpos() == SCANLINE_INT_START)
	{
		if (m_disp)
		{
			m_write_int(ASSERT_LINE);
		}

		m_int_timer->adjust(screen().time_until_pos(SCANLINE_INT_END, 0));
	}
	else
	{
		if (m_disp)
		{
			m_write_int(CLEAR_LINE);
		}

		m_int_timer->adjust(screen().time_until_pos(SCANLINE_INT_START, 0));
	}
}

TIMER_CALLBACK_MEMBER(cdp1864_device::efx_tick)
{
	int scanline = screen().vpos();
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
}

TIMER_CALLBACK_MEMBER(cdp1864_device::dma_tick)
{
	int scanline = screen().vpos();
	if (m_dmaout)
	{
		if (m_disp)
		{
			if (scanline >= SCANLINE_DISPLAY_START && scanline < SCANLINE_DISPLAY_END)
			{
				m_write_dma_out(CLEAR_LINE);
			}
		}

		m_dma_timer->adjust(clocks_to_attotime(CDP1864_CYCLES_DMA_WAIT));

		m_dmaout = 0;
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

		m_dma_timer->adjust(clocks_to_attotime(CDP1864_CYCLES_DMA_ACTIVE));

		m_dmaout = 1;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void cdp1864_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t signal = m_signal;

	if (m_aoe)
	{
		double frequency = unscaled_clock() / 8 / 4 / (m_latch + 1) / 2;
		int rate = stream.sample_rate() / 2;

		/* get progress through wave */
		int incr = m_incr;

		if (signal < 0)
		{
			signal = -1.0;
		}
		else
		{
			signal = 1.0;
		}

		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			stream.put(0, sampindex, signal);
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		/* store progress through wave */
		m_incr = incr;
		m_signal = signal;
	}
}


//-------------------------------------------------
//  dispon_r -
//-------------------------------------------------

uint8_t cdp1864_device::dispon_r()
{
	m_disp = 1;

	return 0xff;
}


//-------------------------------------------------
//  dispoff_r -
//-------------------------------------------------

uint8_t cdp1864_device::dispoff_r()
{
	m_disp = 0;

	m_write_int(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);

	return 0xff;
}


//-------------------------------------------------
//  step_bgcolor_w -
//-------------------------------------------------

void cdp1864_device::step_bgcolor_w(uint8_t data)
{
	m_disp = 1;

	m_bgcolor++;
	m_bgcolor &= 0x03;
}


//-------------------------------------------------
//  tone_latch_w -
//-------------------------------------------------

void cdp1864_device::tone_latch_w(uint8_t data)
{
	m_latch = data;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void cdp1864_device::dma_w(uint8_t data)
{
	int rdata = 1, bdata = 1, gdata = 1;
	int sx = screen().hpos() + 4;
	int y = screen().vpos();

	if (m_con)
	{
		rdata = m_read_rdata();
		bdata = m_read_bdata();
		gdata = m_read_gdata();
	}

	for (int x = 0; x < 8; x++)
	{
		int color = bckgnd[m_bgcolor] + 8;

		if (BIT(data, 7))
		{
			color = (gdata << 2) | (bdata << 1) | rdata;
		}

		m_bitmap.pix(y, sx + x) = m_palette[color];

		data <<= 1;
	}
}


//-------------------------------------------------
//  con_w - color on write
//  At start, color is disabled. If the CON
//  pin is taken low (or pulsed low), color is
//  enabled. It can only be disabled again by
//  resetting the chip.
//-------------------------------------------------

void cdp1864_device::con_w(int state)
{
	if (!state)
		m_con = true;
}


//-------------------------------------------------
//  aoe_w - audio output enable write
//-------------------------------------------------

void cdp1864_device::aoe_w(int state)
{
	if (!state)
	{
		m_latch = CDP1864_DEFAULT_LATCH;
	}

	m_aoe = state;
}


//-------------------------------------------------
//  evs_w - external vertical sync write
//-------------------------------------------------

void cdp1864_device::evs_w(int state)
{
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t cdp1864_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_disp)
	{
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
		m_bitmap.fill(m_palette[bckgnd[m_bgcolor] + 8], cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

void cdp1864_device::initialize_palette()
{
	const int resistances_r[] = { static_cast<int>(m_chr_r) };
	const int resistances_g[] = { static_cast<int>(m_chr_g) };
	const int resistances_b[] = { static_cast<int>(m_chr_b) };

	double color_weights_r[1], color_weights_g[1], color_weights_b[1];
	double color_weights_bkg_r[1], color_weights_bkg_g[1], color_weights_bkg_b[1];

	compute_resistor_weights(0, 0xff, -1.0,
								1, resistances_r, color_weights_r, 0, m_chr_bkg,
								1, resistances_g, color_weights_g, 0, m_chr_bkg,
								1, resistances_b, color_weights_b, 0, m_chr_bkg);

	compute_resistor_weights(0, 0xff, -1.0,
								1, resistances_r, color_weights_bkg_r, m_chr_bkg, 0,
								1, resistances_g, color_weights_bkg_g, m_chr_bkg, 0,
								1, resistances_b, color_weights_bkg_b, m_chr_bkg, 0);

	for (int i = 0; i < 8; i++)
	{
		// foreground colors
		uint8_t r = 0, g = 0, b = 0;

		if (m_chr_r != RES_INF) r = combine_weights(color_weights_r, BIT(i, 0));
		if (m_chr_b != RES_INF) b = combine_weights(color_weights_b, BIT(i, 1));
		if (m_chr_g != RES_INF) g = combine_weights(color_weights_g, BIT(i, 2));

		m_palette[i] = rgb_t(r, g, b);

		// background colors
		r = 0, g = 0, b = 0;

		if (m_chr_r != RES_INF) r = combine_weights(color_weights_bkg_r, BIT(i, 0));
		if (m_chr_b != RES_INF) b = combine_weights(color_weights_bkg_b, BIT(i, 1));
		if (m_chr_g != RES_INF) g = combine_weights(color_weights_bkg_g, BIT(i, 2));

		m_palette[i + 8] = rgb_t(r, g, b);
	}
}
