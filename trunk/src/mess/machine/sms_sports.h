/**********************************************************************

    Sega Master System "Sports Pad" emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SMS_SPORTS_PAD__
#define __SMS_SPORTS_PAD__


#include "emu.h"
#include "machine/smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_sports_pad_device

class sms_sports_pad_device : public device_t,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_sports_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	CUSTOM_INPUT_MEMBER( dir_pins_r );
	CUSTOM_INPUT_MEMBER( th_pin_r );
	INPUT_CHANGED_MEMBER( th_pin_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_sms_control_port_interface overrides
	virtual UINT8 peripheral_r();
	virtual void peripheral_w(UINT8 data);

private:
	required_ioport m_sports_in;
	required_ioport m_sports_out;
	required_ioport m_sports_x;
	required_ioport m_sports_y;

	UINT8 m_sports_pad_state;
	UINT8 m_sports_pad_last_data;
	const attotime m_sports_pad_interval;
	attotime m_sports_pad_last_time;
};


// device type definition
extern const device_type SMS_SPORTS_PAD;


#endif
