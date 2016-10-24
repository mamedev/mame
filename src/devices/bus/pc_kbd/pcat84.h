// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IBM Model F PC/AT 84-key / 3270PC 122-key keyboard emulation

*********************************************************************/

#pragma once

#ifndef __PC_KBD_IBM_PC_AT_84__
#define __PC_KBD_IBM_PC_AT_84__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "pc_kbdc.h"
#include "machine/rescap.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ibm_pc_at_84_keyboard_device

class ibm_pc_at_84_keyboard_device :  public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pc_kbd_interface overrides
	virtual void clock_write(int state) override { m_maincpu->set_input_line(MCS48_INPUT_IRQ, state); };
	virtual void data_write(int state) override { };

private:
	enum
	{
		LED_SCROLL,
		LED_NUM,
		LED_CAPS
	};

	int key_depressed();

	required_device<cpu_device> m_maincpu;
	required_ioport_array<16> m_dr;
	optional_ioport m_kbdida;
	optional_ioport m_kbdidb;

	uint8_t m_db;
	int m_cnt;
	int m_sense;
	int m_t1;
};


// ======================> ibm_3270pc_122_keyboard_device

class ibm_3270pc_122_keyboard_device :  public ibm_pc_at_84_keyboard_device
{
public:
	// construction/destruction
	ibm_3270pc_122_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type PC_KBD_IBM_PC_AT_84;
extern const device_type PC_KBD_IBM_3270PC_122;



#endif
