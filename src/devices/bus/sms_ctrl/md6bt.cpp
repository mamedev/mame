// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mega Drive Six Button Control Pad emulation

    Called "Fighting Pad 6B" (ファイティングパッド6B) in Japan
    Called "6 Button Arcade Pad" in North America
    Called "Bloc d’arcade à 6 boutons" in Canada
    Called "Joystick 6 Botões" in Brazil

**********************************************************************/

#include "emu.h"
#include "md6bt.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

INPUT_PORTS_START( sms_md6button )
	PORT_START("PAD")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p B")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("%p C")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p A")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("%p Z")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("%p Y")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("%p X")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("%p Mode")
INPUT_PORTS_END



class sms_md6button_device : public device_t, public device_sms_control_interface
{
public:
	sms_md6button_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_md6button); }
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(timeout);

	required_ioport m_pad;

	emu_timer *m_idle_timer;

	u8 m_th;
	u8 m_phase;
	u8 m_mode;
};


sms_md6button_device::sms_md6button_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_MD6BUTTON, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_pad(*this, "PAD"),
	m_idle_timer(nullptr),
	m_th(1),
	m_phase(0),
	m_mode(0)
{
}


u8 sms_md6button_device::in_r()
{
	ioport_value const lines = m_pad->read();
	u8 result;
	switch (m_phase)
	{
	default:
	case 0:
	case 1:
		if (m_th)
			result = BIT(lines, 0, 6);                              // CBRLDU
		else
			result = (BIT(lines, 6, 2) << 4) | BIT(lines, 0, 2);    // SA00DU
		break;
	case 2:
		if (m_th)
			result = BIT(lines, 0, 6);                              // CBRLDU
		else
			result = BIT(lines, 6, 2) << 4;                         // SA0000
		break;
	case 3:
		if (m_th)
			result = (BIT(lines, 4, 2) << 4) | BIT(lines, 8, 4);    // CBMXYZ
		else
			result = (BIT(lines, 6, 2) << 4) | 0x0f;                // SA----
		break;
	}
	LOG("%s: TH = %u read 0x%02X\n", machine().describe_context(), m_th, result);
	return result;
}


void sms_md6button_device::out_w(u8 data, u8 mem_mask)
{
	u8 const th = BIT(data, 6);

	if (!th)
	{
		m_idle_timer->reset();
	}
	else if (m_th)
	{
		// not an edge
	}
	else if (BIT(m_mode, 0))
	{
		m_idle_timer->adjust(attotime::from_usec(1500));
		m_phase = (m_phase + 1) & 0x03;
		LOG("%s: 6-button mode, TH rising, phase = %u\n", machine().describe_context(), m_phase);
	}
	else
	{
		LOG("%s: 3-button mode, TH rising, phase = %u\n", machine().describe_context(), m_phase);
	}

	m_th = th;
}


void sms_md6button_device::device_start()
{
	m_idle_timer = timer_alloc(FUNC(sms_md6button_device::timeout), this);

	m_phase = 0;
	m_mode = 0;

	save_item(NAME(m_th));
	save_item(NAME(m_phase));
	save_item(NAME(m_mode));
}


void sms_md6button_device::device_reset()
{
	// TODO: doesn't work - reset happens before host inputs are read for the first time
	if (!m_mode)
		m_mode = BIT(m_pad->read(), 11) | 0x02;
}


TIMER_CALLBACK_MEMBER(sms_md6button_device::timeout)
{
	m_phase = 0;
	LOG("timeout, phase = %u\n", m_phase);
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_MD6BUTTON, device_sms_control_interface, sms_md6button_device, "sms_md6button", "Sega Mega Drive Six Button Control Pad")
