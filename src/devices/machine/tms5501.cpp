// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TMS5501 Multifunction Input/Output Controller emulation

**********************************************************************/

#include "emu.h"
#include "tms5501.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


const uint8_t tms5501_device::rst_vector[] = { 0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TMS5501, tms5501_device, "tms5501", "TMS5501 Multifunction I/O")


// I/O address map
void tms5501_device::io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(tms5501_device::rb_r));
	map(0x01, 0x01).r(FUNC(tms5501_device::xi_r));
	map(0x02, 0x02).r(FUNC(tms5501_device::rst_r));
	map(0x03, 0x03).r(FUNC(tms5501_device::sta_r));
	map(0x04, 0x04).w(FUNC(tms5501_device::cmd_w));
	map(0x05, 0x05).w(FUNC(tms5501_device::rr_w));
	map(0x06, 0x06).w(FUNC(tms5501_device::tb_w));
	map(0x07, 0x07).w(FUNC(tms5501_device::xo_w));
	map(0x08, 0x08).w(FUNC(tms5501_device::mr_w));
	map(0x09, 0x0d).w(FUNC(tms5501_device::tmr_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms5501_device - constructor
//-------------------------------------------------

tms5501_device::tms5501_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TMS5501, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_write_int(*this),
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
	m_write_int.resolve_safe();
	m_write_xmt.resolve_safe();
	m_read_xi.resolve_safe(0);
	m_write_xo.resolve_safe();

	// create timers
	m_timer[TIMER_1] = timer_alloc(FUNC(tms5501_device::timer_expired), this);
	m_timer[TIMER_2] = timer_alloc(FUNC(tms5501_device::timer_expired), this);
	m_timer[TIMER_3] = timer_alloc(FUNC(tms5501_device::timer_expired), this);
	m_timer[TIMER_4] = timer_alloc(FUNC(tms5501_device::timer_expired), this);
	m_timer[TIMER_5] = timer_alloc(FUNC(tms5501_device::timer_expired), this);

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
//  timer_expired -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(tms5501_device::timer_expired)
{
	if (param == TIMER_5)
	{
		if (!(m_cmd & CMD_XI7))
		{
			set_interrupt(IRQ_TMR5);
		}
	}
	else
	{
		static uint8_t const s_irq_ids[4] = { IRQ_TMR1, IRQ_TMR2, IRQ_TMR3, IRQ_TMR4 };
		set_interrupt(s_irq_ids[param]);
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

uint8_t tms5501_device::rb_r()
{
	m_sta &= ~STA_RBL;
	m_irq &= ~IRQ_RB;

	check_interrupt();

	return m_rb;
}


//-------------------------------------------------
//  xi_r - read external inputs
//-------------------------------------------------

uint8_t tms5501_device::xi_r()
{
	uint8_t data = m_read_xi(0);

	if (m_cmd & CMD_XI7)
	{
		data = (m_xi7 << 7) | (data & 0x7f);
	}

	return data;
}


//-------------------------------------------------
//  rst_r - read interrupt address
//-------------------------------------------------

uint8_t tms5501_device::rst_r()
{
	return get_vector();
}


//-------------------------------------------------
//  sta_r - read TMS5510 status
//-------------------------------------------------

uint8_t tms5501_device::sta_r()
{
	if(is_transmit_register_empty())
		m_sta |= STA_XBE;

	uint8_t data = m_sta;

	m_sta &= ~STA_OE;

	return data;
}


//-------------------------------------------------
//  cmd_w - issue discrete commands
//-------------------------------------------------

void tms5501_device::cmd_w(uint8_t data)
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

void tms5501_device::rr_w(uint8_t data)
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

void tms5501_device::tb_w(uint8_t data)
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

void tms5501_device::xo_w(uint8_t data)
{
	if (LOG) logerror("TMS5501 '%s' Output %02x\n", tag(), data);

	m_write_xo(data);
}


//-------------------------------------------------
//  mr_w - load mask register
//-------------------------------------------------

void tms5501_device::mr_w(uint8_t data)
{
	if (LOG) logerror("TMS5501 '%s' Mask Register %02x\n", tag(), data);

	m_mr = data;

	check_interrupt();
}


//-------------------------------------------------
//  tmr_w - load interval timer
//-------------------------------------------------

void tms5501_device::tmr_w(offs_t offset, uint8_t data)
{
	if (LOG) logerror("TMS5501 '%s' Timer %u %02x\n", tag(), offset, data);

	m_timer[offset]->adjust(attotime::from_double((double) data / (clock() / 128.0)), (int)offset);
}


//-------------------------------------------------
//  rcv_w - receive data write
//-------------------------------------------------

void tms5501_device::rcv_w(int state)
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

void tms5501_device::xi7_w(int state)
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

void tms5501_device::sens_w(int state)
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

void tms5501_device::set_interrupt(uint8_t mask)
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
		m_write_int(state);
	}
	else
	{
		m_write_int(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  get_vector -
//-------------------------------------------------

uint8_t tms5501_device::get_vector()
{
	uint8_t rst = 0;

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
