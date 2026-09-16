// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 keyboard controller (HLE)
***************************************************************************/

#ifndef MAME_MACHINE_GT913_KBD_H
#define MAME_MACHINE_GT913_KBD_H

#pragma once

#include "keyboard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_kbd_hle_device

class gt913_kbd_hle_device : public device_t, protected device_matrix_keyboard_interface<14>
{
public:
	// construction/destruction
	gt913_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	uint16_t read();
	void status_w(uint16_t data);
	uint16_t status_r() { return m_status; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override  { key_add(row, column, 0); }
	virtual void key_break(uint8_t row, uint8_t column) override { key_add(row, column, 1); }

	void key_add(uint8_t row, uint8_t column, int state);
	void update_status();
private:
	optional_ioport m_velocity;

	devcb_write_line m_irq_cb;

	uint16_t m_status;
	uint8_t m_fifo[16];
	uint8_t m_fifo_read, m_fifo_write;
};

// device type definition
DECLARE_DEVICE_TYPE(GT913_KBD_HLE, gt913_kbd_hle_device)

#endif // MAME_MACHINE_GT913_KBD_H
