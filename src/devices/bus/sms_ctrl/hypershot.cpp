// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Konami JE 502/JE 503 Hyper Shot emulation

    Very simple controller with two buttons.  RUN is connected to
    both right and down, and JUMP is connected to TL.  Symmetrical
    case allows use with either button to the left.

    Known versions:
    * JE 502       original Japanese version, metal case
    * JE 503       cost-reduced Japanese version, plastic case
    * JE 503-X02   cost-reduced export version, plastic case

**********************************************************************/

#include "emu.h"
#include "hypershot.h"


namespace {

INPUT_PORTS_START( sms_hypershot )
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p Run")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p Jump")
INPUT_PORTS_END



class sms_hypershot_device : public device_t, public device_sms_control_interface
{
public:
	sms_hypershot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_hypershot); }
	virtual void device_start() override { }

private:
	required_ioport m_buttons;
};


sms_hypershot_device::sms_hypershot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_HYPERSHOT, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS")
{
}


u8 sms_hypershot_device::in_r()
{
	u8 const inputs = m_buttons->read();
	return 0x25 | (BIT(inputs, 1) << 4) | (BIT(inputs, 0) ? 0x0a : 0x00);
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_HYPERSHOT, device_sms_control_interface, sms_hypershot_device, "sms_hypershot", "Konami Hyper Shot (JE 502/JE 503, Sega)")
