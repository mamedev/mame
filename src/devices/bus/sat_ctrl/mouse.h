// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Mouse emulation

**********************************************************************/

#pragma once

#ifndef __SATURN_MOUSE__
#define __SATURN_MOUSE__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_mouse_device

class saturn_mouse_device : public device_t,
							public device_saturn_control_port_interface
{
public:
	// construction/destruction
	saturn_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_saturn_control_port_interface overrides
	virtual UINT8 read_ctrl(UINT8 offset) override;
	virtual UINT8 read_status() override { return 0xf1; }
	virtual UINT8 read_id(int idx) override { return m_ctrl_id; }

private:
	required_ioport m_pointx;
	required_ioport m_pointy;
	required_ioport m_buttons;
};


// device type definition
extern const device_type SATURN_MOUSE;


#endif
