// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    FM Towns Pad emulation

**********************************************************************/

#include "emu.h"
#include "townspad.h"


namespace {

INPUT_PORTS_START(fm_towns_pad)
	PORT_START("PAD")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p A")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p B")
	PORT_BIT(0x0c0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_START) PORT_NAME("%p Run")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_SELECT)
INPUT_PORTS_END

INPUT_PORTS_START(marty_pad)
	PORT_INCLUDE(fm_towns_pad)

	PORT_MODIFY("PAD")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("%p Zoom") // shoulder button
INPUT_PORTS_END


class fm_towns_pad_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	fm_towns_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;

protected:
	fm_towns_pad_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override { }
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fm_towns_pad); }

private:
	required_ioport m_pad;
};


fm_towns_pad_device::fm_towns_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: fm_towns_pad_device(mconfig, MSX_TOWNSPAD, tag, owner, clock)
{
}

fm_towns_pad_device::fm_towns_pad_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_pad(*this, "PAD")
{
}

u8 fm_towns_pad_device::read()
{
	auto pad = m_pad->read();
	if (!BIT(pad, 8)) // RUN activates RIGHT+LEFT
		pad &= ~0x0c;
	if (!BIT(pad, 9)) // SELECT activates BACK+FWD
		pad &= ~0x03;
	return u8(pad & 0xff);
}


class marty_pad_device : public fm_towns_pad_device
{
public:
	marty_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(marty_pad); }
};


marty_pad_device::marty_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: fm_towns_pad_device(mconfig, MSX_MARTYPAD, tag, owner, clock)
{
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_TOWNSPAD, device_msx_general_purpose_port_interface, fm_towns_pad_device, "msx_townspad", "FM Towns 2-button Pad")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_MARTYPAD, device_msx_general_purpose_port_interface, marty_pad_device, "msx_martypad", "FM Towns Marty Pad")
