// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System - Bandai Power Pad

**********************************************************************/

#pragma once

#ifndef __NES_POWERPAD__
#define __NES_POWERPAD__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_powerpad_device

class nes_powerpad_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_powerpad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_bit34();
	virtual void write(UINT8 data);

private:
	required_ioport m_ipt1;
	required_ioport m_ipt2;
	UINT32 m_latch[2];
};


// device type definition
extern const device_type NES_POWERPAD;


#endif
