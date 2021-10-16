// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
	Casio GT913 keyboard controller (HLE)
***************************************************************************/

#ifndef MAME_MACHINE_GT913_KBD_H
#define MAME_MACHINE_GT913_KBD_H

#pragma once

#include "cpu/h8/h8_intc.h"
#include "keyboard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_kbd_hle_device

class gt913_kbd_hle_device : public device_t, protected device_matrix_keyboard_interface<13>
{
public:
	// construction/destruction
	gt913_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	gt913_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int irq)
		: gt913_kbd_hle_device(mconfig, tag, owner, 0)
	{
		m_intc_tag = intc;
		m_irq = irq;
	}

	uint16_t read();
	void status_w(uint16_t data);
	uint16_t status_r() { return m_status; }

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override  { key_add(row, column, 0); }
	virtual void key_break(uint8_t row, uint8_t column) override { key_add(row, column, 1); }

	void key_add(uint8_t row, uint8_t column, int state);
	void update_status();
private:
	h8_intc_device *m_intc;
	const char *m_intc_tag;
	int m_irq;
	optional_ioport m_velocity;

	uint16_t m_status;
	uint8_t m_fifo[16];
	uint8_t m_fifo_read, m_fifo_write;
};

// device type definition
DECLARE_DEVICE_TYPE(GT913_KBD_HLE, gt913_kbd_hle_device)

#endif // MAME_MACHINE_GT913_KBD_H
