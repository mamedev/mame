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
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	ibm_pc_at_84_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t0_r );
	DECLARE_READ8_MEMBER( t1_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write ) override { m_maincpu->set_input_line(MCS48_INPUT_IRQ, state); };
	virtual DECLARE_WRITE_LINE_MEMBER( data_write ) override { };

private:
	enum
	{
		LED_SCROLL,
		LED_NUM,
		LED_CAPS
	};

	int key_depressed();

	required_device<cpu_device> m_maincpu;
	required_ioport m_dr00;
	required_ioport m_dr01;
	required_ioport m_dr02;
	required_ioport m_dr03;
	required_ioport m_dr04;
	required_ioport m_dr05;
	required_ioport m_dr06;
	required_ioport m_dr07;
	required_ioport m_dr08;
	required_ioport m_dr09;
	required_ioport m_dr10;
	required_ioport m_dr11;
	required_ioport m_dr12;
	required_ioport m_dr13;
	required_ioport m_dr14;
	required_ioport m_dr15;
	optional_ioport m_kbdida;
	optional_ioport m_kbdidb;

	UINT8 m_db;
	int m_cnt;
	int m_sense;
	int m_t1;
};


// ======================> ibm_3270pc_122_keyboard_device

class ibm_3270pc_122_keyboard_device :  public ibm_pc_at_84_keyboard_device
{
public:
	// construction/destruction
	ibm_3270pc_122_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type PC_KBD_IBM_PC_AT_84;
extern const device_type PC_KBD_IBM_3270PC_122;



#endif
