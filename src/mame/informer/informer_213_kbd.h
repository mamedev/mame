// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 213 Keyboard (HLE)

***************************************************************************/

#ifndef MAME_INFORMER_INFORMER_213_KBD_H
#define MAME_INFORMER_INFORMER_213_KBD_H

#pragma once

#include "machine/keyboard.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> informer_213_kbd_hle_device

class informer_213_kbd_hle_device : public device_t,
									protected device_matrix_keyboard_interface<9>
{
public:
	// construction/destruction
	informer_213_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	// from host
	uint8_t read();
	void write(uint8_t data);

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
	uint8_t m_key;
	uint8_t m_mod;
};

// device type definition
DECLARE_DEVICE_TYPE(INFORMER_213_KBD_HLE, informer_213_kbd_hle_device)

#endif // MAME_INFORMER_INFORMER_213_KBD_H
