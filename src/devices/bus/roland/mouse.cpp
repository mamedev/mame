// license:BSD-3-Clause
// copyright-holders:Carl Lom
/**********************************************************************

    Roland MU-1 mouse emulation

**********************************************************************/

#include "emu.h"
#include "mouse.h"


namespace {

INPUT_PORTS_START(roland_ext_mouse)
	PORT_START("BUTTONS")
	// MAME's UI consumes right-click for its own menu, so map the mouse's
	// right button to IPT_BUTTON3 (middle-click) which passes through.
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Left Button")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Right Button")
	PORT_BIT(0xcf, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOUSE_X")
	PORT_BIT(0xffff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(50)

	PORT_START("MOUSE_Y")
	PORT_BIT(0xffff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(50)
INPUT_PORTS_END


class roland_ext_mouse_device : public device_t, public device_roland_ext_port_interface
{
public:
	roland_ext_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void write(u8 data, u8 mem_mask) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(roland_ext_mouse); }

private:
	required_ioport m_buttons;
	required_ioport m_port_mouse_x;
	required_ioport m_port_mouse_y;
	u16 m_data;
	u8 m_stat;
	u8 m_old_strobe;
	s16 m_mouse_x;
	s16 m_mouse_y;
	attotime m_last_strobe;
};

roland_ext_mouse_device::roland_ext_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ROLAND_EXT_MOUSE, tag, owner, clock)
	, device_roland_ext_port_interface(mconfig, *this)
	, m_buttons(*this, "BUTTONS")
	, m_port_mouse_x(*this, "MOUSE_X")
	, m_port_mouse_y(*this, "MOUSE_Y")
{
}

void roland_ext_mouse_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_stat));
	save_item(NAME(m_old_strobe));
	save_item(NAME(m_last_strobe));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
}

void roland_ext_mouse_device::device_reset()
{
	m_data = 0;
	m_stat = 3;
	m_old_strobe = 0;
	m_mouse_x = 0;
	m_mouse_y = 0;
	m_last_strobe = attotime::zero;
}

// Bits 0-3: movement nybble (cycles through X_hi, X_lo, Y_hi, Y_lo on strobe edges)
// Bits 4-5: mouse buttons (active low: bit4=left, bit5=right)
u8 roland_ext_mouse_device::read()
{
	return (m_buttons->read() & 0x30) | ((m_data >> (4 * (3 - m_stat))) & 0x0f);
}

// Bit 6 drives the mouse strobe pin. Each edge (rising or falling) advances the
// nybble state machine: 0->1->2->3->0. State 0 latches the current mouse X/Y deltas.
// A 3ms gap between edges resets the state machine, matching MSX mouse protocol.
void roland_ext_mouse_device::write(u8 data, u8 mem_mask)
{
	u8 strobe = BIT(data, 6);
	if (strobe != m_old_strobe)
	{
		attotime now = machine().time();
		if (now - m_last_strobe > attotime::from_msec(3))
			m_stat = 3; // timeout — force restart

		m_last_strobe = now;
		m_stat = (m_stat + 1) & 0x03;

		if (m_stat == 0)
		{
			// Latch mouse deltas (signed 8-bit X and Y)
			s16 mouse_x = m_port_mouse_x->read();
			s16 mouse_y = m_port_mouse_y->read();
			m_data = (u8(m_mouse_x - mouse_x) << 8) | u8(m_mouse_y - mouse_y);
			m_mouse_x = mouse_x;
			m_mouse_y = mouse_y;
		}
		m_old_strobe = strobe;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ROLAND_EXT_MOUSE, device_roland_ext_port_interface, roland_ext_mouse_device, "roland_ext_mouse", "Roland MU-1 Mouse")
