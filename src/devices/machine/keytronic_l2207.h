// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Keytronic L2207 83-key keyboard

**********************************************************************/

#ifndef MAME_MACHINE_KEYTRONIC_L2207_H
#define MAME_MACHINE_KEYTRONIC_L2207_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> keytronic_l2207_device

class keytronic_l2207_device : public device_t
{
public:
	// device type constructor
	keytronic_l2207_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// callback configuration
	auto ser_out_callback() { return m_ser_out_callback.bind(); }

	// serial line input
	void ser_in_w(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	u8 led_latch_r();
	u8 p1_r();
	void p2_w(u8 data);

	// address maps
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_beeper;
	required_ioport_array<12> m_keys;
	output_finder<8> m_leds;
	output_finder<> m_all_caps;

	// output callback
	devcb_write_line m_ser_out_callback;

	// internal state
	u8 m_p1_in;
	u8 m_p2_out;
	bool m_beeper_latch;
};

// device type declarations
DECLARE_DEVICE_TYPE(KEYTRONIC_L2207, keytronic_l2207_device)

#endif // MAME_MACHINE_KEYTRONIC_L2207_H
