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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("%p Run")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SELECT)
INPUT_PORTS_END


class fm_towns_pad_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	fm_towns_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;

protected:
	virtual void device_start() override { }
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fm_towns_pad); }

private:
	required_ioport m_pad;
};


fm_towns_pad_device::fm_towns_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_TOWNSPAD, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_pad(*this, "PAD")
{
}

u8 fm_towns_pad_device::read()
{
	u8 pad = m_pad->read();
	if (!BIT(pad, 6)) // RUN activates RIGHT+LEFT
		pad &= 0xf3;
	if (!BIT(pad, 7)) // SELECT activates BACK+FWD
		pad &= 0xfc;
	return pad | 0xc0;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_TOWNSPAD, device_msx_general_purpose_port_interface, fm_towns_pad_device, "msx_townspad", "FM Towns 2-button Pad")
