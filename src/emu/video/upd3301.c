// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    NEC uPD3301 Programmable CRT Controller emulation

**********************************************************************/

/*

    TODO:

    - attributes
    - N interrupt
    - light pen
    - reset counters
    - proper DMA timing (now the whole screen is transferred at the end of the frame,
        accurate timing requires CCLK timer which kills performance)

*/

#include "upd3301.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define COMMAND_MASK                    0xe0
#define COMMAND_RESET                   0x00
#define COMMAND_START_DISPLAY           0x20
#define COMMAND_SET_INTERRUPT_MASK      0x40
#define COMMAND_READ_LIGHT_PEN          0x60    // not supported
#define COMMAND_LOAD_CURSOR_POSITION    0x80
#define COMMAND_RESET_INTERRUPT         0xa0
#define COMMAND_RESET_COUNTERS          0xc0    // not supported


#define STATUS_VE                       0x10
#define STATUS_U                        0x08    // not supported
#define STATUS_N                        0x04    // not supported
#define STATUS_E                        0x02
#define STATUS_LP                       0x01    // not supported


enum
{
	MODE_NONE,
	MODE_RESET,
	MODE_READ_LIGHT_PEN,
	MODE_LOAD_CURSOR_POSITION,
	MODE_RESET_COUNTERS
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type UPD3301 = &device_creator<upd3301_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd3301_device - constructor
//-------------------------------------------------

upd3301_device::upd3301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, UPD3301, "UPD3301", tag, owner, clock, "upd3301", __FILE__),
	device_video_interface(mconfig, *this),
	m_write_int(*this),
	m_write_drq(*this),
	m_write_hrtc(*this),
	m_write_vrtc(*this),
	m_width(0),
	m_status(0),
	m_param_count(0),
	m_data_fifo_pos(0),
	m_attr_fifo_pos(0),
	m_input_fifo(0),
	m_me(0),
	m_h(80),
	m_l(20),
	m_r(10),
	m_v(6),
	m_z(32),
	m_attr_blink(0),
	m_attr_frame(0),
	m_cm(0),
	m_cx(0),
	m_cy(0),
	m_cursor_blink(0),
	m_cursor_frame(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd3301_device::device_start()
{
	// resolve callbacks
	m_display_cb.bind_relative_to(*owner());
	m_write_drq.resolve_safe();
	m_write_int.resolve_safe();
	m_write_hrtc.resolve_safe();
	m_write_vrtc.resolve_safe();

	// allocate timers
	m_hrtc_timer = timer_alloc(TIMER_HRTC);
	m_vrtc_timer = timer_alloc(TIMER_VRTC);
	m_drq_timer = timer_alloc(TIMER_DRQ);

	// state saving
	save_item(NAME(m_y));
	save_item(NAME(m_hrtc));
	save_item(NAME(m_vrtc));
	save_item(NAME(m_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_param_count));
	save_item(NAME(m_data_fifo_pos));
	save_item(NAME(m_attr_fifo_pos));
	save_item(NAME(m_input_fifo));
	save_item(NAME(m_mn));
	save_item(NAME(m_me));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_h));
	save_item(NAME(m_b));
	save_item(NAME(m_l));
	save_item(NAME(m_s));
	save_item(NAME(m_c));
	save_item(NAME(m_r));
	save_item(NAME(m_v));
	save_item(NAME(m_z));
	save_item(NAME(m_at1));
	save_item(NAME(m_at0));
	save_item(NAME(m_sc));
	save_item(NAME(m_attr));
	save_item(NAME(m_attr_blink));
	save_item(NAME(m_attr_frame));
	save_item(NAME(m_cm));
	save_item(NAME(m_cx));
	save_item(NAME(m_cy));
	save_item(NAME(m_cursor_blink));
	save_item(NAME(m_cursor_frame));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd3301_device::device_reset()
{
	set_interrupt(0);
	set_drq(0);

	recompute_parameters();
}


//-------------------------------------------------
//  device_clock_changed - handle clock change
//-------------------------------------------------

void upd3301_device::device_clock_changed()
{
	recompute_parameters();
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void upd3301_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_HRTC:
		if (LOG) logerror("UPD3301 '%s' HRTC: %u\n", tag(), param);

		m_write_hrtc(param);
		m_hrtc = param;

		update_hrtc_timer(param);
		break;

	case TIMER_VRTC:
		if (LOG) logerror("UPD3301 '%s' VRTC: %u\n", tag(), param);

		m_write_vrtc(param);
		m_vrtc = param;

		if (param && !m_me)
		{
			m_status |= STATUS_E;
			set_interrupt(1);
		}

		update_vrtc_timer(param);
		break;

	case TIMER_DRQ:
		break;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( upd3301_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x01)
	{
	case 0: // data
		break;

	case 1: // status
		data = m_status;
		m_status &= ~(STATUS_LP | STATUS_E |STATUS_N | STATUS_U);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( upd3301_device::write )
{
	switch (offset & 0x01)
	{
	case 0: // data
		switch (m_mode)
		{
		case MODE_RESET:
			switch (m_param_count)
			{
			case 0:
				m_dma_mode = BIT(data, 7);
				m_h = (data & 0x7f) + 2;
				if (LOG) logerror("UPD3301 '%s' DMA Mode: %s\n", tag(), m_dma_mode ? "character" : "burst");
				if (LOG) logerror("UPD3301 '%s' H: %u\n", tag(), m_h);
				break;

			case 1:
				m_b = ((data >> 6) + 1) * 16;
				m_l = (data & 0x3f) + 1;
				if (LOG) logerror("UPD3301 '%s' B: %u\n", tag(), m_b);
				if (LOG) logerror("UPD3301 '%s' L: %u\n", tag(), m_l);
				break;

			case 2:
				m_s = BIT(data, 7);
				m_c = (data >> 4) & 0x03;
				m_r = (data & 0x1f) + 1;
				if (LOG) logerror("UPD3301 '%s' S: %u\n", tag(), m_s);
				if (LOG) logerror("UPD3301 '%s' C: %u\n", tag(), m_c);
				if (LOG) logerror("UPD3301 '%s' R: %u\n", tag(), m_r);
				break;

			case 3:
				m_v = (data >> 5) + 1;
				m_z = (data & 0x1f) + 2;
				if (LOG) logerror("UPD3301 '%s' V: %u\n", tag(), m_v);
				if (LOG) logerror("UPD3301 '%s' Z: %u\n", tag(), m_z);
				recompute_parameters();
				break;

			case 4:
				m_at1 = BIT(data, 7);
				m_at0 = BIT(data, 6);
				m_sc = BIT(data, 5);
				m_attr = (data & 0x1f) + 1;
				if (LOG) logerror("UPD3301 '%s' AT1: %u\n", tag(), m_at1);
				if (LOG) logerror("UPD3301 '%s' AT0: %u\n", tag(), m_at0);
				if (LOG) logerror("UPD3301 '%s' SC: %u\n", tag(), m_sc);
				if (LOG) logerror("UPD3301 '%s' ATTR: %u\n", tag(), m_attr);

				m_mode = MODE_NONE;
				break;
			}

			m_param_count++;
			break;

		case MODE_LOAD_CURSOR_POSITION:
			switch (m_param_count)
			{
			case 0:
				m_cx = data & 0x7f;
				if (LOG) logerror("UPD3301 '%s' CX: %u\n", tag(), m_cx);
				break;

			case 1:
				m_cy = data & 0x3f;
				if (LOG) logerror("UPD3301 '%s' CY: %u\n", tag(), m_cy);

				m_mode = MODE_NONE;
				break;
			}

			m_param_count++;
			break;

		default:
			if (LOG) logerror("UPD3301 '%s' Invalid Parameter Byte %02x!\n", tag(), data);
		}
		break;

	case 1: // command
		m_mode = MODE_NONE;
		m_param_count = 0;

		switch (data & 0xe0)
		{
		case COMMAND_RESET:
			if (LOG) logerror("UPD3301 '%s' Reset\n", tag());
			m_mode = MODE_RESET;
			set_display(0);
			set_interrupt(0);
			break;

		case COMMAND_START_DISPLAY:
			if (LOG) logerror("UPD3301 '%s' Start Display\n", tag());
			set_display(1);
			reset_counters();
			break;

		case COMMAND_SET_INTERRUPT_MASK:
			if (LOG) logerror("UPD3301 '%s' Set Interrupt Mask\n", tag());
			m_me = BIT(data, 0);
			m_mn = BIT(data, 1);
			if (LOG) logerror("UPD3301 '%s' ME: %u\n", tag(), m_me);
			if (LOG) logerror("UPD3301 '%s' MN: %u\n", tag(), m_mn);
			break;

		case COMMAND_READ_LIGHT_PEN:
			if (LOG) logerror("UPD3301 '%s' Read Light Pen\n", tag());
			m_mode = MODE_READ_LIGHT_PEN;
			break;

		case COMMAND_LOAD_CURSOR_POSITION:
			if (LOG) logerror("UPD3301 '%s' Load Cursor Position\n", tag());
			m_mode = MODE_LOAD_CURSOR_POSITION;
			m_cm = BIT(data, 0);
			if (LOG) logerror("UPD3301 '%s' CM: %u\n", tag(), m_cm);
			break;

		case COMMAND_RESET_INTERRUPT:
			if (LOG) logerror("UPD3301 '%s' Reset Interrupt\n", tag());
			set_interrupt(0);
			break;

		case COMMAND_RESET_COUNTERS:
			if (LOG) logerror("UPD3301 '%s' Reset Counters\n", tag());
			m_mode = MODE_RESET_COUNTERS;
			reset_counters();
			break;
		}
		break;
	}
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

WRITE8_MEMBER( upd3301_device::dack_w )
{
	if (m_y >= (m_l * m_r))
	{
		return;
	}

	if (m_data_fifo_pos < m_h)
	{
		m_data_fifo[m_data_fifo_pos][m_input_fifo] = data;
		m_data_fifo_pos++;
	}
	else
	{
		m_attr_fifo[m_attr_fifo_pos][m_input_fifo] = data;
		m_attr_fifo_pos++;
	}

	if ((m_data_fifo_pos == m_h) && (m_attr_fifo_pos == (m_attr << 1)))
	{
		m_input_fifo = !m_input_fifo;

		m_data_fifo_pos = 0;
		m_attr_fifo_pos = 0;

		draw_scanline();

		if (m_y == (m_l * m_r))
		{
			// end DMA transfer
			set_drq(0);
		}
	}
}


//-------------------------------------------------
//  lpen_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd3301_device::lpen_w )
{
}


//-------------------------------------------------
//  hrtc_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd3301_device::hrtc_r )
{
	return m_hrtc;
}


//-------------------------------------------------
//  vrtc_r -
//-------------------------------------------------

READ_LINE_MEMBER( upd3301_device::vrtc_r )
{
	return m_vrtc;
}


//-------------------------------------------------
//  draw_scanline -
//-------------------------------------------------

void upd3301_device::draw_scanline()
{
	for (int lc = 0; lc < m_r; lc++)
	{
		for (int sx = 0; sx < m_h; sx++)
		{
			int y = m_y + lc;
			UINT8 cc = m_data_fifo[sx][!m_input_fifo];
			int hlgt = 0; // TODO
			int rvv = 0; // TODO
			int vsp = 0; // TODO
			int sl0 = 0; // TODO
			int sl12 = 0; // TODO
			int csr = m_cm && m_cursor_blink && ((y / m_r) == m_cy) && (sx == m_cx);
			int gpa = 0; // TODO

			m_display_cb(*m_bitmap, y, sx, cc, lc, hlgt, rvv, vsp, sl0, sl12, csr, gpa);
		}
	}

	m_y += m_r;
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 upd3301_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_status & STATUS_VE)
	{
		m_y = 0;
		m_bitmap = &bitmap;
		m_data_fifo_pos = 0;
		m_attr_fifo_pos = 0;

		m_cursor_frame++;

		if (m_cursor_frame == m_b)
		{
			m_cursor_frame = 0;
			m_cursor_blink = !m_cursor_blink;
		}

		m_attr_frame++;

		if (m_attr_frame == (m_b << 1))
		{
			m_attr_frame = 0;
			m_attr_blink = !m_attr_blink;
		}

		// start DMA transfer
		set_drq(1);
	}
	else
	{
		bitmap.fill(rgb_t(0x00,0x00,0x00), cliprect);
	}
	return 0;
}


//-------------------------------------------------
//  set_interrupt -
//-------------------------------------------------

void upd3301_device::set_interrupt(int state)
{
	if (LOG) logerror("UPD3301 '%s' Interrupt: %u\n", tag(), state);

	m_write_int(state);

	if (!state)
	{
		m_status &= ~(STATUS_N | STATUS_E);
	}
}


//-------------------------------------------------
//  set_drq -
//-------------------------------------------------

void upd3301_device::set_drq(int state)
{
	if (LOG) logerror("UPD3301 '%s' DRQ: %u\n", tag(), state);

	m_write_drq(state);
}


//-------------------------------------------------
//  set_display -
//-------------------------------------------------

void upd3301_device::set_display(int state)
{
	if (state)
	{
		m_status |= STATUS_VE;
	}
	else
	{
		m_status &= ~STATUS_VE;
	}
}


//-------------------------------------------------
//  reset_counters -
//-------------------------------------------------

void upd3301_device::reset_counters()
{
	set_interrupt(0);
	set_drq(0);
}


//-------------------------------------------------
//  update_hrtc_timer -
//-------------------------------------------------

void upd3301_device::update_hrtc_timer(int state)
{
	int y = m_screen->vpos();

	int next_x = state ? m_h : 0;
	int next_y = state ? y : ((y + 1) % ((m_l + m_v) * m_width));

	attotime duration = m_screen->time_until_pos(next_y, next_x);

	m_hrtc_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_vrtc_timer -
//-------------------------------------------------

void upd3301_device::update_vrtc_timer(int state)
{
	int next_y = state ? (m_l * m_r) : 0;

	attotime duration = m_screen->time_until_pos(next_y, 0);

	m_vrtc_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

void upd3301_device::recompute_parameters()
{
	int horiz_pix_total = (m_h + m_z) * m_width;
	int vert_pix_total = (m_l + m_v) * m_r;

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock()) * horiz_pix_total * vert_pix_total;

	rectangle visarea;

	visarea.set(0, (m_h * m_width) - 1, 0, (m_l * m_r) - 1);

	if (LOG)
	{
		if (LOG) logerror("UPD3301 '%s' Screen: %u x %u @ %f Hz\n", tag(), horiz_pix_total, vert_pix_total, 1 / ATTOSECONDS_TO_DOUBLE(refresh));
		if (LOG) logerror("UPD3301 '%s' Visible Area: (%u, %u) - (%u, %u)\n", tag(), visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);
	}

	m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

	update_hrtc_timer(0);
	update_vrtc_timer(0);
}
