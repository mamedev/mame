// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_TV950KB_H
#define MAME_MACHINE_TV950KB_H

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

	DECLARE_WRITE_LINE_MEMBER(rx_w);

protected:
	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	u8 keys_r();
	DECLARE_WRITE_LINE_MEMBER(tx_w);

	void rw_map(address_map &map);

	// line output callback
	devcb_write_line m_tx_cb;

	required_device<mcs48_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_beeper;
	required_ioport_array<13> m_keys;
};

// device type definition
DECLARE_DEVICE_TYPE(TV950_KEYBOARD, tv950kb_device)

#endif // MAME_MACHINE_TV950KB_H
