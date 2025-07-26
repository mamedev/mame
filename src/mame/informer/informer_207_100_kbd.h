// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Informer 207/100 Keyboard

***************************************************************************/

#ifndef MAME_INFORMER_INFORMER_207_100_KBD_H
#define MAME_INFORMER_INFORMER_207_100_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"


class informer_207_100_kbd_device : public device_t
{
public:
	// construction/destruction
	informer_207_100_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_callback() { return m_txd_callback.bind(); }

	void rxd_w(int state);

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 bus_r();
	void p2_w(u8 data);

	required_device<mcs48_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<13> m_keys;

	devcb_write_line m_txd_callback;
};

// device type declaration
DECLARE_DEVICE_TYPE(INFORMER_207_100_KBD, informer_207_100_kbd_device)

#endif // MAME_INFORMER_INFORMER_207_100_KBD_H
