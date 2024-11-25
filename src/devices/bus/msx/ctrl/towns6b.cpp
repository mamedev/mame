// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    FM Towns 6-button Pad emulation

**********************************************************************/

#include "emu.h"
#include "towns6b.h"


namespace {

INPUT_PORTS_START(fm_towns_6b)
	PORT_START("PAD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("%p Run")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SELECT)

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("%p Z")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("%p Y")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("%p X")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("%p C")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p A")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p B")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


class fm_towns_6b_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	fm_towns_6b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_8_w(int state) override { m_pin8 = state; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fm_towns_6b); }

private:
	required_ioport m_pad;
	required_ioport m_buttons;
	u8 m_pin8;
};


fm_towns_6b_device::fm_towns_6b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_TOWNS6B, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_pad(*this, "PAD")
	, m_buttons(*this, "BUTTONS")
	, m_pin8(0)
{
}

u8 fm_towns_6b_device::read()
{
	const u8 buttons = m_buttons->read();
	if (m_pin8)
	{
		return buttons;
	}
	else
	{
		u8 pad = m_pad->read();
		if (!BIT(pad, 6)) // RUN activates RIGHT+LEFT
			pad &= 0xf3;
		if (!BIT(pad, 7)) // SELECT activates BACK+FWD
			pad &= 0xfc;
		return (buttons & 0xf0) | (pad & 0x0f);
	}
}

void fm_towns_6b_device::device_start()
{
	save_item(NAME(m_pin8));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_TOWNS6B, device_msx_general_purpose_port_interface, fm_towns_6b_device, "msx_towns6b", "FM Towns 6-button Pad")
