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
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_WRITE_LINE_MEMBER(clock_write) override;
	virtual DECLARE_WRITE_LINE_MEMBER(data_write) override;

	DECLARE_READ8_MEMBER( internal_data_read );
	DECLARE_WRITE8_MEMBER( internal_data_write );
	DECLARE_READ8_MEMBER( p1_read );
	DECLARE_WRITE8_MEMBER( p1_write );
	DECLARE_READ8_MEMBER( p2_read );
	DECLARE_WRITE8_MEMBER( p2_write );
	DECLARE_READ8_MEMBER( p3_read );
	DECLARE_WRITE8_MEMBER( p3_write );

	void keytronic_pc3270_io(address_map &map);
	void keytronic_pc3270_program(address_map &map);

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

	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_KEYTRONIC_PC3270,    pc_kbd_keytronic_pc3270_device)
DECLARE_DEVICE_TYPE(PC_KBD_KEYTRONIC_PC3270_AT, pc_kbd_keytronic_pc3270_at_device)

#endif // MAME_BUS_PC_KBD_KEYTRO_H
