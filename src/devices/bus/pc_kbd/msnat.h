// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Microsoft Natural Keyboard

***************************************************************************/

#ifndef __KB_MSNAT_H__
#define __KB_MSNAT_H__

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

	required_device<cpu_device> m_cpu;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void clock_write(int state) override;
	virtual void data_write(int state) override;

	uint8_t p0_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p0_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p1_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p3_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p3_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	required_ioport m_p2_0;
	required_ioport m_p2_1;
	required_ioport m_p2_2;
	required_ioport m_p2_3;
	required_ioport m_p2_4;
	required_ioport m_p2_5;
	required_ioport m_p2_6;
	required_ioport m_p2_7;
	required_ioport m_p1_0;
	required_ioport m_p1_1;
	required_ioport m_p1_2;
	required_ioport m_p1_3;
	required_ioport m_p1_4;
	required_ioport m_p1_5;
	required_ioport m_p1_6;
	required_ioport m_p1_7;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t   m_p0;
	uint8_t   m_p1;
	uint8_t   m_p2;
	uint8_t   m_p3;
};


// device type definition
extern const device_type PC_KBD_MICROSOFT_NATURAL;

#endif  /* __KB_MSNAT_H__ */
