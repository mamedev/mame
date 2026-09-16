// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Facit F4431 keyboard

***************************************************************************/

#ifndef MAME_FACIT_F4431_KBD_H
#define MAME_FACIT_F4431_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> f4431_kbd_device

class f4431_kbd_device :  public device_t
{
public:
	// construction/destruction
	f4431_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto tx_handler() { return m_tx_handler.bind(); }

	void rx_w(int state);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	int t0_r();

	required_device<i8035_device> m_mcu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<13> m_keys;
	required_ioport m_switches;

	output_finder<7> m_leds;

	devcb_write_line m_tx_handler;

	int m_rx;
	uint8_t m_kbd_p2;
};

// device type definition
DECLARE_DEVICE_TYPE(F4431_KBD, f4431_kbd_device)

#endif // MAME_FACIT_F4431_KBD_H
