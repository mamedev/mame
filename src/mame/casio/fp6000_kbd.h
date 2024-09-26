// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio FP-6000 Keyboard

***************************************************************************/

#ifndef MAME_CASIO_FP6000_KBD_H
#define MAME_CASIO_FP6000_KBD_H

#pragma once

#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fp6000_kbd_device

class fp6000_kbd_device :  public device_t, protected device_matrix_keyboard_interface<6>
{
public:
	// construction/destruction
	fp6000_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	devcb_write_line m_int_handler;

	enum
	{
		STATUS_READY_FOR_DATA = 0x01,
		STATUS_DATA_AVAILABLE = 0x02
	};

	uint8_t translate(uint8_t row, uint8_t column);
	void send_key(uint8_t code);

	uint8_t m_status;
	uint8_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(FP6000_KBD, fp6000_kbd_device)

#endif // MAME_CASIO_FP6000_KBD_H
