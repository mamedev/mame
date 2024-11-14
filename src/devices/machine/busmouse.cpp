// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Logitech bus mouse interface emulation

    References:
    - ec1841 technical manual
    - https://github.com/OBattler/86Box/blob/master/src/mouse_bus.c
    - https://communities.intel.com/docs/DOC-22714
    - http://toastytech.com/guis/msmouse.html

    To do:
    - selectable IRQ level
    - Microsoft protocol
    - ec1841.0003 clone: diag mode
    - ec1841.0003 clone: fix detection by m86v32 driver

**********************************************************************/

#include "emu.h"
#include "busmouse.h"

#include "machine/i8255.h"


//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BUS_MOUSE, bus_mouse_device, "bus_mouse", "Bus Mouse Interface")

void bus_mouse_device::map(address_map &map)
{
	map(0x0, 0x3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_x_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER(bus_mouse_device::mouse_x_changed)
{
	m_x += newval - oldval;
	LOG("m_x_c: irqdis %d; %d = %d - %d\n", irq_disabled, m_x, newval, oldval);
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_y_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER(bus_mouse_device::mouse_y_changed)
{
	m_y += newval - oldval;
	LOG("m_y_c: irqdis %d; %d = %d - %d\n", irq_disabled, m_y, newval, oldval);
}


//-------------------------------------------------
//  INPUT_PORTS( bus_mouse )
//-------------------------------------------------

INPUT_PORTS_START( bus_mouse )
	PORT_START("mouse_x")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bus_mouse_device::mouse_x_changed), 0)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bus_mouse_device::mouse_y_changed), 0)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Middle Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("irq_rate")
	PORT_DIPNAME(0x0f, 0x02, "IRQ rate")
	PORT_DIPSETTING(0x01, "15 hz")
	PORT_DIPSETTING(0x02, "30 hz")
	PORT_DIPSETTING(0x04, "60 hz")
	PORT_DIPSETTING(0x08, "120 hz")

	PORT_START("irq_line")
	PORT_DIPNAME(0x0f, 0x02, "IRQ line")
	PORT_DIPSETTING(0x02, "IRQ4")
#if 0
	PORT_DIPSETTING(0x08, "IRQ2")
	PORT_DIPSETTING(0x04, "IRQ3")
	PORT_DIPSETTING(0x01, "IRQ5")
#endif
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bus_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bus_mouse);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bus_mouse_device - constructor
//-------------------------------------------------

bus_mouse_device::bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BUS_MOUSE, tag, owner, clock)
	, m_write_extint(*this)
	, m_buttons(*this, "mouse_buttons")
{
}


void bus_mouse_device::device_add_mconfig(machine_config &config)
{
	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set(FUNC(bus_mouse_device::ppi_a_r));
	ppi.out_pc_callback().set(FUNC(bus_mouse_device::ppi_c_w));
	ppi.in_pc_callback().set(FUNC(bus_mouse_device::ppi_c_r));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bus_mouse_device::device_start()
{
	m_irq_timer = timer_alloc(FUNC(bus_mouse_device::irq_timer_tick), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bus_mouse_device::device_reset()
{
	int hz = 2 * 15 * ioport("irq_rate")->read();

	m_irq_timer->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));

	irq = 0;
	irq_disabled = 1;
	irq_line = ioport("irq_line")->read();

	LOG("irq rate: %d Hz\n", hz);
}

TIMER_CALLBACK_MEMBER(bus_mouse_device::irq_timer_tick)
{
	irq = !irq;

	if (!irq_disabled && irq)
	{
		m_write_extint(irq);
	}
}

uint8_t bus_mouse_device::ppi_a_r()
{
	return m_pa;
}

uint8_t bus_mouse_device::ppi_c_r()
{
	return irq ? irq_line : 0;
}

void bus_mouse_device::ppi_c_w(uint8_t data)
{
	irq_disabled = BIT(data, 4);

	switch (data & 0xe0)
	{
	case 0:
		m_write_extint(CLEAR_LINE);
		m_pa = 0;
		break;

	case 0x80: // LSB X
		m_pa = m_x & 0xf;
		break;

	case 0xa0: // MSB X
		m_pa = (m_x >> 4) & 0xf;
		if (m_x < 0) m_pa |= 8;
		m_pa |= m_buttons->read() & 0xe0;
		m_x = 0;
		break;

	case 0xc0: // LSB Y
		m_pa = m_y & 0xf;
		break;

	case 0xe0: // MSB Y
		m_pa = (m_y >> 4) & 0xf;
		if (m_y < 0) m_pa |= 8;
		m_pa |= m_buttons->read() & 0xe0;
		m_y = 0;
		break;
	}
	LOG("c_w: data %02x m_pa %02x\n", data, m_pa);
}
