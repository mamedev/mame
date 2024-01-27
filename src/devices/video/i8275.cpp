// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Intel 8275 Programmable CRT Controller
    Intel 8276 Small Systems CRT Controller

    This emulation allows up to four 8275 or 8276 CRTCs connected in
    parallel and reading out their row buffers simultaneously. The
    secondary CRTCs are assumed to use identical timings to the
    primary CRTC and run in perfect sync with each other.

    The features provided by 8276 are practically a subset of the
    8275. Light pen input, line attributes, invisible character
    attributes and DMA bursts are not supported on the 8276. Also,
    the DRQ and _DACK pins are renamed BRDY (Buffer Ready) and _BS
    (Buffer Select), and the latter must be asserted coincident
    with _WR (but not _CS). (Even in systems without a DMAC, the
    processor usually does not write to the row buffer directly,
    but enables some pseudo-DMA mode which causes memory read
    operations to transmit data to the CRTC in the same cycle.)

**********************************************************************/

/*

    TODO:

    - double spaced rows
    - preset counters - how it affects DMA and HRTC?

*/

#include "emu.h"
#include "i8275.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


const int i8275_device::character_attribute[3][16] =
{
	{ 2, 2, 4, 4, 2, 4, 4, 4, 2, 4, 4, 0, 2, 0, 0, 0 },
	{ 8, 0xc, 8, 0xc, 1, 0xc, 8, 1, 1, 4, 1, 0, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 4, 4, 4, 2, 2, 4, 4, 0, 2, 0, 0, 0 }
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(I8275, i8275_device, "i8275", "Intel 8275 CRTC")
DEFINE_DEVICE_TYPE(I8276, i8276_device, "i8276", "Intel 8276 CRTC")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8275_device - constructor
//-------------------------------------------------

i8275_device::i8275_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_drq(*this),
	m_write_hrtc(*this),
	m_write_vrtc(*this),
	m_write_lc(*this),
	m_display_cb(*this),
	m_refresh_hack(false),
	m_next_crtc(*this, finder_base::DUMMY_TAG),
	m_is_crtc0(true),
	m_status(0),
	m_param_idx(0),
	m_param_end(0),
	m_buffer_idx(0),
	m_fifo_idx(0),
	m_fifo_idx_out(0),
	m_dma_idx(0),
	m_dma_last_char(0),
	m_buffer_dma(0),
	m_lpen(0),
	m_scanline(0),
	m_dma_stop(false),
	m_end_of_screen(false),
	m_preset(false),
	m_cursor_blink(0),
	m_char_blink(0),
	m_stored_attr(0),
	m_field_attr(0)
{
	memset(m_param, 0x00, sizeof(m_param));
}

i8275_device::i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8275_device(mconfig, I8275, tag, owner, clock)
{
}

i8276_device::i8276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8275_device(mconfig, I8276, tag, owner, clock)
{
}


void i8275_device::device_resolve_objects()
{
	if (m_next_crtc.found())
		m_next_crtc->m_is_crtc0 = false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8275_device::device_start()
{
	if (m_is_crtc0)
	{
		// get the screen device
		screen().register_screen_bitmap(m_bitmap);

		// resolve delegates
		m_display_cb.resolve_safe();

		// allocate timers
		m_hrtc_on_timer = timer_alloc(FUNC(i8275_device::hrtc_on), this);
		m_drq_on_timer = timer_alloc(FUNC(i8275_device::drq_on), this);
		m_scanline_timer = timer_alloc(FUNC(i8275_device::scanline_tick), this);
	}

	// state saving
	save_item(NAME(m_status));
	save_item(NAME(m_param));
	save_item(NAME(m_param_idx));
	save_item(NAME(m_param_end));
	save_item(NAME(m_buffer[0]));
	save_item(NAME(m_buffer[1]));
	save_item(NAME(m_fifo[0]));
	save_item(NAME(m_fifo[1]));
	save_item(NAME(m_buffer_idx));
	save_item(NAME(m_fifo_idx));
	save_item(NAME(m_dma_idx));
	save_item(NAME(m_dma_last_char));
	save_item(NAME(m_buffer_dma));
	save_item(NAME(m_lpen));
	save_item(NAME(m_scanline));
	save_item(NAME(m_irq_scanline));
	save_item(NAME(m_vrtc_scanline));
	save_item(NAME(m_vrtc_drq_scanline));
	save_item(NAME(m_dma_stop));
	save_item(NAME(m_end_of_screen));
	save_item(NAME(m_preset));
	save_item(NAME(m_cursor_blink));
	save_item(NAME(m_char_blink));
	save_item(NAME(m_stored_attr));
	save_item(NAME(m_field_attr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8275_device::device_reset()
{
	memset(m_buffer, 0, sizeof(m_buffer));

	m_status &= ~ST_IE;

	m_write_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  vrtc_start - update for beginning of vertical
//  retrace period
//-------------------------------------------------

void i8275_device::vrtc_start()
{
	// reset field attributes
	m_field_attr = 0;

	// Intel datasheets imply DMA requests begin only after a "Start Display" command is issued.
	// WY-100, however, expects a BRDY cycle from the 8276 after the program first configures and stops the display.
	// This suggests that DMA bursts proceed as normal in this case, but any characters sent will not be displayed.
	m_buffer_idx = characters_per_row();
	m_dma_stop = false;
	m_end_of_screen = !(m_status & ST_VE);

	m_cursor_blink++;
	m_cursor_blink &= 0x1f;

	m_char_blink++;
	m_char_blink &= 0x3f;
	m_stored_attr = 0;
}


//-------------------------------------------------
//  vrtc_end - update for end of vertical retrace
//  period
//-------------------------------------------------

void i8275_device::vrtc_end()
{
	//LOG("I8275 y %u x %u VRTC 0\n", y, x);
	m_write_vrtc(0);
}


//-------------------------------------------------
//  dma_start - start DMA for a new row
//-------------------------------------------------

void i8275_device::dma_start()
{
	m_buffer_idx = 0;
	m_fifo_idx = 0;
	m_dma_idx = 0;
	m_dma_last_char = 0;

	if (m_is_crtc0)
		m_drq_on_timer->adjust(clocks_to_attotime(dma_burst_space()));
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(i8275_device::hrtc_on)
{
	m_write_hrtc(1);
}

TIMER_CALLBACK_MEMBER(i8275_device::drq_on)
{
	m_write_drq(1);
}

TIMER_CALLBACK_MEMBER(i8275_device::scanline_tick)
{
	int rc = m_scanline / scanlines_per_row();
	int lc = m_scanline % scanlines_per_row();

	int line_counter = offset_line_counter() ? ((lc - 1) % scanlines_per_row()) : lc;
	m_write_lc(line_counter);
	m_write_hrtc(0);

	if (m_scanline == 0)
		vrtc_end();

	if (lc == 0 && m_scanline < m_vrtc_scanline)
	{
		for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc)
		{
			if (!crtc->m_dma_stop && crtc->m_buffer_idx < characters_per_row())
			{
				crtc->m_status |= ST_DU;
				crtc->m_dma_stop = true;

				// blank screen until after VRTC
				crtc->m_end_of_screen = true;

				crtc->m_write_drq(0);
			}

			if (!crtc->m_dma_stop)
			{
				// swap line buffers
				crtc->m_buffer_dma = !crtc->m_buffer_dma;

				if (m_scanline < (m_vrtc_scanline - scanlines_per_row()))
					crtc->dma_start();
			}
		}
	}

	if (m_scanline == m_irq_scanline)
	{
		for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc)
		{
			if ((crtc->m_status & ST_IE) && !(crtc->m_status & ST_IR))
			{
				crtc->m_status |= ST_IR;
				crtc->m_write_irq(ASSERT_LINE);
			}
		}
	}

	if (m_scanline == m_vrtc_scanline)
	{
		//LOG("I8275 y %u x %u VRTC 1\n", y, x);
		m_write_vrtc(1);

		for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc)
			crtc->vrtc_start();
	}

	if (m_scanline == m_vrtc_drq_scanline)
	{
		for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc)
		{
			if (!crtc->m_dma_stop)
			{
				// swap line buffers
				crtc->m_buffer_dma = !crtc->m_buffer_dma;

				// start DMA burst
				crtc->dma_start();
			}
		}
	}

	if (m_scanline < m_vrtc_scanline)
	{
		int end_of_row = 0;
		int blank_row = 0;
		{
			int n = 0;
			for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc, n++)
			{
				if (crtc->m_end_of_screen || !(m_status & ST_VE))
					end_of_row |= 1 << n;
				if ((crtc->underline() >= 8) && ((lc == 0) || (lc == scanlines_per_row() - 1)))
					blank_row |= 1 << n;
				crtc->m_fifo_idx_out = 0;
				crtc->m_field_attr = crtc->m_stored_attr;
			}
		}

		for (int sx = 0; sx < characters_per_row(); sx++)
		{
			int n = 0;
			uint32_t charcode = 0;
			uint32_t attrcode = 0;

			for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc, n++)
			{
				auto [data, attr] = crtc->char_from_buffer(n, sx, rc, lc, end_of_row, blank_row);
				charcode |= uint32_t(data) << (n * 8);
				attrcode |= uint32_t(attr) << (n * 8);
			}

			m_display_cb(m_bitmap,
					sx * m_hpixels_per_column, // x position on screen of starting point
					m_scanline, // y position on screen
					line_counter, // current line of char
					charcode,  // char code to be displayed
					attrcode);
		}

		if ((scanlines_per_row() - lc) == 1)
		{
			for (i8275_device *crtc = this; crtc != nullptr; crtc = crtc->m_next_crtc)
				crtc->m_stored_attr = crtc->m_field_attr;
		}
	}

	m_scanline++;
	m_scanline %= ((character_rows_per_frame() + vrtc_row_count()) * scanlines_per_row());
}


std::pair<uint8_t, uint8_t> i8275_device::char_from_buffer(int n, int sx, int rc, int lc, int &end_of_row, int blank_row)
{
	uint8_t data = BIT(end_of_row, n) ? 0 : m_buffer[!m_buffer_dma][sx];
	uint8_t attr = m_field_attr;
	int lineattr = 0;

	if ((data & 0xc0) == 0x80)
	{
		// field attribute code
		m_field_attr = data & (FAC_H | FAC_B | FAC_GG | FAC_R | FAC_U);

		if (!visible_field_attribute())
		{
			attr = m_field_attr;
			data = m_fifo[!m_buffer_dma][m_fifo_idx_out];

			m_fifo_idx_out++;
			m_fifo_idx_out &= 0xf;

			if (BIT(blank_row, n))
				attr |= FAC_B;
			else if (!(m_char_blink < 32))
				attr &= ~FAC_B;
			if (lc != underline())
				attr &= ~FAC_U;
		}
		else
		{
			// simply blank the attribute character itself
			attr = FAC_B;
		}
	}
	else if (data >= 0xf0 || BIT(end_of_row, n))
	{
		// special control character
		switch (data)
		{
		case SCC_END_OF_ROW:
		case SCC_END_OF_ROW_DMA:
			end_of_row |= 1 << n;
			break;

		case SCC_END_OF_SCREEN:
		case SCC_END_OF_SCREEN_DMA:
			m_end_of_screen = true;
			break;
		}
		attr = FAC_B;
	}
	else if (data >= 0xc0)
	{
		// character attribute code
		attr = data & (m_char_blink < 32 ? (CA_H | CA_B) : CA_H);

		uint8_t ca;
		int cccc = (data >> 2) & 0x0f;

		if (lc < underline())
		{
			ca = character_attribute[0][cccc];
		}
		else if (lc == underline())
		{
			ca = character_attribute[1][cccc];
		}
		else
		{
			ca = character_attribute[2][cccc];
		}

		if (ca & CA_LTEN)
			attr |= FAC_U;
		if (ca & CA_VSP)
			attr |= FAC_B;
		lineattr = ca >> 2;
	}
	else
	{
		if (BIT(blank_row, n))
			attr |= FAC_B;
		else if (!(m_char_blink < 32))
			attr &= ~FAC_B;
		if (lc != underline())
			attr &= ~FAC_U;
	}

	if ((rc == m_param[REG_CUR_ROW]) && (sx == m_param[REG_CUR_COL]))
	{
		if ((cursor_format() & 0x02) || (m_cursor_blink < 16))
		{
			if (cursor_format() & 0x01)
				attr |= (lc == underline()) ? FAC_U : 0;
			else
				attr ^= FAC_R;
		}
	}

	return std::make_pair(data & 0x7f, attr | lineattr << 6);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t i8275_device::read(offs_t offset)
{
	if (offset & 0x01)
	{
		uint8_t status = m_status;

		if (!machine().side_effects_disabled())
		{
			if (m_status & ST_IR)
			{
				//LOG("I8275 IRQ 0\n");
				m_write_irq(CLEAR_LINE);
			}

			m_status &= ~(ST_IR | ST_LP | ST_IC | ST_DU | ST_FO);
		}

		return status;
	}
	else
	{
		uint8_t data = m_param[m_param_idx];

		if (!machine().side_effects_disabled())
		{
			m_param_idx++;
			if (m_param_idx > m_param_end)
			{
				m_status |= ST_IC;
			}
		}

		return data;
	}
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void i8275_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x01)
	{
		LOG("I8275 Command %02x\n", data);

		/*
		 * "internal timing counters are preset, corresponding to a screen display
		 * position at the top left corner.  Two character clocks are required
		 * for this operation.  The counters will remain in this state until any
		 * other command is given."
		 */
		if (m_preset)
		{
			int hrtc_on_pos = characters_per_row() * m_hpixels_per_column;

			m_preset = false;
			m_scanline = m_vrtc_drq_scanline;
			if (m_is_crtc0)
			{
				m_hrtc_on_timer->adjust(screen().time_until_pos(screen().vpos(), hrtc_on_pos), 0, screen().scan_period());
				m_scanline_timer->adjust(screen().time_until_pos(m_vrtc_drq_scanline, 0), 0, screen().scan_period());
			}
		}

		switch (data >> 5)
		{
		/*
		 * DMA requests stop;
		 * IE is reset, interrupts are disabled;
		 * VE is reset, VSP output is used to blank the screen;
		 * HRTC and VRTC continue to run;
		 * HRTC and VRTC timing are random on power-up.
		 */
		case CMD_RESET:
			LOG("I8275 Reset\n");

			m_status &= ~(ST_IE | ST_IR | ST_VE);
			LOG("I8275 IRQ 0\n");
			m_write_irq(CLEAR_LINE);
			m_write_drq(0);
			if (m_is_crtc0)
				m_drq_on_timer->adjust(attotime::never);
			m_dma_stop = true;

			m_param_idx = REG_SCN1;
			m_param_end = REG_SCN4;
			break;

		/*
		 * IE is set, interrupts are enabled;
		 * VE is set, video is enabled;
		 * DMA requests begin.
		 */
		case CMD_START_DISPLAY:
			m_param[REG_DMA] = data;
			LOG("I8275 Start Display %u %u\n", dma_burst_count(), dma_burst_space());
			m_dma_stop = false;
			m_status |= (ST_IE | ST_VE);
			break;

		/*
		 * interrupts remain enabled;
		 * HRTC and VRTC continue to run.
		 */
		case CMD_STOP_DISPLAY:
			LOG("I8275 Stop Display\n");
			m_status &= ~ST_VE;
			break;

		case CMD_READ_LIGHT_PEN:
			LOG("I8275 Read Light Pen\n");
			m_param_idx = REG_LPEN_COL;
			m_param_end = REG_LPEN_ROW;
			break;

		case CMD_LOAD_CURSOR:
			LOG("I8275 Load Cursor\n");
			m_param_idx = REG_CUR_COL;
			m_param_end = REG_CUR_ROW;
			break;

		case CMD_ENABLE_INTERRUPT:
			LOG("I8275 Enable Interrupt\n");
			m_status |= ST_IE;
			break;

		case CMD_DISABLE_INTERRUPT:
			LOG("I8275 Disable Interrupt\n");
			m_status &= ~ST_IE;
			break;

		case CMD_PRESET_COUNTERS:
			LOG("I8275 Preset Counters\n");
			m_preset = true;
			if (m_is_crtc0)
			{
				m_scanline_timer->adjust(attotime::never);
				m_hrtc_on_timer->adjust(attotime::never);
			}
			break;
		}
	}
	else
	{
		LOG("I8275 Parameter %02x\n", data);

		m_param[m_param_idx] = data;

		if (m_param_idx == REG_SCN4)
			recompute_parameters();

		m_param_idx++;
	}
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

void i8275_device::dack_w(uint8_t data)
{
	//LOG("I8275 y %u x %u DACK %02x %u (%u)\n", screen().vpos(), screen().hpos(), data, m_buffer_idx, m_dma_idx);

	m_write_drq(0);

	if (!visible_field_attribute() && ((m_dma_last_char & 0xc0) == 0x80))
	{
		if (m_fifo_idx == 16)
		{
			m_fifo_idx = 0;
			m_status |= ST_FO;
		}

		// FIFO is 7 bits wide
		m_fifo[m_buffer_dma][m_fifo_idx++] = data & 0x7f;

		data = 0;
	}
	else if (m_buffer_idx < characters_per_row())
	{
		m_buffer[m_buffer_dma][m_buffer_idx++] = data;
	}

	m_dma_idx++;

	switch (m_dma_last_char)
	{
	case SCC_END_OF_ROW_DMA:
		// stop DMA
		m_buffer_idx = characters_per_row();
		break;

	case SCC_END_OF_SCREEN_DMA:
		m_dma_stop = true;
		m_buffer_idx = characters_per_row();
		break;

	default:
		if (m_is_crtc0)
		{
			if (m_buffer_idx == characters_per_row())
			{
				// stop DMA
				m_drq_on_timer->adjust(attotime::never);
			}
			else if (!(m_dma_idx % dma_burst_count()))
			{
				m_drq_on_timer->adjust(clocks_to_attotime(dma_burst_space()));
			}
			else
			{
				m_drq_on_timer->adjust(attotime::zero);
			}
		}
	}

	m_dma_last_char = data;
}


//-------------------------------------------------
//  lpen_w -
//-------------------------------------------------

void i8275_device::lpen_w(int state)
{
	if (!m_lpen && state)
	{
		m_param[REG_LPEN_COL] = screen().hpos() / m_hpixels_per_column;
		m_param[REG_LPEN_ROW] = screen().vpos() / scanlines_per_row();

		m_status |= ST_LP;
	}

	m_lpen = state;
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t i8275_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!(m_status & ST_VE))
	{
		m_bitmap.fill(rgb_t::black(), cliprect);
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

void i8275_device::recompute_parameters()
{
	if (!m_is_crtc0)
		return;

	int y = screen().vpos();

	int horiz_pix_total = (characters_per_row() + hrtc_count()) * m_hpixels_per_column;
	int vert_pix_total = (character_rows_per_frame() + vrtc_row_count()) * scanlines_per_row();
	attotime refresh = clocks_to_attotime((characters_per_row() + hrtc_count()) * vert_pix_total);
	int max_visible_x = (characters_per_row() * m_hpixels_per_column) - 1;
	int max_visible_y = (character_rows_per_frame() * scanlines_per_row()) - 1;

	LOG("width %u height %u max_x %u max_y %u refresh %f\n", horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, refresh.as_hz());

	rectangle visarea(0, max_visible_x, 0, max_visible_y);
	screen().configure(horiz_pix_total, vert_pix_total, visarea, (m_refresh_hack ? screen().frame_period() : refresh).as_attoseconds());

	int hrtc_on_pos = characters_per_row() * m_hpixels_per_column;
	m_hrtc_on_timer->adjust(screen().time_until_pos(y, hrtc_on_pos), 0, screen().scan_period());

	m_irq_scanline = (character_rows_per_frame() - 1) * scanlines_per_row();
	m_vrtc_scanline = character_rows_per_frame() * scanlines_per_row();
	m_vrtc_drq_scanline = vert_pix_total - scanlines_per_row();

	LOG("irq_y %u vrtc_y %u drq_y %u\n", m_irq_scanline, m_vrtc_scanline, m_vrtc_drq_scanline);

	m_scanline = y;
	m_scanline_timer->adjust(screen().time_until_pos((y + 1) % vert_pix_total, 0), 0, screen().scan_period());

	if (double_spaced_rows()) fatalerror("Double spaced rows not supported!");
}
