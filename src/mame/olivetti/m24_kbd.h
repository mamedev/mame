// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_OLIVETTI_M24_KBD_H
#define MAME_OLIVETTI_M24_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


class m24_keyboard_device :  public device_t
{
public:
	m24_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_data_handler() { return m_out_data.bind(); }

	void clock_w(int state);
	void data_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(reset_mcu);

private:
	devcb_write_line m_out_data;

	required_device<i8049_device> m_mcu;

	required_ioport_array<16> m_rows;
	required_ioport m_mousebtn;
	required_ioport m_mousex;
	required_ioport m_mousey;

	emu_timer *m_reset_timer;

	uint8_t m_p1;
	bool m_keypress, m_kbcdata;

	uint32_t m_mouse_read_count;
	int32_t m_count_mx, m_count_my;
	uint8_t m_bits_mx, m_bits_my;
	uint8_t m_last_mx, m_last_my;

	void bus_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
};

DECLARE_DEVICE_TYPE(M24_KEYBOARD, m24_keyboard_device)

#endif // MAME_OLIVETTI_M24_KBD_H
