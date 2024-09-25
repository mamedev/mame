// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_ASTROCDE_JOY_H
#define MAME_BUS_ASTROCDE_JOY_H

#pragma once

#include "ctrl.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> astrocade_joy_device

class astrocade_joy_device : public device_t,
							 public device_astrocade_ctrl_interface
{
public:
	// construction/destruction
	astrocade_joy_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~astrocade_joy_device();

	// device_astrocade_ctrl_interface implementation
	virtual uint8_t read_handle() override;
	virtual uint8_t read_knob() override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override { }

private:
	required_ioport m_handle;
	required_ioport m_knob;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(ASTROCADE_JOY, astrocade_joy_device)

#endif // MAME_BUS_ASTROCDE_JOY_H
