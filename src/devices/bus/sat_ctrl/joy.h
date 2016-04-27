// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Joypad emulation

**********************************************************************/

#pragma once

#ifndef __SATURN_JOY__
#define __SATURN_JOY__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_joy_device

class saturn_joy_device : public device_t,
							public device_saturn_control_port_interface
{
public:
	// construction/destruction
	saturn_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_saturn_control_port_interface overrides
	virtual UINT16 read_direct() override;
	virtual UINT8 read_ctrl(UINT8 offset) override;
	virtual UINT8 read_status() override { return 0xf1; }
	virtual UINT8 read_id(int idx) override { return m_ctrl_id; }

private:
	required_ioport m_joy;
};


// device type definition
extern const device_type SATURN_JOY;


#endif
