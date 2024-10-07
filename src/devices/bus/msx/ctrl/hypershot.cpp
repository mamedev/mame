// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Konami JE 502/JE 503 Hyper Shot emulation

    Very simple controller with two buttons.  RUN is connected to
    both right and down, and JUMP is connected to TL.  Symmetrical
    case allows use with either button to the left.  Pin 8 is used
    as the common pin so it will work with Sega and Atari consoles.
    It will work with MSX computers provided pin 8 is pulled low.

    Known versions:
    * JE 502       original Japanese version, metal case
    * JE 503       cost-reduced Japanese version, plastic case
    * JE 503-X02   cost-reduced export version, plastic case

**********************************************************************/

#include "emu.h"
#include "hypershot.h"


namespace {

INPUT_PORTS_START(hypershot)
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p Run")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p Jump")
INPUT_PORTS_END


class hypershot_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	hypershot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(hypershot); }

private:
	required_ioport m_buttons;

	u8 m_pin8_in;
};

hypershot_device::hypershot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_HYPERSHOT, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_buttons(*this, "BUTTONS")
	, m_pin8_in(1)
{
}

u8 hypershot_device::read()
{
	if (m_pin8_in)
	{
		return 0xff;
	}
	else
	{
		u8 const inputs = m_buttons->read();
		return 0xe5 | (BIT(inputs, 1) << 4) | (BIT(inputs, 0) ? 0x0a : 0x00);
	}
}

void hypershot_device::pin_8_w(int state)
{
	m_pin8_in = state ? 1 : 0;
}

void hypershot_device::device_start()
{
	save_item(NAME(m_pin8_in));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_HYPERSHOT, device_msx_general_purpose_port_interface, hypershot_device, "msx_hypershot", "Konami Hyper Shot (JE 502/JE 503, MSX)")
