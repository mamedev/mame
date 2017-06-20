// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Furrtek's homemade multitap emulation

**********************************************************************/

#ifndef MAME_BUS_SMS_CTRL_MULTITAP_H
#define MAME_BUS_SMS_CTRL_MULTITAP_H

#pragma once


#include "smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_multitap_device

class sms_multitap_device : public device_t,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_sms_control_port_interface overrides
	virtual uint8_t peripheral_r() override;
	virtual void peripheral_w(uint8_t data) override;

private:
	DECLARE_READ32_MEMBER(pixel_r);

	required_device<sms_control_port_device> m_subctrl1_port;
	required_device<sms_control_port_device> m_subctrl2_port;
	required_device<sms_control_port_device> m_subctrl3_port;
	required_device<sms_control_port_device> m_subctrl4_port;

	uint8_t m_read_state;
	uint8_t m_last_data;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_MULTITAP, sms_multitap_device)


#endif // MAME_BUS_SMS_CTRL_MULTITAP_H
