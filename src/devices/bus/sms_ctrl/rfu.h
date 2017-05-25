// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega SG-1000/Mark-III/SMS "Rapid Fire Unit" emulation

**********************************************************************/

#ifndef MAME_BUS_SMS_CTRL_RFU_H
#define MAME_BUS_SMS_CTRL_RFU_H

#pragma once


#include "smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_rapid_fire_device

class sms_rapid_fire_device : public device_t,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_rapid_fire_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER(th_pin_w);
	DECLARE_READ32_MEMBER(pixel_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_sms_control_port_interface overrides
	virtual uint8_t peripheral_r() override;
	virtual void peripheral_w(uint8_t data) override;

private:
	required_ioport m_rfire_sw;
	required_device<sms_control_port_device> m_subctrl_port;

	uint8_t m_read_state;
	attotime m_start_time;
	const attotime m_interval;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_RAPID_FIRE, sms_rapid_fire_device)


#endif // MAME_BUS_SMS_CTRL_RFU_H
