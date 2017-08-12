// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    PSI HLE ASCII Keyboard

***************************************************************************/

#ifndef MAME_BUS_PSI_KBD_HLE_H
#define MAME_BUS_PSI_KBD_HLE_H

#pragma once

#include "psi_kbd.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psi_hle_keyboard_device

class psi_hle_keyboard_device : public device_t, public device_psi_keyboard_interface
{
public:
	// construction/destruction
	psi_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	void kbd_put(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(PSI_HLE_KEYBOARD, psi_hle_keyboard_device)


#endif // MAME_BUS_PSI_KBD_HLE_H
