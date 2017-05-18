// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Keytronic Keyboard

***************************************************************************/

#ifndef MAME_BUS_PC_KBD_KEYTRO_H
#define MAME_BUS_PC_KBD_KEYTRO_H

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

	required_device<cpu_device> m_cpu;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
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

protected:
	pc_kbd_keytronic_pc3270_device(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
extern const device_type PC_KBD_KEYTRONIC_PC3270;
extern const device_type PC_KBD_KEYTRONIC_PC3270_AT;

#endif // MAME_BUS_PC_KBD_KEYTRO_H
