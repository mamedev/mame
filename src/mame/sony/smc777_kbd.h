// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_SONY_SMC777_KBD_H
#define MAME_SONY_SMC777_KBD_H

#pragma once

#include "machine/keyboard.h"

class smc777_kbd_device : public device_t
						, protected device_matrix_keyboard_interface<10>
{
public:
	smc777_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 data_r(offs_t offset);
	void data_w(offs_t offset, u8 data);
	u8 status_r(offs_t offset);
	void control_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	uint8_t translate(uint8_t row, uint8_t column);

private:
	required_ioport m_key_mod;

	void scan_mode(u8 data);
	u8 m_command;
	u8 m_status, m_aux_hs;
	u8 m_scan_code;
	u16 m_repeat_interval;
	u16 m_repeat_start;
	s8 m_held_keys;
	u8 m_fkey_table[3][6];
	bool m_fkey_dir;
	u8 m_fkey_target;
	u8 m_fkey_index;

	emu_timer *m_aux_timer;
	TIMER_CALLBACK_MEMBER(aux_ready);
};

DECLARE_DEVICE_TYPE(SMC777_KBD, smc777_kbd_device)


#endif // MAME_SONY_SMC777_KBD_H
