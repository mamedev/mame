/**********************************************************************

    Nintendo Entertainment System - Miracle Piano Keyboard

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __NES_MIRACLE__
#define __NES_MIRACLE__


#include "emu.h"
#include "ctrl.h"
//#include "cpu/mcs51/mcs51.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_miracle_device

class nes_miracle_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_miracle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_bit0();
	virtual void write(UINT8 data);

	static const device_timer_id TIMER_STROBE_ON = 0;
	emu_timer *strobe_timer;

	//required_device<i8051_device> m_cpu;
	int m_strobe_on, m_midi_mode, m_sent_bits;
	UINT32 m_strobe_clock;
};

// device type definition
extern const device_type NES_MIRACLE;

#endif
