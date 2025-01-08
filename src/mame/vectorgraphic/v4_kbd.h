// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/**********************************************************************

Vector 4 91-key keyboard

**********************************************************************/

#ifndef MAME_VECTORGRAPHIC_V4_KBD_H
#define MAME_VECTORGRAPHIC_V4_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"


class vector4_keyboard_device : public device_t
{
public:
	// device type constructor
	vector4_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto txd_handler() { return m_txd_cb.bind(); }

	// serial line input
	void write_rxd(int state);

protected:
	// device-level overrides
	void device_resolve_objects() override;
	void device_start() override ATTR_COLD;
	ioport_constructor device_input_ports() const override;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	uint8_t p1_r();
	void p2_w(uint8_t data);
	void leds_w(uint8_t data);

	// address maps
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<beep_device> m_beeper;
	required_ioport_array<12> m_keys;
	output_finder<> m_led;

	// output callback
	devcb_write_line m_txd_cb;

	uint8_t m_column;
	uint8_t m_p24;
};

DECLARE_DEVICE_TYPE(VECTOR4_KEYBOARD, vector4_keyboard_device)

#endif
