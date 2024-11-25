// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    K1801VP1-014 gate array (keyboard controller for the BK) and MS7008
    keyboard emulation.

***************************************************************************/

#ifndef MAME_USSR_1801VP014_H
#define MAME_USSR_1801VP014_H

#pragma once

#include "machine/keyboard.h"
#include "machine/pdp11.h"
#include "machine/z80daisy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> k1801vp014_device

class k1801vp014_device : public device_t,
					public device_matrix_keyboard_interface<12U>,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	k1801vp014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }
	auto keydown_wr_callback() { return m_write_keydown.bind(); }
	auto halt_wr_callback() { return m_write_halt.bind(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_INPUT_CHANGED_MEMBER(stop_button);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

private:
	required_ioport m_io_kbdc;
	devcb_write_line m_write_virq;
	devcb_write_line m_write_keydown;
	devcb_write_line m_write_halt;

	uint8_t m_rxrdy;

	uint16_t m_kbd_state;
	uint16_t m_key_code;
	uint16_t m_key_irq_vector;
};


// device type definition
DECLARE_DEVICE_TYPE(K1801VP014, k1801vp014_device)

#endif
