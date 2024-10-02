// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Microsoft Natural Keyboard

***************************************************************************/

#ifndef MAME_BUS_PC_KB_MSNAT_H
#define MAME_BUS_PC_KB_MSNAT_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "pc_kbdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc_kbd_microsoft_natural_device : public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	pc_kbd_microsoft_natural_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void clock_write(int state) override;
	virtual void data_write(int state) override;

private:
	required_device<i8051_device> m_cpu;

	required_ioport_array<8> m_p2_r;
	required_ioport_array<8> m_p1_r;

	uint8_t   m_p0;
	uint8_t   m_p1;
	uint8_t   m_p2;
	uint8_t   m_p3;

	uint8_t p0_read();
	void p0_write(uint8_t data);
	void p1_write(uint8_t data);
	void p2_write(uint8_t data);
	uint8_t p3_read();
	void p3_write(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_MICROSOFT_NATURAL, pc_kbd_microsoft_natural_device)

#endif // MAME_BUS_PC_KB_MSNAT_H
