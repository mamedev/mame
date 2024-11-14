// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Keytronic Keyboard

***************************************************************************/

#ifndef MAME_BUS_PC_KBD_KEYTRO_H
#define MAME_BUS_PC_KBD_KEYTRO_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "pc_kbdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc_kbd_keytronic_pc3270_device

class pc_kbd_keytronic_pc3270_device :  public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	pc_kbd_keytronic_pc3270_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pc_kbd_keytronic_pc3270_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void clock_write(int state) override;
	virtual void data_write(int state) override;

	uint8_t internal_data_read(offs_t offset);
	void internal_data_write(offs_t offset, uint8_t data);
	uint8_t p1_read();
	void p1_write(uint8_t data);
	uint8_t p2_read();
	void p2_write(uint8_t data);
	uint8_t p3_read();
	void p3_write(uint8_t data);

	void keytronic_pc3270_io(address_map &map) ATTR_COLD;
	void keytronic_pc3270_program(address_map &map) ATTR_COLD;

	required_device<i8051_device> m_cpu;

	uint8_t   m_p1;
	uint8_t   m_p1_data;
	uint8_t   m_p2;
	uint8_t   m_p3;
	uint16_t  m_last_write_addr;
};


class pc_kbd_keytronic_pc3270_at_device : public pc_kbd_keytronic_pc3270_device
{
public:
	// construction/destruction
	pc_kbd_keytronic_pc3270_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_KEYTRONIC_PC3270,    pc_kbd_keytronic_pc3270_device)
DECLARE_DEVICE_TYPE(PC_KBD_KEYTRONIC_PC3270_AT, pc_kbd_keytronic_pc3270_at_device)

#endif // MAME_BUS_PC_KBD_KEYTRO_H
