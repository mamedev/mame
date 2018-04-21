// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H
#define MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H

#pragma once

#include "keyboard.h"

#include "machine/keyboard.h"
#include "sound/spkrdev.h"

namespace bus { namespace interpro { namespace keyboard {

class lle_device_base
	: public device_t
	, public device_interpro_keyboard_port_interface
	, protected device_matrix_keyboard_interface<5U>
{
public:
	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) override;

protected:
	// constructor/destructor
	lle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override {};
	virtual void key_break(u8 row, u8 column) override {};

private:
	required_device<cpu_device>           m_mcu;
	required_device<speaker_sound_device> m_bell;
};

class lle_en_us_device : public lle_device_base
{
public:
	lle_en_us_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override;
	virtual tiny_rom_entry const *device_rom_region() const override;
};

} } } // namespace bus::interpro::keyboard

DECLARE_DEVICE_TYPE_NS(INTERPRO_LLE_EN_US_KEYBOARD, bus::interpro::keyboard, lle_en_us_device)

#endif // MAME_DEVICES_INTERPRO_KEYBOARD_LLE_H
