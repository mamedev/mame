// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef MAME_SKELETON_NGEN_KB_H
#define MAME_SKELETON_NGEN_KB_H

#pragma once

#include "bus/rs232/keyboard.h"

class ngen_keyboard_device : public serial_keyboard_device
{
public:
	ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

private:
	void write(uint8_t data);

	uint8_t m_keys_down;
	uint8_t m_last_reset;
};

DECLARE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device)

#endif // MAME_SKELETON_NGEN_KB_H
