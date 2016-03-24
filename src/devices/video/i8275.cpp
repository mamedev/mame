// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8275 Programmable CRT Controller emulation

**********************************************************************/

/*

    TODO:

    - double spaced rows

*/

#include "i8275.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


const int DMA_BURST_SPACING[] = { 0, 7, 15, 23, 31, 39, 47, 55 };


#define DOUBLE_SPACED_ROWS \
	BIT(m_param[REG_SCN1], 7)

#define CHARACTERS_PER_ROW \
	((m_param[REG_SCN1] & 0x7f) + 1)

#define VRTC_ROW_COUNT \
	((m_param[REG_SCN2] >> 5) + 1)

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

// device type definition
const device_type I8275 = &device_creator<i8275_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8275_device - constructor
//-------------------------------------------------

i8275_device::i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8275, "I8275 CRTC", tag, owner, clock, "i8275x", __FILE__),
	device_video_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_drq(*this),
	m_write_hrtc(*this),
	m_write_vrtc(*this),
	m_status(0),
	m_param_idx(0),
	m_param_end(0),
	m_buffer_idx(0),
	m_fifo_next(false),
	m_buffer_dma(0),
	m_lpen(0),
	m_hlgt(0),
	m_vsp(0),
	m_gpa(0),
	m_rvv(0),
	m_lten(0),
	m_scanline(0),
	m_du(false),
	m_dma_stop(false),
	m_end_of_screen(false),
	m_cursor_blink(0),
	m_char_blink(0),
	m_stored_attr(0)
{
	memset(m_param, 0x00, sizeof(m_param));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8275_device::device_start()
{
	// get the screen device
	m_screen->register_screen_bitmap(m_bitmap);

	// resolve callbacks
	m_display_cb.bind_relative_to(*owner());
	m_write_drq.resolve_safe();
	m_write_irq.resolve_safe();
	m_write_hrtc.resolve_safe();
	m_write_vrtc.resolve_safe();

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
	save_item(NAME(m_fifo_next));
	save_item(NAME(m_buffer_dma));
	save_item(NAME(m_lpen));
	save_item(NAME(m_hlgt));
	save_item(NAME(m_vsp));
	save_item(NAME(m_gpa));
	save_item(NAME(m_rvv));
	save_item(NAME(m_lten));
	save_item(NAME(m_scanline));
	save_item(NAME(m_irq_scanline));
	save_item(NAME(m_vrtc_scanline));
	save_item(NAME(m_vrtc_drq_scanline));
	save_item(NAME(m_du));
	save_item(NAME(m_dma_stop));
	save_item(NAME(m_end_of_screen));
	save_item(NAME(m_cursor_blink));
	save_item(NAME(m_char_blink));
	save_item(NAME(m_stored_attr));
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
//  device_timer - handle timer events
//-------------------------------------------------

void i8275_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	//int y = m_screen->vpos();
	//int x = m_screen->hpos();
	int rc = m_scanline / SCANLINES_PER_ROW;
	int lc = m_scanline % SCANLINES_PER_ROW;

	switch (id)
	{
	case TIMER_HRTC_ON:
		//if (LOG) logerror("I8275 '%s' y %u x %u HRTC 1\n", tag(), y, x);
		m_write_hrtc(1);
		break;

	case TIMER_DRQ_ON:
		//if (LOG) logerror("I8275 '%s' y %u x %u DRQ 1\n", tag(), y, x);
		m_write_drq(1);
		break;

	case TIMER_SCANLINE:
		if (!(m_status & ST_VE)) break;

		//if (LOG) logerror("I8275 '%s' y %u x %u HRTC 0\n", tag(), y, x);
		m_write_hrtc(0);

		if (m_scanline == 0)
		{
			//if (LOG) logerror("I8275 '%s' y %u x %u VRTC 0\n", tag(), y, x);
			m_write_vrtc(0);
		}

		if (m_scanline <= (m_vrtc_scanline - SCANLINES_PER_ROW))
		{
			if (lc == 0)
			{
				if (m_buffer_idx < CHARACTERS_PER_ROW)
				{
					m_status |= ST_DU;
					m_du = true;

					//if (LOG) logerror("I8275 '%s' y %u x %u DMA Underrun\n", tag(), y, x);

					m_write_drq(0);
				}

				if (!m_du && !m_dma_stop)
				{
					// swap line buffers
					m_buffer_dma = !m_buffer_dma;
					m_buffer_idx = 0;
					m_fifo_idx = 0;

					if ((m_scanline < (m_vrtc_scanline - SCANLINES_PER_ROW)))
					{
						// start DMA burst
						m_drq_on_timer->adjust(clocks_to_attotime(DMA_BURST_SPACE));
					}
				}
			}
		}

		if (m_scanline == m_irq_scanline)
		{
			if (m_status & ST_IE)
			{
				//if (LOG) logerror("I8275 '%s' y %u x %u IRQ 1\n", tag(), y, x);
				m_status |= ST_IR;
				m_write_irq(ASSERT_LINE);
			}
		}

		if (m_scanline == m_vrtc_scanline)
		{
			//if (LOG) logerror("I8275 '%s' y %u x %u VRTC 1\n", tag(), y, x);
			m_write_vrtc(1);

			// reset field attributes
			m_hlgt = 0;
			m_vsp = 0;
			m_gpa = 0;
			m_rvv = 0,
			m_lten = 0;

			m_du = false;
			m_dma_stop = false;
			m_end_of_screen = false;

			m_cursor_blink++;
			m_cursor_blink &= 0x1f;

			m_char_blink++;
			m_char_blink &= 0x3f;
			m_stored_attr = 0;
		}

		if (m_scanline == m_vrtc_drq_scanline)
		{
			// swap line buffers
			m_buffer_dma = !m_buffer_dma;
			m_buffer_idx = 0;
			m_fifo_idx = 0;

			// start DMA burst
			m_drq_on_timer->adjust(clocks_to_attotime(DMA_BURST_SPACE));
		}

		if (m_scanline < m_vrtc_scanline)
		{
			int line_counter = OFFSET_LINE_COUNTER ? ((lc - 1) % SCANLINES_PER_ROW) : lc;
			bool end_of_row = false;
			int fifo_idx = 0;
			m_hlgt = (m_stored_attr & FAC_H) ? 1 : 0;
			m_vsp = (m_stored_attr & FAC_B) ? 1 : 0;
			m_gpa = (m_stored_attr & FAC_GG) >> 2;
			m_rvv = (m_stored_attr & FAC_R) ? 1 : 0;
			m_lten = ((m_stored_attr & FAC_U) != 0) && (lc == UNDERLINE) ? 1 : 0;

			for (int sx = 0; sx < CHARACTERS_PER_ROW; sx++)
			{
				int m_lineattr = 0;
				int lten = 0;
				int vsp = 0;
				int rvv = 0;

				UINT8 data = (end_of_row || m_end_of_screen) ? 0 : m_buffer[!m_buffer_dma][sx];

				if (data & 0x80)
				{
					if ((data & 0xc0) == 0x80)
					{
						// field attribute code
						m_hlgt = (data & FAC_H) ? 1 : 0;
						m_vsp = (data & FAC_B) ? 1 : 0;
						m_gpa = (data & FAC_GG) >> 2;
						m_rvv = (data & FAC_R) ? 1 : 0;
						m_lten = ((data & FAC_U) != 0) && (lc == UNDERLINE) ? 1 : 0;
						if ((SCANLINES_PER_ROW - lc)==1)
							m_stored_attr = data;

						if (!VISIBLE_FIELD_ATTRIBUTE)
						{
							data = m_fifo[!m_buffer_dma][fifo_idx];

							fifo_idx++;
							fifo_idx &= 0xf;
						}
						else
						{
							vsp = 1;
						}
					}
					else
					{
						if ((data & 0xf0) == 0xf0)
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
							//vsp = 1;
						}
						else
						{
							// character attribute code
							m_hlgt = (data & CA_H) ? 1 : 0;
							m_vsp = (data & CA_B) ? 1 : 0;

							UINT8 ca;
							int cccc = (data >> 2) & 0x0f;

							if (line_counter < UNDERLINE)
							{
								ca = character_attribute[0][cccc];
							}
							else if (line_counter == UNDERLINE)
							{
								ca = character_attribute[1][cccc];
							}
							else
							{
								ca = character_attribute[2][cccc];
							}

							m_lten = (ca & CA_LTEN) ? 1 : 0;
							m_vsp = (ca & CA_VSP) ? 1 : 0;
							m_lineattr = ca >> 2;
						}
					}
				}

				if (!vsp && m_vsp)
				{
					vsp = (m_char_blink < 32) ? 1 : 0;
				}

				if ((rc == m_param[REG_CUR_ROW]) && (sx == m_param[REG_CUR_COL]))
				{
					int vis = 1;

					if (!(CURSOR_FORMAT & 0x02))
					{
						vis = (m_cursor_blink < 16) ? 1 : 0;
					}

					if (CURSOR_FORMAT & 0x01)
					{
						lten = (lc == UNDERLINE) ? vis : 0;
					}
					else
					{
						rvv = vis;
					}
				}

				if (end_of_row || m_end_of_screen)
				{
					vsp = 1;
				}

				if (!m_display_cb.isnull())
				m_display_cb(m_bitmap,
					sx * m_hpixels_per_column, // x position on screen of starting point
					m_scanline, // y position on screen
					line_counter, // current line of char
					(data & 0x7f),  // char code to be displayed
					m_lineattr,  // line attribute code
					lten | m_lten,  // light enable signal
					rvv ^ m_rvv,  // reverse video signal
					vsp, // video suppression
					m_gpa,  // general purpose attribute code
					m_hlgt  // highlight
				);
			}
		}

		m_scanline++;
		m_scanline %= ((CHARACTER_ROWS_PER_FRAME + VRTC_ROW_COUNT) * SCANLINES_PER_ROW);
		break;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( i8275_device::read )
{
	UINT8 data;

	if (offset & 0x01)
	{
		data = m_status;

		if (m_status & ST_IR)
		{
			//if (LOG) logerror("I8275 '%s' IRQ 0\n", tag());
			m_write_irq(CLEAR_LINE);
		}

		m_status &= ~(ST_IR | ST_LP | ST_IC | ST_DU | ST_FO);
	}
	else
	{
		data = m_param[m_param_idx];
		m_param_idx++;

		if (m_param_idx > m_param_end)
		{
			m_status |= ST_IC;
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( i8275_device::write )
{
	if (offset & 0x01)
	{
		if (LOG) logerror("I8275 '%s' Command %02x\n", tag(), data);

		switch (data >> 5)
		{
		case CMD_RESET:
			if (LOG) logerror("I8275 '%s' Reset\n", tag());

			m_status &= ~ST_IE;
			if (LOG) logerror("I8275 '%s' IRQ 0\n", tag());
			m_write_irq(CLEAR_LINE);
			m_write_drq(0);

			m_param_idx = REG_SCN1;
			m_param_end = REG_SCN4;
			break;

		case CMD_START_DISPLAY:
			{
				m_param[REG_DMA] = data;
				if (LOG) logerror("I8275 '%s' Start Display %u %u\n", tag(), DMA_BURST_COUNT, DMA_BURST_SPACE);
				m_status |= (ST_IE | ST_VE);
			}
			break;

		case CMD_STOP_DISPLAY:
			if (LOG) logerror("I8275 '%s' Stop Display\n", tag());
			m_status &= ~ST_VE;
			break;

		case CMD_READ_LIGHT_PEN:
			if (LOG) logerror("I8275 '%s' Read Light Pen\n", tag());
			m_param_idx = REG_LPEN_COL;
			m_param_end = REG_LPEN_ROW;
			break;

		case CMD_LOAD_CURSOR:
			if (LOG) logerror("I8275 '%s' Load Cursor\n", tag());
			m_param_idx = REG_CUR_COL;
			m_param_end = REG_CUR_ROW;
			break;

		case CMD_ENABLE_INTERRUPT:
			if (LOG) logerror("I8275 '%s' Enable Interrupt\n", tag());
			m_status |= ST_IE;
			break;

		case CMD_DISABLE_INTERRUPT:
			if (LOG) logerror("I8275 '%s' Disable Interrupt\n", tag());
			m_status &= ~ST_IE;
			break;

		case CMD_PRESET_COUNTERS:
			if (LOG) logerror("I8275 '%s' Preset Counters\n", tag());
			m_scanline = 0;
			break;
		}
	}
	else
	{
		if (LOG) logerror("I8275 '%s' Parameter %02x\n", tag(), data);

		m_param[m_param_idx] = data;

		if (m_param_idx == REG_SCN4)
		{
			recompute_parameters();
		}

		m_param_idx++;
	}
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

WRITE8_MEMBER( i8275_device::dack_w )
{
	//if (LOG) logerror("I8275 '%s' y %u x %u DACK %04x:%02x %u\n", tag(), m_screen->vpos(), m_screen->hpos(), offset, data, m_buffer_idx);

	m_write_drq(0);

	if (m_fifo_next)
	{
		if (m_fifo_idx == 16)
		{
			m_fifo_idx = 0;
			m_status |= ST_FO;
		}

		m_fifo[m_buffer_dma][m_fifo_idx++] = data;

		m_fifo_next = false;
	}
	else
	{
		assert(m_buffer_idx >= 0 && m_buffer_idx < ARRAY_LENGTH(m_buffer[m_buffer_dma]));
		m_buffer[m_buffer_dma][m_buffer_idx++] = data;

		if (!VISIBLE_FIELD_ATTRIBUTE && ((data & 0xc0) == 0x80))
		{
			m_fifo_next = true;
		}

		switch (data)
		{
		case SCC_END_OF_ROW_DMA:
			// stop DMA
			// TODO should read one more character if DMA burst not completed
			break;

		case SCC_END_OF_SCREEN_DMA:
			m_dma_stop = true;
			// TODO should read one more character if DMA burst not completed
			break;

		default:
			if (m_buffer_idx == CHARACTERS_PER_ROW)
			{
				// stop DMA
			}
			else if (!(m_buffer_idx % DMA_BURST_COUNT))
			{
				m_drq_on_timer->adjust(clocks_to_attotime(DMA_BURST_SPACE));
			}
			else
			{
				m_drq_on_timer->adjust(attotime::zero);
			}
		}

	}
}


//-------------------------------------------------
//  lpen_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8275_device::lpen_w )
{
	if (!m_lpen && state)
	{
		m_param[REG_LPEN_COL] = m_screen->hpos() / m_hpixels_per_column;
		m_param[REG_LPEN_ROW] = m_screen->vpos() / SCANLINES_PER_ROW;

		m_status |= ST_LP;
	}

	m_lpen = state;
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 i8275_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!(m_status & ST_VE))
	{
		m_bitmap.fill(rgb_t::black);
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

void i8275_device::recompute_parameters()
{
	int y = m_screen->vpos();

	int horiz_pix_total = (CHARACTERS_PER_ROW + HRTC_COUNT) * m_hpixels_per_column;
	int vert_pix_total = (CHARACTER_ROWS_PER_FRAME + VRTC_ROW_COUNT) * SCANLINES_PER_ROW;
	attoseconds_t refresh = m_screen->frame_period().attoseconds();
	int max_visible_x = (CHARACTERS_PER_ROW * m_hpixels_per_column) - 1;
	int max_visible_y = (CHARACTER_ROWS_PER_FRAME * SCANLINES_PER_ROW) - 1;

	if (LOG) logerror("width %u height %u max_x %u max_y %u refresh %f\n", horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

	rectangle visarea;
	visarea.set(0, max_visible_x, 0, max_visible_y);
	m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

	int hrtc_on_pos = CHARACTERS_PER_ROW * m_hpixels_per_column;
	m_hrtc_on_timer->adjust(m_screen->time_until_pos(y, hrtc_on_pos), 0, m_screen->scan_period());

	m_irq_scanline = (CHARACTER_ROWS_PER_FRAME - 1) * SCANLINES_PER_ROW;
	m_vrtc_scanline = CHARACTER_ROWS_PER_FRAME * SCANLINES_PER_ROW;
	m_vrtc_drq_scanline = vert_pix_total - SCANLINES_PER_ROW;

	if (LOG) logerror("irq_y %u vrtc_y %u drq_y %u\n", m_irq_scanline, m_vrtc_scanline, m_vrtc_drq_scanline);

	m_scanline_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->scan_period());

	if (DOUBLE_SPACED_ROWS) fatalerror("Double spaced rows not supported!");
}
