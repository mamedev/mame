// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H
#define MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H

#pragma once

#include "keyboard.h"

#include "machine/keyboard.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "sound/spkrdev.h"

namespace bus::interpro::keyboard {

class lle_device_base
	: public device_t
	, public device_interpro_keyboard_port_interface
{
protected:
	// constructor/destructor
	lle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void io_map(address_map &map) ATTR_COLD;
	virtual void ext_map(address_map &map) ATTR_COLD;

	void input_txd(int state) override { m_txd = state; }

	int t0_r();
	int t1_r();
	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 bus_r();
	void bus_w(u8 data);

private:
	required_device<i8049_device> m_mcu;
	required_device<address_map_bank_device> m_ext;
	required_ioport_array<15> m_upper;
	required_ioport_array<11> m_lower;
	required_device<speaker_sound_device> m_speaker;

	output_finder<8> m_leds;

	u8 m_txd;
	u8 m_p1;
	u8 m_p2;
	u8 m_bus;

	u8 m_row;
	u8 m_count;
};

class lle_en_us_device : public lle_device_base
{
public:
	lle_en_us_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
};

} // namespace bus::interpro::keyboard

DECLARE_DEVICE_TYPE_NS(INTERPRO_LLE_EN_US_KEYBOARD, bus::interpro::keyboard, lle_en_us_device)

#endif // MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H
