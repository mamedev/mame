// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Falco F5220 Keyboard

***************************************************************************/

#ifndef MAME_FALCO_F5220_KBD_H
#define MAME_FALCO_F5220_KBD_H

#pragma once

#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> f5220_kbd_device

class f5220_kbd_device :  public device_t, protected device_matrix_keyboard_interface<8>
{
public:
	// construction/destruction
	f5220_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	uint8_t read();

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

	TIMER_CALLBACK_MEMBER(reset_done);

private:
	devcb_write_line m_int_handler;

	uint8_t translate(uint8_t row, uint8_t column);
	void send_key(uint8_t code);

	emu_timer *m_reset_timer;

	uint8_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(F5220_KBD, f5220_kbd_device)

#endif // MAME_FALCO_F5220_KBD_H
