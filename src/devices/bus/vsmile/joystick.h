// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_VSMILE_JOYSTICK_H
#define MAME_BUS_VSMILE_JOYSTICK_H

#pragma once

#include "vsmile_ctrl.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> vsmile_joystick_device

class vsmile_joystick_device : public device_t, public device_vsmile_ctrl_interface
{
public:
	// construction/destruction
	vsmile_joystick_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~vsmile_joystick_device();

protected:
	// device_t implementation
	virtual void device_start() override;

	// device_vsmile_ctrl_interface implementation
	virtual void cts_w(int state) override;
	virtual void data_w(uint8_t data) override;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_JOYSTICK, vsmile_joystick_device)

#endif // MAME_BUS_VSMILE_JOYSTICK_H
