// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline Keyboard (HLE)

***************************************************************************/

#ifndef MAME_REGNECENTRALEN_RC759_KBD_H
#define MAME_REGNECENTRALEN_RC759_KBD_H

#pragma once

#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rc759_kbd_hle_device

class rc759_kbd_hle_device : public device_t, protected device_matrix_keyboard_interface<7>
{
public:
	// construction/destruction
	rc759_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	uint8_t read();
	void enable_w(int state);

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
	devcb_write_line m_int_handler;
	uint8_t m_data;
	bool m_enabled;
};

// device type definition
DECLARE_DEVICE_TYPE(RC759_KBD_HLE, rc759_kbd_hle_device)

#endif // MAME_REGNECENTRALEN_RC759_KBD_H
