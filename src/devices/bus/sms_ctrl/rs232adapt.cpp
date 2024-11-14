// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Mega Drive RS-232 Adapter emulation

**********************************************************************/

#include "emu.h"
#include "rs232adapt.h"

#include "bus/rs232/rs232.h"


namespace {

class sms_rs232_device : public device_t, public device_sms_control_interface
{
public:
	sms_rs232_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<rs232_port_device> m_port;
};


sms_rs232_device::sms_rs232_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_RS232, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_port(*this, "com")
{
}


void sms_rs232_device::out_w(u8 data, u8 mem_mask)
{
	m_port->write_txd(BIT(data, 4));
}


void sms_rs232_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_port, default_rs232_devices, nullptr);
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_RS232, device_sms_control_interface, sms_rs232_device, "sms_rs232", "Mega Drive RS-232 Adapter")
