// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (Japanese model) emulation

**********************************************************************/

// The Japanese Sports Pad controller is only required to play the cartridge
// Sports Pad Soccer, released in Japan. It uses a different mode than the
// used by the US model, due to missing output lines on Sega Mark III
// controller ports.

#pragma once

#ifndef __SMS_SPORTS_PAD_JP__
#define __SMS_SPORTS_PAD_JP__


#include "emu.h"
#include "smsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_sports_pad_jp_device

class sms_sports_pad_jp_device : public device_t,
							public device_sms_control_port_interface
{
public:
	// construction/destruction
	sms_sports_pad_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	CUSTOM_INPUT_MEMBER( dir_pins_r );

protected:
	// device-level overrides
	virtual void device_start();

	// device_sms_control_port_interface overrides
	virtual UINT8 peripheral_r();

private:
	required_ioport m_sports_jp_in;
	required_ioport m_sports_jp_x;
	required_ioport m_sports_jp_y;

	UINT8 m_read_state;
	attotime m_start_time;
	const attotime m_interval;
};


// device type definition
extern const device_type SMS_SPORTS_PAD_JP;


#endif
