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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(uint8_t row, uint8_t column) override;

private:
	virtual void received_byte(uint8_t byte) override;

	TIMER_CALLBACK_MEMBER(bell_off);

	required_ioport m_modifiers;
	required_device<beep_device> m_beeper;
	emu_timer *m_bell_timer;
};

DECLARE_DEVICE_TYPE(M20_KEYBOARD, m20_keyboard_device)

#endif // MAME_OLIVETTI_M20_KBD_H
