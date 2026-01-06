// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADI keyboard for an unknown system

***************************************************************************/

#ifndef MAME_MISC_ADI_UNK_KBD_H
#define MAME_MISC_ADI_UNK_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class adi_unk_kbd_device : public device_t
{
public:
	// construction/destruction
	adi_unk_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto txd_cb() { return m_txd_cb.bind(); }

	void alt_w(int state);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8035_device> m_mcu;
	required_ioport_array<14> m_keys;
	required_ioport m_shift;
	required_ioport m_ctrl;
	required_ioport m_capslock;

	devcb_write_line m_txd_cb;

	uint8_t m_key_row_p1;
	uint8_t m_key_row_p2;

	void mem_map(address_map &map) ATTR_COLD;

	int t0_r();
	int t1_r();
	uint8_t bus_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
};

// device type declaration
DECLARE_DEVICE_TYPE(ADI_UNK_KBD, adi_unk_kbd_device)

#endif // MAME_MISC_ADI_UNK_KBD_H
