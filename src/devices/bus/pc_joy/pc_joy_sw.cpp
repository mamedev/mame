// license:BSD-3-Clause
// copyright-holders:Carl
//TODO: determine when to switch modes and add single bit mode

#include "pc_joy_sw.h"

const device_type PC_MSSW_PAD = &device_creator<pc_mssw_pad_device>;

pc_mssw_pad_device::pc_mssw_pad_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	device_t(mconfig, PC_MSSW_PAD, "Microsoft Sidewinder Pad", tag, owner, clock, "mssw_pad", __FILE__),
	device_pc_joy_interface(mconfig, *this),
	m_btn1(*this, "btn1"),
	m_btn2(*this, "btn2"),
	m_btn3(*this, "btn3"),
	m_btn4(*this, "btn4"),
	m_conf(*this, "CONFIG"),
	m_timer(NULL),
	m_count(0),
	m_state(0),
	m_active(false)
{
}

void pc_mssw_pad_device::device_start()
{
	m_timer = timer_alloc();
}

void pc_mssw_pad_device::device_reset()
{
	m_count = 0;
	m_state = 0xf;
	m_active = false;
	m_timer->adjust(attotime::never, 0);
}

void pc_mssw_pad_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	UINT16 pad_state = 0;
	// only multibit mode for now
	if(m_count == -1)
	{
		reset();
		return;
	}

	if((m_count / 5) > m_conf->read())
	{
		m_timer->adjust(attotime::from_usec(50), 0);
		m_state &= ~1;
		m_count = -1;
		return;
	}

	if(m_state & 1)
	{
		m_state &= ~1;
		return;
	}

	switch(m_count / 5)
	{
		case 0:
			pad_state = m_btn1->read();
			break;
		case 1:
			pad_state = m_btn2->read();
			break;
		case 2:
			pad_state = m_btn3->read();
			break;
		case 3:
			pad_state = m_btn4->read();
			break;
	}

	switch(m_count % 5)
	{
		case 0:
			m_state = ((pad_state & 7) << 1) | 1;
			break;
		case 1:
			m_state = ((pad_state & 0x38) >> 2) | 1;
			break;
		case 2:
			m_state = ((pad_state & 0x1c0) >> 5) | 1;
			break;
		case 3:
			m_state = ((pad_state & 0xe00) >> 8) | 1;
			break;
		case 4:
		{
			UINT8 parity = (pad_state >> 8) ^ pad_state;
			parity = (parity >> 4) ^ parity;
			parity = (parity >> 2) ^ parity;
			parity = (((parity >> 1) ^ parity) & 1);

			m_state = ((pad_state & 0x3000) >> 11) | (parity << 3) | 1;
			break;
		}
	}
	m_count++;
}

static INPUT_PORTS_START( sidewinder_pad )
	PORT_START("btn1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("X")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Y")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Z")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("L")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("R")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Start")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("M")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("btn2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C") PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("X") PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Y") PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Z") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("L") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("R") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Start") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("M") PORT_PLAYER(2)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("btn3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C") PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("X") PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Y") PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Z") PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("L") PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("R") PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Start") PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("M") PORT_PLAYER(3)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("btn4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C") PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("X") PORT_PLAYER(4)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Y") PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Z") PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("L") PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("R") PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Start") PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("M") PORT_PLAYER(4)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "Number of Sidewinder Pads")
	PORT_CONFSETTING( 0x00, "1")
	PORT_CONFSETTING( 0x01, "2")
	PORT_CONFSETTING( 0x02, "3")
	PORT_CONFSETTING( 0x03, "4")
INPUT_PORTS_END

ioport_constructor pc_mssw_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sidewinder_pad );
}
