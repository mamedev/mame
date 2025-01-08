// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100 Keyboard (HLE)

***************************************************************************/

#ifndef MAME_SKELETON_BASF7100_KBD_H
#define MAME_SKELETON_BASF7100_KBD_H

#pragma once

#include "machine/keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> basf7100_kbd_device

class basf7100_kbd_device :  public device_t, protected device_matrix_keyboard_interface<9>
{
public:
	// construction/destruction
	basf7100_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	uint8_t read();

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	required_region_ptr<uint8_t> m_translation;
	required_ioport m_modifiers;

	devcb_write_line m_int_handler;

	uint8_t translate(uint8_t row, uint8_t column);
	void send_key(uint8_t code);

	uint8_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(BASF7100_KBD, basf7100_kbd_device)

#endif // MAME_SKELETON_BASF7100_KBD_H
