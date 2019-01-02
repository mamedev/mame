// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Mark III "Paddle Control" emulation

**********************************************************************/

#ifndef MAME_BUS_SMS_CTRL_PADDLE_H
#define MAME_BUS_SMS_CTRL_PADDLE_H

#pragma once


#include "smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_paddle_device

class sms_paddle_device : public device_t,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_paddle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_CUSTOM_INPUT_MEMBER( rldu_pins_r ); // Right, Left, Down and Up lines.
	DECLARE_READ_LINE_MEMBER( tr_pin_r );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_sms_control_port_interface overrides
	virtual uint8_t peripheral_r() override;

private:
	required_ioport m_paddle_pins;
	required_ioport m_paddle_x;

	uint8_t m_read_state;
	attotime m_start_time;
	const attotime m_interval;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_PADDLE, sms_paddle_device)

#endif // MAME_BUS_SMS_CTRL_PADDLE_H
