// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Esprit Systems Executive 10/102 Keyboard

***************************************************************************/

#ifndef MAME_ESPRIT_EXECUTIVE10_102_KBD_H
#define MAME_ESPRIT_EXECUTIVE10_102_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class executive10_102_kbd_device : public device_t
{
public:
	// construction/destruction
	executive10_102_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto txd_cb() { return m_txd_cb.bind(); }
	auto cts_cb() { return m_cts_cb.bind(); }

	// from host
	void rxd_w(int state);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8749_device> m_mcu;
	required_device<beep_device> m_buzzer;
	required_ioport m_shift;
	required_ioport m_ctrl;
	required_ioport_array<16> m_keys;

	devcb_write_line m_txd_cb;
	devcb_write_line m_cts_cb;

	uint8_t m_key_row;

	int t0_r();
	int t1_r();
	uint8_t bus_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
};

// device type declaration
DECLARE_DEVICE_TYPE(EXECUTIVE10_102_KBD, executive10_102_kbd_device)

#endif // MAME_ESPRIT_EXECUTIVE10_102_KBD_H
