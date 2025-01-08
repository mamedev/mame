// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 207/376 Keyboard (HLE)

***************************************************************************/

#ifndef MAME_INFORMER_INFORMER_207_376_KBD_H
#define MAME_INFORMER_INFORMER_207_376_KBD_H

#pragma once

#include "machine/keyboard.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> informer_207_376_kbd_hle_device

class informer_207_376_kbd_hle_device : public device_t,
										public device_buffered_serial_interface<16>,
										protected device_matrix_keyboard_interface<8>
{
public:
	// construction/destruction
	informer_207_376_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto tx_handler() { return m_tx_handler.bind(); }

	// from host
	void rx_w(int state);

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void received_byte(uint8_t byte) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	devcb_write_line m_tx_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(INFORMER_207_376_KBD_HLE, informer_207_376_kbd_hle_device)

#endif // MAME_INFORMER_INFORMER_207_376_KBD_H
