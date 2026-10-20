// license:BSD-3-Clause
// copyright-holders:Carl,Vas Crabb
#ifndef MAME_OLIVETTI_M20_KBD_H
#define MAME_OLIVETTI_M20_KBD_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"
#include "sound/beep.h"

class m20_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<9U>
{
public:
	m20_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// Share one keyboard implementation across the language-labelled slot types.
	m20_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(uint8_t row, uint8_t column) override;

private:
	virtual void received_byte(uint8_t byte) override;

	TIMER_CALLBACK_MEMBER(bell_off);

	required_ioport m_modifiers;
	required_ioport m_layout;
	required_device<beep_device> m_beeper;
	emu_timer *m_bell_timer;
};

// Separate device types give -listslots meaningful names; only the defaults differ.
class m20_keyboard_de_device : public m20_keyboard_device
{
public:
	m20_keyboard_de_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m20_keyboard_de_original_device : public m20_keyboard_device
{
public:
	m20_keyboard_de_original_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m20_keyboard_us_device : public m20_keyboard_device
{
public:
	m20_keyboard_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(M20_KEYBOARD, m20_keyboard_device)
DECLARE_DEVICE_TYPE(M20_KEYBOARD_DE, m20_keyboard_de_device)
DECLARE_DEVICE_TYPE(M20_KEYBOARD_DE_ORIGINAL, m20_keyboard_de_original_device)
DECLARE_DEVICE_TYPE(M20_KEYBOARD_US, m20_keyboard_us_device)

#endif // MAME_OLIVETTI_M20_KBD_H
