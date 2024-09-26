// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    PSI HLE ASCII Keyboard

***************************************************************************/

#ifndef MAME_BUS_PSI_KBD_HLE_H
#define MAME_BUS_PSI_KBD_HLE_H

#pragma once

#include "psi_kbd.h"
#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psi_hle_keyboard_device

class psi_hle_keyboard_device : public device_t,
								public device_psi_keyboard_interface,
								protected device_matrix_keyboard_interface<7>
{
public:
	// construction/destruction
	psi_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	uint8_t translate(uint8_t row, uint8_t column);
	void send_key(uint8_t code);
	required_ioport m_modifiers;
};


// device type definition
DECLARE_DEVICE_TYPE(PSI_HLE_KEYBOARD, psi_hle_keyboard_device)


#endif // MAME_BUS_PSI_KBD_HLE_H
