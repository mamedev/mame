// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
#ifndef MAME_ERICSSON_EISPC_KB_H
#define MAME_ERICSSON_EISPC_KB_H

#pragma once

#include "cpu/m6800/m6801.h"

DECLARE_DEVICE_TYPE(EISPC_KB, eispc_keyboard_device)

class eispc_keyboard_device : public device_t
{
public:
	auto txd_cb() { return m_txd_cb.bind(); }
	auto caps_cb() { return m_led_caps_cb.bind(); }
	auto num_cb() { return m_led_num_cb.bind(); }
	auto scroll_cb() { return m_led_scroll_cb.bind(); }

	eispc_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(key);
	void rxd_w(int state);
	void hold_w(int state);
	void rst_line_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	required_device<m6801_cpu_device> m_mcu;
	required_ioport_array<6>       m_rows;
	devcb_write_line               m_txd_cb; // Callback for KBD-> EPC
	devcb_write_line               m_led_caps_cb; // Callback for Caps led -> layout
	devcb_write_line               m_led_num_cb; // Callback for Num led -> layout
	devcb_write_line               m_led_scroll_cb; // Callback for Scroll led -> layout

	bool    m_rxd_high; // state of Rx input line
	bool    m_txd_high; // state of Tx output line
	bool    m_hold;
	uint16_t m_col_select;
	uint8_t m_p1;

	void eispc_kb_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_ERICSSON_EISPC_KB_H
