// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

**********************************************************************/

#include "emu.h"
#include "cdp1864.h"
#include "screen.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1864_DEFAULT_LATCH       0x35

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
	m_int(0),
	m_efx(0),
	m_bgcolor(0),
	m_con(0),
	m_aoe(0),
	m_latch(CDP1864_DEFAULT_LATCH),
	m_tpa(0),
	m_sc(0),
	m_line(0),
	m_cycle(0),
	m_next_line(0),
	m_next_cycle(0)
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

	if (!screen().has_been_setup())
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

	// find devices
	screen().register_screen_bitmap(m_bitmap);

	// register for state saving
	save_item(NAME(m_disp));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_int));
	save_item(NAME(m_efx));
	save_item(NAME(m_bgcolor));
	save_item(NAME(m_con));
	save_item(NAME(m_aoe));
	save_item(NAME(m_latch));
	save_item(NAME(m_signal));
	save_item(NAME(m_incr));
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

void cdp1864_device::device_reset()
{
	m_line = m_next_line = 0;
	m_cycle = m_next_cycle = 0;

	m_disp = 0;

	set_int(CLEAR_LINE);
	set_dma_out(CLEAR_LINE);
	set_efx(CLEAR_LINE);
	m_write_hsync(CLEAR_LINE);
}


//-------------------------------------------------
//  output lines
//-------------------------------------------------

void cdp1864_device::set_int(int state)
{
	m_int = state;
	m_write_int(state);
}

void cdp1864_device::set_dma_out(int state)
{
	m_dmaout = state;
	m_write_dma_out(state);
}

void cdp1864_device::set_efx(int state)
{
	m_efx = state;
	m_write_efx(state);
}


//-------------------------------------------------
//  tpa_w - clock the horizontal line counter
//-------------------------------------------------

void cdp1864_device::tpa_w(int state)
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

		// a fetch cycle (S0) at horizontal sync makes the line one cycle short
		m_next_cycle = m_cycle + ((m_cycle == 0 && m_sc == 0) ? 2 : 1);

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

void cdp1864_device::sc_w(uint8_t data)
{
	m_sc = data & 0x03;
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

	set_int(CLEAR_LINE);
	set_dma_out(CLEAR_LINE);

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
	int sx = m_cycle * 8 + 4;

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

		if (m_bitmap.cliprect().contains(sx + x, m_line))
			m_bitmap.pix(m_line, sx + x) = m_palette[color];

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
