// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Iskra-1030 XX-key keyboard emulation

*********************************************************************/

#pragma once

#ifndef __PC_KBD_ISKR_1030__
#define __PC_KBD_ISKR_1030__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "pc_kbdc.h"
#include "machine/rescap.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iskr_1030_keyboard_device

class iskr_1030_keyboard_device :  public device_t,
										public device_pc_kbd_interface
{
public:
	// construction/destruction
	iskr_1030_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t1_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write );
	virtual DECLARE_WRITE_LINE_MEMBER( data_write );

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_md00;
	required_ioport m_md01;
	required_ioport m_md02;
	required_ioport m_md03;
	required_ioport m_md04;
	required_ioport m_md05;
	required_ioport m_md06;
	required_ioport m_md07;
	required_ioport m_md08;
	required_ioport m_md09;
	required_ioport m_md10;
	required_ioport m_md11;
	required_ioport m_md12;
	required_ioport m_md13;
	required_ioport m_md14;
	required_ioport m_md15;
	required_ioport m_md16;
	required_ioport m_md17;
	required_ioport m_md18;
	required_ioport m_md19;
	required_ioport m_md20;
	required_ioport m_md21;
	required_ioport m_md22;
	required_ioport m_md23;

	dynamic_buffer m_ram;
	UINT8 m_bus;
	UINT8 m_p1;
	UINT8 m_p2;
	int m_q;
};


// device type definition
extern const device_type PC_KBD_ISKR_1030;

#endif
