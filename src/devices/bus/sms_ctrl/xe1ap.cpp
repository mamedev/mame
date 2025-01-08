// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Dempa Micom Soft Analog/Digital Controller XE-1AP emulation

**********************************************************************/

#include "emu.h"
#include "xe1ap.h"

#include "machine/micomxe1a.h"


namespace {

INPUT_PORTS_START( sms_xe1ap )
	PORT_START("BUTTONS")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("%p D")  // left shoulder lower
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("%p C")  // left shoulder upper
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p B")  // right shoulder lower
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p A")  // right shoulder upper
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_SELECT)                     // left face bottom
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_START)                      // right face bottom
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("%p E2") // left face top inner
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("%p E1") // left face top outer
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("%p B'") // right face top inner
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("%p A'") // right face top outer
	PORT_BIT(0xfc00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CH0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("CH1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("CH2")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Z) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("%p Throttle")

	PORT_START("CH3")
	PORT_BIT(0xff, 0x00, IPT_UNUSED)

	PORT_START("MODE")
	PORT_CONFNAME(0x01, 0x01, "Mode") PORT_WRITE_LINE_DEVICE_MEMBER("xe1", FUNC(micom_xe_1a_device::mode_w))
	PORT_CONFSETTING(   0x00, "Digital")
	PORT_CONFSETTING(   0x01, "Analog")
	PORT_CONFNAME(0x02, 0x02, "Interface") PORT_WRITE_LINE_DEVICE_MEMBER("xe1", FUNC(micom_xe_1a_device::interface_w))
	PORT_CONFSETTING(   0x00, "Personal Computer")
	PORT_CONFSETTING(   0x02, "MD")
INPUT_PORTS_END



class sms_xe1ap_device : public device_t, public device_sms_control_interface
{
public:
	sms_xe1ap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) ATTR_COLD;

	virtual u8 in_r() override { return m_xe1->out_r(); }
	virtual void out_w(u8 data, u8 mem_mask) override { m_xe1->req_w(BIT(data, 6)); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD { return INPUT_PORTS_NAME(sms_xe1ap); }
	virtual void device_start() override ATTR_COLD { }
	virtual void device_reset() override ATTR_COLD;

private:
	u8 read_channel(offs_t offset) { return m_axes[offset]->read(); }

	required_device<micom_xe_1a_device> m_xe1;
	required_ioport_array<4> m_axes;
	required_ioport m_mode;
};



sms_xe1ap_device::sms_xe1ap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_XE1AP, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_xe1(*this, "xe1"),
	m_axes(*this, "CH%u", 0U),
	m_mode(*this, "MODE")
{
}


void sms_xe1ap_device::device_add_mconfig(machine_config &config)
{
	MICOM_XE_1A(config, m_xe1);
	m_xe1->buttons_handler().set_ioport("BUTTONS");
	m_xe1->analog_handler().set(FUNC(sms_xe1ap_device::read_channel));
}


void sms_xe1ap_device::device_reset()
{
	auto const mode = m_mode->read();
	m_xe1->mode_w(BIT(mode, 0));
	m_xe1->interface_w(BIT(mode, 1));
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_XE1AP, device_sms_control_interface, sms_xe1ap_device, "sms_xe1ap", "Dempa Micom Soft Analog Controller (XE-1AP, Sega)")
