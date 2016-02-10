// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (US model) emulation

**********************************************************************/

#pragma once

#ifndef __SMS_SPORTS_PAD__
#define __SMS_SPORTS_PAD__


#include "emu.h"
#include "smsctrl.h"



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
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_CUSTOM_INPUT_MEMBER( dir_pins_r );
	DECLARE_CUSTOM_INPUT_MEMBER( th_pin_r );
	DECLARE_INPUT_CHANGED_MEMBER( th_pin_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_sms_control_port_interface overrides
	virtual UINT8 peripheral_r() override;
	virtual void peripheral_w(UINT8 data) override;

private:
	required_ioport m_sports_in;
	required_ioport m_sports_out;
	required_ioport m_sports_x;
	required_ioport m_sports_y;

	UINT8 m_read_state;
	UINT8 m_last_data;
	UINT8 m_x_axis_reset_value;
	UINT8 m_y_axis_reset_value;
	const attotime m_interval;
	emu_timer *m_sportspad_timer;
	static const device_timer_id TIMER_SPORTSPAD = 0;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


// device type definition
extern const device_type SMS_SPORTS_PAD;


#endif
