// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Micomsoft Libble Rabble Joypad (XPD-1LR) emulation

    Bundled with the following games:
    - Crazy Climber Video Game Anthology Vol.5 (Sharp X68000)
    - Libble Rabble (Sharp X68000)
    - Libble Rabble (Fujitsu FM Towns)

**********************************************************************/

#include "emu.h"
#include "libbler.h"


namespace {

INPUT_PORTS_START(xpd_1lr)
	PORT_START("PAD1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT) PORT_8WAY
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PAD2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT) PORT_8WAY
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BUTTONS")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p A") // used to add credits
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p B") // used to pause
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


class xpd_1lr_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	xpd_1lr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override { return m_pad[m_pin8]->read() | m_buttons->read(); }
	virtual void pin_8_w(int state) override { m_pin8 = state ? 1 : 0; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(xpd_1lr); }

private:
	required_ioport_array<2> m_pad;
	required_ioport m_buttons;
	u8 m_pin8;
};


xpd_1lr_device::xpd_1lr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_LIBBLERPAD, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_pad(*this, "PAD%u", 1U)
	, m_buttons(*this, "BUTTONS")
	, m_pin8(0)
{
}

void xpd_1lr_device::device_start()
{
	save_item(NAME(m_pin8));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_LIBBLERPAD, device_msx_general_purpose_port_interface, xpd_1lr_device, "msx_libblerpad", "Micomsoft Libble Rabble Joypad (XPD-1LR)")
