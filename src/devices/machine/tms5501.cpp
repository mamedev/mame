// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TMS5501 Multifunction Input/Output Controller emulation

**********************************************************************/

#include "tms5501.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


const UINT8 tms5501_device::rst_vector[] = { 0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type TMS5501 = &device_creator<tms5501_device>;


// I/O address map
DEVICE_ADDRESS_MAP_START( io_map, 8, tms5501_device )
	AM_RANGE(0x00, 0x00) AM_READ(rb_r)
	AM_RANGE(0x01, 0x01) AM_READ(xi_r)
	AM_RANGE(0x02, 0x02) AM_READ(rst_r)
	AM_RANGE(0x03, 0x03) AM_READ(sta_r)
	AM_RANGE(0x04, 0x04) AM_WRITE(cmd_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(rr_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(tb_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(xo_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(mr_w)
	AM_RANGE(0x09, 0x0d) AM_WRITE(tmr_w)
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms5501_device - constructor
//-------------------------------------------------

tms5501_device::tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, TMS5501, "TMS5501", tag, owner, clock, "tms5501", __FILE__),
	device_serial_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_xmt(*this),
	m_read_xi(*this),
	m_write_xo(*this),
	m_irq(IRQ_TB),
	m_rb(0),
	m_sta(STA_XBE | STA_SR),
	m_cmd(0),
	m_rr(0),
	m_tb(0),
	m_mr(0),
	m_sens(0),
	m_xi7(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5501_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_xmt.resolve_safe();
	m_read_xi.resolve_safe(0);
	m_write_xo.resolve_safe();

	// create timers
	m_timer[TIMER_1] = timer_alloc(TIMER_1);
	m_timer[TIMER_2] = timer_alloc(TIMER_2);
	m_timer[TIMER_3] = timer_alloc(TIMER_3);
	m_timer[TIMER_4] = timer_alloc(TIMER_4);
	m_timer[TIMER_5] = timer_alloc(TIMER_5);

	// state saving
	save_item(NAME(m_rb));
	save_item(NAME(m_sta));
	save_item(NAME(m_cmd));
	save_item(NAME(m_rr));
	save_item(NAME(m_tb));
	save_item(NAME(m_mr));
	save_item(NAME(m_sens));
	save_item(NAME(m_xi7));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms5501_device::device_reset()
{
	receive_register_reset();
	transmit_register_reset();

	m_write_xmt(1);

	check_interrupt();
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void tms5501_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_1:
		set_interrupt(IRQ_TMR1);
		break;

	case TIMER_2:
		set_interrupt(IRQ_TMR2);
		break;

	case TIMER_3:
		set_interrupt(IRQ_TMR3);
		break;

	case TIMER_4:
		set_interrupt(IRQ_TMR4);
		break;

	case TIMER_5:
		if (!(m_cmd & CMD_XI7))
		{
			set_interrupt(IRQ_TMR5);
		}
		break;

	default:
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void tms5501_device::tra_callback()
{
	m_write_xmt(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void tms5501_device::tra_complete()
{
	if (!(m_sta & STA_XBE))
	{
		transmit_register_setup(m_tb);

		m_sta |= STA_XBE;

		set_interrupt(IRQ_TB);
	}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void tms5501_device::rcv_complete()
{
	receive_register_extract();
	m_rb = get_received_char();

	if (is_receive_framing_error())
	{
		m_sta |= STA_FE;
	}
	else
	{
		m_sta &= ~STA_FE;
	}

	if (m_sta & STA_RBL)
	{
		m_sta |= STA_OE;
	}

	m_sta |= (STA_RBL | STA_SR);
	m_sta &= ~(STA_SBD | STA_FBD);

	set_interrupt(IRQ_RB);
}


//-------------------------------------------------
//  rb_r - read receiver buffer
//-------------------------------------------------

READ8_MEMBER( tms5501_device::rb_r )
{
	m_sta &= ~STA_RBL;
	m_irq &= ~IRQ_RB;

	check_interrupt();

	return m_rb;
}


//-------------------------------------------------
//  xi_r - read external inputs
//-------------------------------------------------

READ8_MEMBER( tms5501_device::xi_r )
{
	UINT8 data = m_read_xi(0);

	if (m_cmd & CMD_XI7)
	{
		data = (m_xi7 << 7) | (data & 0x7f);
	}

	return data;
}


//-------------------------------------------------
//  rst_r - read interrupt address
//-------------------------------------------------

READ8_MEMBER( tms5501_device::rst_r )
{
	return get_vector();
}


//-------------------------------------------------
//  sta_r - read TMS5510 status
//-------------------------------------------------

READ8_MEMBER( tms5501_device::sta_r )
{
	UINT8 data = m_sta;

	m_sta &= ~STA_OE;

	return data;
}


//-------------------------------------------------
//  cmd_w - issue discrete commands
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::cmd_w )
{
	if (LOG) logerror("TMS5501 '%s' Command %02x\n", tag(), data);

	m_cmd = data;

	if (m_cmd & CMD_RST)
	{
		m_sta &= ~(STA_SBD | STA_FBD | STA_RBL | STA_OE);
		m_sta |= (STA_XBE | STA_SR);

		receive_register_reset();
		transmit_register_reset();

		m_write_xmt(1);

		m_irq = 0;
		set_interrupt(IRQ_TB);

		m_timer[TIMER_1]->enable(false);
		m_timer[TIMER_2]->enable(false);
		m_timer[TIMER_3]->enable(false);
		m_timer[TIMER_4]->enable(false);
		m_timer[TIMER_5]->enable(false);
	}
	else if (m_cmd & CMD_BRK)
	{
		receive_register_reset();
		transmit_register_reset();

		m_write_xmt(0);
	}
}


//-------------------------------------------------
//  rr_w - load rate register
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::rr_w )
{
	if (LOG) logerror("TMS5501 '%s' Rate Register %02x\n", tag(), data);

	m_rr = data;

	stop_bits_t stop_bits = (m_rr & RR_STOP) ? STOP_BITS_1 : STOP_BITS_2;

	set_data_frame(1, 8, PARITY_NONE, stop_bits);

	int rate = 0;

	if (m_rr & RR_9600) rate = 9600;
	else if (m_rr & RR_4800) rate = 4800;
	else if (m_rr & RR_2400) rate = 2400;
	else if (m_rr & RR_1200) rate = 1200;
	else if (m_rr & RR_300) rate = 300;
	else if (m_rr & RR_150) rate = 150;
	else if (m_rr & RR_110) rate = 110;

	if (m_cmd & CMD_TST1)
	{
		rate *= 8;
	}

	set_rcv_rate(rate);
	set_tra_rate(rate);
}


//-------------------------------------------------
//  tb_w - load transmitter buffer
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::tb_w )
{
	if (LOG) logerror("TMS5501 '%s' Transmitter Buffer %02x\n", tag(), data);

	m_tb = data;

	if (is_transmit_register_empty())
	{
		transmit_register_setup(m_tb);

		m_sta |= STA_XBE;

		set_interrupt(IRQ_TB);
	}
	else
	{
		m_sta &= ~STA_XBE;
	}
}


//-------------------------------------------------
//  xo_w - load output port
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::xo_w )
{
	if (LOG) logerror("TMS5501 '%s' Output %02x\n", tag(), data);

	m_write_xo(data);
}


//-------------------------------------------------
//  mr_w - load mask register
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::mr_w )
{
	if (LOG) logerror("TMS5501 '%s' Mask Register %02x\n", tag(), data);

	m_mr = data;

	check_interrupt();
}


//-------------------------------------------------
//  tmr_w - load interval timer
//-------------------------------------------------

WRITE8_MEMBER( tms5501_device::tmr_w )
{
	if (LOG) logerror("TMS5501 '%s' Timer %u %02x\n", tag(), offset, data);

	m_timer[offset]->adjust(attotime::from_double((double) data / (clock() / 128.0)));
}


//-------------------------------------------------
//  rcv_w - receive data write
//-------------------------------------------------

WRITE_LINE_MEMBER( tms5501_device::rcv_w )
{
	device_serial_interface::rx_w(state);

	if (is_receive_register_synchronized())
	{
		m_sta |= STA_SBD;
		m_sta &= ~STA_SR;
	}

	if (is_receive_register_shifting())
	{
		m_sta |= STA_FBD;
	}
}


//-------------------------------------------------
//  xi7_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( tms5501_device::xi7_w )
{
	if (m_cmd & CMD_XI7)
	{
		if (!m_xi7 && state)
		{
			set_interrupt(IRQ_XI7);
		}
	}

	m_xi7 = state;
}


//-------------------------------------------------
//  sens_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( tms5501_device::sens_w )
{
	if (!m_sens && state)
	{
		set_interrupt(IRQ_SENS);
	}

	m_sens = state;
}


//-------------------------------------------------
//  set_interrupt -
//-------------------------------------------------

void tms5501_device::set_interrupt(UINT8 mask)
{
	m_irq |= mask;

	if (LOG) logerror("TMS5501 '%s' Interrupt %02x\n", tag(), mask);

	check_interrupt();
}


//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

void tms5501_device::check_interrupt()
{
	int state = (m_irq & m_mr) ? ASSERT_LINE : CLEAR_LINE;

	if (state == ASSERT_LINE)
	{
		if (LOG) logerror("TMS5501 '%s' Interrupt Assert\n", tag());

		m_sta |= STA_IP;
	}
	else
	{
		m_sta &= ~STA_IP;
	}

	if (m_cmd & CMD_IAE)
	{
		m_write_irq(state);
	}
	else
	{
		m_write_irq(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  get_vector -
//-------------------------------------------------

UINT8 tms5501_device::get_vector()
{
	UINT8 rst = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT((m_irq & m_mr), i))
		{
			rst = rst_vector[i];
			m_irq &= ~(1 << i);

			check_interrupt();

			if (LOG) logerror("%s: TMS5501 '%s' Interrupt Acknowledge %02x\n", machine().describe_context(), tag(), rst);
			break;
		}
	}

	return rst;
}
