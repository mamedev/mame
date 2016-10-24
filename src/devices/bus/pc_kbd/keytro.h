// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Keytronic Keyboard

***************************************************************************/

#ifndef __KB_KEYTRO_H__
#define __KB_KEYTRO_H__

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

	virtual void clock_write(int state) override;
	virtual void data_write(int state) override;

	uint8_t internal_data_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void internal_data_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p1_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p2_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p2_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p3_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p3_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
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
	pc_kbd_keytronic_pc3270_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: pc_kbd_keytronic_pc3270_device(mconfig, tag, owner, clock)
	{
	}

	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type PC_KBD_KEYTRONIC_PC3270;
extern const device_type PC_KBD_KEYTRONIC_PC3270_AT;

#endif  /* __KB_KEYTRO_H__ */
