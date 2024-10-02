// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_TELEVIDEO_TV950KB_H
#define MAME_TELEVIDEO_TV950KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tv950kb_device

class tv950kb_device : public device_t
{
public:
	tv950kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto tx_cb() { return m_tx_cb.bind(); }

	void rx_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 keys_r();
	void tx_w(int state);

	void rw_map(address_map &map) ATTR_COLD;

	// line output callback
	devcb_write_line m_tx_cb;

	required_device<mcs48_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_beeper;
	required_ioport_array<13> m_keys;
};

// device type definition
DECLARE_DEVICE_TYPE(TV950_KEYBOARD, tv950kb_device)

#endif // MAME_TELEVIDEO_TV950KB_H
