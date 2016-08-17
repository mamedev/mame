// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega SG-1000/Mark-III/SMS "Rapid Fire Unit" emulation

**********************************************************************/

#pragma once

#ifndef __SMS_RAPID_FIRE__
#define __SMS_RAPID_FIRE__


#include "emu.h"
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
	sms_rapid_fire_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER(th_pin_w);
	DECLARE_READ32_MEMBER(pixel_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_sms_control_port_interface overrides
	virtual UINT8 peripheral_r() override;
	virtual void peripheral_w(UINT8 data) override;

private:
	required_ioport m_rfire_sw;
	required_device<sms_control_port_device> m_subctrl_port;

	UINT8 m_read_state;
	attotime m_start_time;
	const attotime m_interval;
};


// device type definition
extern const device_type SMS_RAPID_FIRE;


#endif
