// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Digilog protocol anaylzer keyboard

***************************************************************************/

#ifndef MAME_SKELETON_DIGILOG_KBD_H
#define MAME_SKELETON_DIGILOG_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> digilog_kbd_device

class digilog_kbd_device : public device_t
{
public:
	// construction/destruction
	digilog_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto tx_handler() { return m_tx_handler.bind(); }

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8748_device> m_mcu;
	required_ioport_array<16> m_keys;

	uint8_t bus_r();
	void bus_w(uint8_t data);
	int t0_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();

	devcb_write_line m_tx_handler;

	uint8_t m_bus;
	uint8_t m_key_row;
};

// device type definition
DECLARE_DEVICE_TYPE(DIGILOG_KBD, digilog_kbd_device)

#endif // MAME_SKELETON_DIGILOG_KBD_H
