// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mega Drive Control Pad emulation

**********************************************************************/

#include "emu.h"
#include "mdpad.h"


namespace {

INPUT_PORTS_START( sms_mdpad )
	PORT_START("PAD")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("%p C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )
INPUT_PORTS_END



class sms_mdpad_device : public device_t, public device_sms_control_interface
{
public:
	sms_mdpad_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_mdpad); }
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_pad;

	u8 m_th;
};


sms_mdpad_device::sms_mdpad_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_MDPAD, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_pad(*this, "PAD"),
	m_th(1)
{
}


u8 sms_mdpad_device::in_r()
{
	ioport_value const lines = m_pad->read();
	if (m_th)
		return BIT(lines, 0, 6);
	else
		return (BIT(lines, 6, 2) << 4) | BIT(lines, 0, 2);
}


void sms_mdpad_device::out_w(u8 data, u8 mem_mask)
{
	m_th = BIT(data, 6);
}


void sms_mdpad_device::device_start()
{
	save_item(NAME(m_th));
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_MDPAD, device_sms_control_interface, sms_mdpad_device, "sms_mdpad", "Sega Mega Drive Control Pad")
