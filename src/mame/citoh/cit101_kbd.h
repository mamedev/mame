// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    CIT-101 85-key keyboard

**********************************************************************/

#ifndef MAME_CITOH_CIT101_KBD_H
#define MAME_CITOH_CIT101_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cit101_keyboard_device

class cit101_keyboard_device : public device_t
{
public:
	// device type constructor
	cit101_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	// serial line input
	void write_rxd(int state);

protected:
	cit101_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	void p2_w(u8 data);
	void leds_w(u8 data);
	u8 keys_r();

	// address maps
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<beep_device> m_beeper;
	required_ioport_array<11> m_keys;
	required_ioport m_kbid;
	output_finder<7> m_leds;

	// output callback
	devcb_write_line m_txd_callback;

	// internal state
	bool m_kbid_enabled;
};

// ======================> cit101e_keyboard_device

class cit101e_keyboard_device : public cit101_keyboard_device
{
public:
	// device type constructor
	cit101e_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type declarations
DECLARE_DEVICE_TYPE(CIT101_KEYBOARD, cit101_keyboard_device)
DECLARE_DEVICE_TYPE(CIT101E_KEYBOARD, cit101e_keyboard_device)

#endif // MAME_CITOH_CIT101_KBD_H
