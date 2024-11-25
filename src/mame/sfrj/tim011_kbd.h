// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    TIM-011 94-key keyboard

**********************************************************************/

#ifndef MAME_SFRJ_TIM011_KBD_H
#define MAME_SFRJ_TIM011_KBD_H

#pragma once

#include "sound/spkrdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tim011_keyboard_device

class tim011_keyboard_device : public device_t
{
public:
	// device type constructor
	tim011_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	// serial line input
	void write_rxd(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// I/O handlers
	void q_w(int state);
	void shift_w(u8 data);
	void reset_w(u8 data);
	void speaker_off_w(u8 data);
	void speaker_on_w(u8 data);

	// address maps
	void keyboard_mem(address_map &map) ATTR_COLD;
	void keyboard_io(address_map &map) ATTR_COLD;

	// object finders
	required_device<cpu_device> m_keybcpu;
	required_device<speaker_sound_device> m_speaker;
	output_finder<8> m_leds;

	// output callback
	devcb_write_line m_txd_callback;

	// internal state
	u8 m_shifter;
};

// device type declaration
DECLARE_DEVICE_TYPE(TIM011_KEYBOARD, tim011_keyboard_device)

#endif // MAME_SFRJ_TIM011_KBD_H
