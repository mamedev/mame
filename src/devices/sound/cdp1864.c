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

#include "cdp1864.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1864_DEFAULT_LATCH       0x35

#define CDP1864_CYCLES_DMA_START    2*8
#define CDP1864_CYCLES_DMA_ACTIVE   8*8
#define CDP1864_CYCLES_DMA_WAIT     6*8

const int cdp1864_device::bckgnd[] = { 2, 0, 4, 1 };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// devices
const device_type CDP1864 = &device_creator<cdp1864_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1864_device - constructor
//-------------------------------------------------

cdp1864_device::cdp1864_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDP1864, "CDP1864", tag, owner, clock, "cdp1864", __FILE__),
		device_sound_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_read_inlace(*this),
		m_read_rdata(*this),
		m_read_bdata(*this),
		m_read_gdata(*this),
		m_write_irq(*this),
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
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1864_device::device_start()
{
	// resolve callbacks
	m_read_inlace.resolve_safe(1);
	m_read_rdata.resolve_safe(0);
	m_read_bdata.resolve_safe(0);
	m_read_gdata.resolve_safe(0);
	m_write_irq.resolve_safe();
	m_write_dma_out.resolve_safe();
	m_write_efx.resolve_safe();
	m_write_hsync.resolve_safe();

	// initialize palette
	initialize_palette();

	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());

	// allocate timers
	m_int_timer = timer_alloc(TIMER_INT);
	m_efx_timer = timer_alloc(TIMER_EFX);
	m_dma_timer = timer_alloc(TIMER_DMA);
	m_hsync_timer = timer_alloc(TIMER_HSYNC);

	// find devices
	m_screen->register_screen_bitmap(m_bitmap);

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
	m_int_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_INT_START, 0));
	m_efx_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_EFX_TOP_START, 0));
	m_dma_timer->adjust(clocks_to_attotime(CDP1864_CYCLES_DMA_START));

	m_disp = 0;
	m_dmaout = 0;

	m_write_irq(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);
	m_write_efx(CLEAR_LINE);
	m_write_hsync(CLEAR_LINE);
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void cdp1864_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int scanline = m_screen->vpos();

	switch (id)
	{
	case TIMER_INT:
		if (scanline == CDP1864_SCANLINE_INT_START)
		{
			if (m_disp)
			{
				m_write_irq(ASSERT_LINE);
			}

			m_int_timer->adjust(m_screen->time_until_pos( CDP1864_SCANLINE_INT_END, 0));
		}
		else
		{
			if (m_disp)
			{
				m_write_irq(CLEAR_LINE);
			}

			m_int_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_INT_START, 0));
		}
		break;

	case TIMER_EFX:
		switch (scanline)
		{
		case CDP1864_SCANLINE_EFX_TOP_START:
			m_write_efx(ASSERT_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_EFX_TOP_END, 0));
			break;

		case CDP1864_SCANLINE_EFX_TOP_END:
			m_write_efx(CLEAR_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_EFX_BOTTOM_START, 0));
			break;

		case CDP1864_SCANLINE_EFX_BOTTOM_START:
			m_write_efx(ASSERT_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_EFX_BOTTOM_END, 0));
			break;

		case CDP1864_SCANLINE_EFX_BOTTOM_END:
			m_write_efx(CLEAR_LINE);
			m_efx_timer->adjust(m_screen->time_until_pos(CDP1864_SCANLINE_EFX_TOP_START, 0));
			break;
		}
		break;

	case TIMER_DMA:
		if (m_dmaout)
		{
			if (m_disp)
			{
				if (scanline >= CDP1864_SCANLINE_DISPLAY_START && scanline < CDP1864_SCANLINE_DISPLAY_END)
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
				if (scanline >= CDP1864_SCANLINE_DISPLAY_START && scanline < CDP1864_SCANLINE_DISPLAY_END)
				{
					m_write_dma_out(ASSERT_LINE);
				}
			}

			m_dma_timer->adjust(clocks_to_attotime(CDP1864_CYCLES_DMA_ACTIVE));

			m_dmaout = 1;
		}
		break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void cdp1864_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	INT16 signal = m_signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (m_aoe)
	{
		double frequency = unscaled_clock() / 8 / 4 / (m_latch + 1) / 2;
		int rate = machine().sample_rate() / 2;

		/* get progress through wave */
		int incr = m_incr;

		if (signal < 0)
		{
			signal = -0x7fff;
		}
		else
		{
			signal = 0x7fff;
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
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

READ8_MEMBER( cdp1864_device::dispon_r )
{
	m_disp = 1;

	return 0xff;
}


//-------------------------------------------------
//  dispoff_r -
//-------------------------------------------------

READ8_MEMBER( cdp1864_device::dispoff_r )
{
	m_disp = 0;

	m_write_irq(CLEAR_LINE);
	m_write_dma_out(CLEAR_LINE);

	return 0xff;
}


//-------------------------------------------------
//  step_bgcolor_w -
//-------------------------------------------------

WRITE8_MEMBER( cdp1864_device::step_bgcolor_w )
{
	m_disp = 1;

	m_bgcolor++;
	m_bgcolor &= 0x03;
}


//-------------------------------------------------
//  tone_latch_w -
//-------------------------------------------------

WRITE8_MEMBER( cdp1864_device::tone_latch_w )
{
	m_latch = data;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( cdp1864_device::dma_w )
{
	int rdata = 1, bdata = 1, gdata = 1;
	int sx = m_screen->hpos() + 4;
	int y = m_screen->vpos();

	if (!m_con)
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

		m_bitmap.pix32(y, sx + x) = m_palette[color];

		data <<= 1;
	}
}


//-------------------------------------------------
//  con_w - color on write
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1864_device::con_w )
{
	if (!state)
	{
		m_con = 0;
	}
}


//-------------------------------------------------
//  aoe_w - audio output enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1864_device::aoe_w )
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

WRITE_LINE_MEMBER( cdp1864_device::evs_w )
{
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 cdp1864_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_disp)
	{
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
		m_bitmap.fill(m_palette[bckgnd[m_bgcolor] + 8], cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

void cdp1864_device::initialize_palette()
{
	const int resistances_r[] = { m_chr_r };
	const int resistances_g[] = { m_chr_g };
	const int resistances_b[] = { m_chr_b };

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
		UINT8 r = 0, g = 0, b = 0;

		if (m_chr_r != RES_INF) r = combine_1_weights(color_weights_r, BIT(i, 0));
		if (m_chr_b != RES_INF) b = combine_1_weights(color_weights_b, BIT(i, 1));
		if (m_chr_g != RES_INF) g = combine_1_weights(color_weights_g, BIT(i, 2));

		m_palette[i] = rgb_t(r, g, b);

		// background colors
		r = 0, g = 0, b = 0;

		if (m_chr_r != RES_INF) r = combine_1_weights(color_weights_bkg_r, BIT(i, 0));
		if (m_chr_b != RES_INF) b = combine_1_weights(color_weights_bkg_b, BIT(i, 1));
		if (m_chr_g != RES_INF) g = combine_1_weights(color_weights_bkg_g, BIT(i, 2));

		m_palette[i + 8] = rgb_t(r, g, b);
	}
}
