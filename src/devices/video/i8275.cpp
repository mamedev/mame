// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8275 Programmable CRT Controller
    Intel 8276 Small Systems CRT Controller

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


static const int DMA_BURST_SPACING[] = { 0, 7, 15, 23, 31, 39, 47, 55 };


#define DOUBLE_SPACED_ROWS \
	BIT(m_param[REG_SCN1], 7)

#define CHARACTERS_PER_ROW \
	((m_param[REG_SCN1] & 0x7f) + 1)

#define VRTC_ROW_COUNT \
	((m_param[REG_SCN2] >> 6) + 1)

#define CHARACTER_ROWS_PER_FRAME \
	((m_param[REG_SCN2] & 0x3f) + 1)

#define UNDERLINE \
	(m_param[REG_SCN3] >> 4)

#define SCANLINES_PER_ROW \
	((m_param[REG_SCN3] & 0x0f) + 1)

#define OFFSET_LINE_COUNTER \
	BIT(m_param[REG_SCN4], 7)

#define VISIBLE_FIELD_ATTRIBUTE \
	BIT(m_param[REG_SCN4], 6)

#define CURSOR_FORMAT \
	((m_param[REG_SCN4] >> 4) & 0x03)

#define HRTC_COUNT \
	(((m_param[REG_SCN4] & 0x0f) + 1) * 2)

#define DMA_BURST_COUNT \
	(1 << (m_param[REG_DMA] & 0x03))

#define DMA_BURST_SPACE \
	DMA_BURST_SPACING[(m_param[REG_DMA] >> 2) & 0x07]


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
	m_status(0),
	m_param_idx(0),
	m_param_end(0),
	m_buffer_idx(0),
	m_fifo_idx(0),
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


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8275_device::device_start()
{
	// get the screen device
	screen().register_screen_bitmap(m_bitmap);

	// resolve callbacks
	m_write_drq.resolve_safe();
	m_write_irq.resolve_safe();
	m_write_hrtc.resolve_safe();
	m_write_vrtc.resolve_safe();
	m_write_lc.resolve_safe();
	m_display_cb.resolve();

	// allocate timers
	m_hrtc_on_timer = timer_alloc(TIMER_HRTC_ON);
	m_drq_on_timer = timer_alloc(TIMER_DRQ_ON);
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);

	// state saving
	save_item(NAME(m_status));
	save_item(NAME(m_param));
	save_item(NAME(m_param_idx));
	save_item(NAME(m_param_end));
	save_item(NAME(m_buffer[0]));
	save_item(NAME(m_buffer[1]));
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
	//LOG("I8275 y %u x %u VRTC 1\n", y, x);
	m_write_vrtc(1);

	// reset field attributes
	m_field_attr = 0;

	// Intel datasheets imply DMA requests begin only after a "Start Display" command is issued.
	// WY-100, however, expects a BRDY cycle from the 8276 after the program first configures and stops the display.
	// This suggests that DMA bursts proceed as normal in this case, but any characters sent will not be displayed.
	m_buffer_idx = CHARACTERS_PER_ROW;
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

	m_drq_on_timer->adjust(clocks_to_attotime(DMA_BURST_SPACE));
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void i8275_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	//int y = screen().vpos();
	//int x = screen().hpos();
	int rc = m_scanline / SCANLINES_PER_ROW;
	int lc = m_scanline % SCANLINES_PER_ROW;

	switch (id)
	{
	case TIMER_HRTC_ON:
		//LOG("I8275 y %u x %u HRTC 1\n", y, x);
		m_write_hrtc(1);
		break;

	case TIMER_DRQ_ON:
		//LOG("I8275 y %u x %u DRQ 1\n", y, x);
		m_write_drq(1);
		break;

	case TIMER_SCANLINE:
		//LOG("I8275 y %u x %u HRTC 0\n", y, x);
		int line_counter = OFFSET_LINE_COUNTER ? ((lc - 1) % SCANLINES_PER_ROW) : lc;
		m_write_lc(line_counter);
		m_write_hrtc(0);

		if (m_scanline == 0)
			vrtc_end();

		if (lc == 0 && m_scanline < m_vrtc_scanline)
		{
			if (!m_dma_stop && m_buffer_idx < CHARACTERS_PER_ROW)
			{
				m_status |= ST_DU;
				m_dma_stop = true;

				// blank screen until after VRTC
				m_end_of_screen = true;

				//LOG("I8275 y %u x %u DMA Underrun\n", y, x);

				m_write_drq(0);
			}

			if (!m_dma_stop)
			{
				// swap line buffers
				m_buffer_dma = !m_buffer_dma;

				if (m_scanline < (m_vrtc_scanline - SCANLINES_PER_ROW))
					dma_start();
			}
		}

		if ((m_status & ST_IE) && !(m_status & ST_IR) && m_scanline == m_irq_scanline)
		{
			//LOG("I8275 y %u x %u IRQ 1\n", y, x);
			m_status |= ST_IR;
			m_write_irq(ASSERT_LINE);
		}

		if (m_scanline == m_vrtc_scanline)
			vrtc_start();

		if (!m_dma_stop && m_scanline == m_vrtc_drq_scanline)
		{
			// swap line buffers
			m_buffer_dma = !m_buffer_dma;

			// start DMA burst
			dma_start();
		}

		if ((m_status & ST_VE) && m_scanline < m_vrtc_scanline)
		{
			bool end_of_row = false;
			bool blank_row = (UNDERLINE >= 8) && ((lc == 0) || (lc == SCANLINES_PER_ROW - 1));
			int fifo_idx = 0;
			m_field_attr = m_stored_attr;

			for (int sx = 0; sx < CHARACTERS_PER_ROW; sx++)
			{
				int lineattr = 0;

				uint8_t data = (end_of_row || m_end_of_screen) ? 0 : m_buffer[!m_buffer_dma][sx];
				uint8_t attr = m_field_attr;

				if ((data & 0xc0) == 0x80)
				{
					// field attribute code
					m_field_attr = data & (FAC_H | FAC_B | FAC_GG | FAC_R | FAC_U);

					if (!VISIBLE_FIELD_ATTRIBUTE)
					{
						attr = m_field_attr;
						data = m_fifo[!m_buffer_dma][fifo_idx];

						fifo_idx++;
						fifo_idx &= 0xf;

						if (blank_row)
							attr |= FAC_B;
						else if (!(m_char_blink < 32))
							attr &= ~FAC_B;
						if (lc != UNDERLINE)
							attr &= ~FAC_U;
					}
					else
					{
						// simply blank the attribute character itself
						attr = FAC_B;
					}
				}
				else if (data >= 0xf0 || end_of_row || m_end_of_screen)
				{
					// special control character
					switch (data)
					{
					case SCC_END_OF_ROW:
					case SCC_END_OF_ROW_DMA:
						end_of_row = true;
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

					if (lc < UNDERLINE)
					{
						ca = character_attribute[0][cccc];
					}
					else if (lc == UNDERLINE)
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
					if (blank_row)
						attr |= FAC_B;
					else if (!(m_char_blink < 32))
						attr &= ~FAC_B;
					if (lc != UNDERLINE)
						attr &= ~FAC_U;
				}

				if ((rc == m_param[REG_CUR_ROW]) && (sx == m_param[REG_CUR_COL]))
				{
					if ((CURSOR_FORMAT & 0x02) || (m_cursor_blink < 16))
					{
						if (CURSOR_FORMAT & 0x01)
							attr |= (lc == UNDERLINE) ? FAC_U : 0;
						else
							attr ^= FAC_R;
					}
				}

				if (!m_display_cb.isnull())
				m_display_cb(m_bitmap,
					sx * m_hpixels_per_column, // x position on screen of starting point
					m_scanline, // y position on screen
					line_counter, // current line of char
					(data & 0x7f),  // char code to be displayed
					lineattr,  // line attribute code
					(attr & FAC_U) ? 1 : 0,  // light enable signal
					(attr & FAC_R) ? 1 : 0,  // reverse video signal
					(attr & FAC_B) ? 1 : 0, // video suppression
					(attr & FAC_GG) >> 2,  // general purpose attribute code
					(attr & FAC_H) ? 1 : 0  // highlight
				);
			}

			if ((SCANLINES_PER_ROW - lc) == 1)
				m_stored_attr = m_field_attr;
		}

		m_scanline++;
		m_scanline %= ((CHARACTER_ROWS_PER_FRAME + VRTC_ROW_COUNT) * SCANLINES_PER_ROW);
		break;
	}
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
			int hrtc_on_pos = CHARACTERS_PER_ROW * m_hpixels_per_column;

			m_preset = false;
			m_hrtc_on_timer->adjust(screen().time_until_pos(screen().vpos(), hrtc_on_pos), 0, screen().scan_period());
			m_scanline = m_vrtc_drq_scanline;
			m_scanline_timer->adjust(screen().time_until_pos(m_vrtc_drq_scanline, 0), 0, screen().scan_period());
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
			LOG("I8275 Start Display %u %u\n", DMA_BURST_COUNT, DMA_BURST_SPACE);
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
			m_scanline_timer->adjust(attotime::never);
			m_hrtc_on_timer->adjust(attotime::never);
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

	if (!VISIBLE_FIELD_ATTRIBUTE && ((m_dma_last_char & 0xc0) == 0x80))
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
	else if (m_buffer_idx < CHARACTERS_PER_ROW)
	{
		m_buffer[m_buffer_dma][m_buffer_idx++] = data;
	}

	m_dma_idx++;

	switch (m_dma_last_char)
	{
	case SCC_END_OF_ROW_DMA:
		// stop DMA
		m_buffer_idx = CHARACTERS_PER_ROW;
		break;

	case SCC_END_OF_SCREEN_DMA:
		m_dma_stop = true;
		m_buffer_idx = CHARACTERS_PER_ROW;
		break;

	default:
		if (m_buffer_idx == CHARACTERS_PER_ROW)
		{
			// stop DMA
			m_drq_on_timer->adjust(attotime::never);
		}
		else if (!(m_dma_idx % DMA_BURST_COUNT))
		{
			m_drq_on_timer->adjust(clocks_to_attotime(DMA_BURST_SPACE));
		}
		else
		{
			m_drq_on_timer->adjust(attotime::zero);
		}
	}

	m_dma_last_char = data;
}


//-------------------------------------------------
//  lpen_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8275_device::lpen_w )
{
	if (!m_lpen && state)
	{
		m_param[REG_LPEN_COL] = screen().hpos() / m_hpixels_per_column;
		m_param[REG_LPEN_ROW] = screen().vpos() / SCANLINES_PER_ROW;

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
	int y = screen().vpos();

	int horiz_pix_total = (CHARACTERS_PER_ROW + HRTC_COUNT) * m_hpixels_per_column;
	int vert_pix_total = (CHARACTER_ROWS_PER_FRAME + VRTC_ROW_COUNT) * SCANLINES_PER_ROW;
	attotime refresh = clocks_to_attotime((CHARACTERS_PER_ROW + HRTC_COUNT) * vert_pix_total);
	int max_visible_x = (CHARACTERS_PER_ROW * m_hpixels_per_column) - 1;
	int max_visible_y = (CHARACTER_ROWS_PER_FRAME * SCANLINES_PER_ROW) - 1;

	LOG("width %u height %u max_x %u max_y %u refresh %f\n", horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, refresh.as_hz());

	rectangle visarea(0, max_visible_x, 0, max_visible_y);
	screen().configure(horiz_pix_total, vert_pix_total, visarea, (m_refresh_hack ? screen().frame_period() : refresh).as_attoseconds());

	int hrtc_on_pos = CHARACTERS_PER_ROW * m_hpixels_per_column;
	m_hrtc_on_timer->adjust(screen().time_until_pos(y, hrtc_on_pos), 0, screen().scan_period());

	m_irq_scanline = (CHARACTER_ROWS_PER_FRAME - 1) * SCANLINES_PER_ROW;
	m_vrtc_scanline = CHARACTER_ROWS_PER_FRAME * SCANLINES_PER_ROW;
	m_vrtc_drq_scanline = vert_pix_total - SCANLINES_PER_ROW;

	LOG("irq_y %u vrtc_y %u drq_y %u\n", m_irq_scanline, m_vrtc_scanline, m_vrtc_drq_scanline);

	m_scanline = y;
	m_scanline_timer->adjust(screen().time_until_pos((y + 1) % vert_pix_total, 0), 0, screen().scan_period());

	if (DOUBLE_SPACED_ROWS) fatalerror("Double spaced rows not supported!");
}
