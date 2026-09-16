// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IBM Model F PC/AT 84-key / 3270PC 122-key keyboard emulation

*********************************************************************/

#ifndef MAME_BUS_PC_KBD_PCAT84_H
#define MAME_BUS_PC_KBD_PCAT84_H

#pragma once

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
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_pc_kbd_interface overrides
	virtual void clock_write(int state) override { m_maincpu->set_input_line(MCS48_INPUT_IRQ, state); }
	virtual void data_write(int state) override { }

private:
	void bus_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	int t0_r();
	int t1_r();

	enum
	{
		LED_SCROLL = 0,
		LED_NUM,
		LED_CAPS
	};

	int key_depressed();

	required_device<i8048_device> m_maincpu;
	required_ioport_array<16> m_dr;
	optional_ioport m_kbdida;
	optional_ioport m_kbdidb;
	output_finder<3> m_leds;

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
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_IBM_PC_AT_84,   ibm_pc_at_84_keyboard_device)
DECLARE_DEVICE_TYPE(PC_KBD_IBM_3270PC_122, ibm_3270pc_122_keyboard_device)

#endif // MAME_BUS_PC_KBD_PCAT84_H
