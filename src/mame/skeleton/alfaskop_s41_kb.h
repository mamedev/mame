// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
#ifndef MAME_SKELETON_ALFASKOP_S41_KB_H
#define MAME_SKELETON_ALFASKOP_S41_KB_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "machine/mc6846.h"

DECLARE_DEVICE_TYPE(ALFASKOP_S41_KB, alfaskop_s41_keyboard_device)

class alfaskop_s41_keyboard_device : public device_t
{
public:
	auto txd_cb() { return m_txd_cb.bind(); }
	auto leds_cb() { return m_leds_cb.bind(); }

	alfaskop_s41_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(key);
	void rxd_w(int state);
	//void hold_w(int state);
	void rst_line_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	required_device<m6802_cpu_device> m_mcu;
	required_device<mc6846_device> m_mc6846;
	required_ioport_array<6>       m_rows;
	devcb_write_line               m_txd_cb; // Callback for KBD-> S41
	devcb_write8                   m_leds_cb; // Callback for all 8 leds -> layout

	bool    m_rxd_high; // state of Rx input line
	bool    m_txd_high; // state of Tx output line
	bool    m_hold;
	uint16_t m_col_select;
	uint8_t m_p1;
	uint8_t m_leds;

	void alfaskop_s41_kb_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_SKELETON_ALFASKOP_S41_KB_H
