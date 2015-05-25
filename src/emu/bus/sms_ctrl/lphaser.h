// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Light Phaser" (light gun) emulation

**********************************************************************/

#pragma once

#ifndef __SMS_LIGHT_PHASER__
#define __SMS_LIGHT_PHASER__


#include "emu.h"
#include "smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_light_phaser_device

class sms_light_phaser_device : public device_t,
							public device_video_interface,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_light_phaser_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_CUSTOM_INPUT_MEMBER( th_pin_r );
	DECLARE_INPUT_CHANGED_MEMBER( position_changed );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_sms_control_port_interface overrides
	virtual UINT8 peripheral_r();

private:
	required_ioport m_lphaser_pins;
	required_ioport m_lphaser_x;
	required_ioport m_lphaser_y;

	int m_sensor_last_state;
	emu_timer *m_lphaser_timer;
	static const device_timer_id TIMER_LPHASER = 0;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	void sensor_check();
	int bright_aim_area( emu_timer *timer, int lgun_x, int lgun_y );
	UINT16 screen_hpos_nonscaled(int scaled_hpos);
	UINT16 screen_vpos_nonscaled(int scaled_vpos);
};


// device type definition
extern const device_type SMS_LIGHT_PHASER;


#endif
