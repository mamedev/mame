// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Furrtek's homemade multitap emulation

    Schematic: http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png

    Works by pulling ground low on one of the four ports at a time.
    Doesn't pass through power, only works with passive devices.

**********************************************************************/

#include "emu.h"
#include "multitap.h"

#include "controllers.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_multitap_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override;
	virtual void out_w(uint8_t data, uint8_t mem_mask) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(reset_timeout);

	required_device_array<sms_control_port_device, 4> m_subctrl_ports;

	emu_timer *m_reset_timer;

	uint8_t m_read_state;
	uint8_t m_th_state;
};


//-------------------------------------------------
//  sms_multitap_device - constructor
//-------------------------------------------------

sms_multitap_device::sms_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_MULTITAP, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_subctrl_ports(*this, "ctrl%u", 1U),
	m_reset_timer(nullptr),
	m_read_state(0),
	m_th_state(1)
{
}


void sms_multitap_device::device_resolve_objects()
{
	m_read_state = 0;
	m_th_state = 1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_multitap_device::device_start()
{
	m_reset_timer = timer_alloc(FUNC(sms_multitap_device::reset_timeout), this);

	save_item(NAME(m_read_state));
	save_item(NAME(m_th_state));
}


//-------------------------------------------------
//  sms_peripheral_r - multitap read
//-------------------------------------------------

uint8_t sms_multitap_device::in_r()
{
	// only low two bits of count value are decoded
	uint8_t const port = m_read_state & 0x03;
	uint8_t const result = m_subctrl_ports[port]->in_r();
	LOG("%s: read player %u = 0x%02X\n", machine().describe_context(), port + 1, result);
	return result;
}


//-------------------------------------------------
//  sms_peripheral_w - multitap write
//-------------------------------------------------

void sms_multitap_device::out_w(uint8_t data, uint8_t mem_mask)
{
	uint8_t const th_state = BIT(data, 6);
	if (!th_state)
	{
		m_reset_timer->reset(); // capacitor discharged through diode
		if (m_th_state)
		{
			m_read_state = (m_read_state + 1) & 0x0f; // 74LS393 4-bit counter
			LOG("%s: TH falling, count = 0x%X (player %u)\n", machine().describe_context(), m_read_state, (m_read_state & 0x03) + 1);
		}
	}
	else if (!m_th_state)
	{
		// start charging 0.1u capacitor via 100k resistor - timeout approximated
		m_reset_timer->adjust(attotime::from_msec(10));
	}
	m_th_state = th_state;

	for (auto &port : m_subctrl_ports)
		port->out_w(data | 0x40, mem_mask & ~0x40); // TH not connected to controllers
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sms_multitap_device::device_add_mconfig(machine_config &config)
{
	// TH not connected, no point configuring screen when power isn't passed through
	for (auto port : m_subctrl_ports)
		SMS_CONTROL_PORT(config, port, sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD);
}


TIMER_CALLBACK_MEMBER(sms_multitap_device::reset_timeout)
{
	m_read_state = 0;
	LOG("Timeout, count = 0x%X (player %u)\n", m_read_state, (m_read_state & 0x03) + 1);
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_MULTITAP, device_sms_control_interface, sms_multitap_device, "sms_multitap", "Furrtek Sega Master System Multitap")
