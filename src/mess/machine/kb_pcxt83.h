/**********************************************************************

    IBM Model F PC/XT 83-key keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __PC_KBD_IBM_PC_XT_83__
#define __PC_KBD_IBM_PC_XT_83__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/pc_kbdc.h"
#include "machine/rescap.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ibm_pc_xt_83_keyboard_device

class ibm_pc_xt_83_keyboard_device :  public device_t,
									  public device_pc_kbd_interface
{
public:
	// construction/destruction
	ibm_pc_xt_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t0_r );
	DECLARE_READ8_MEMBER( t1_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write ) { m_maincpu->set_input_line(MCS48_INPUT_IRQ, !state); };
	virtual DECLARE_WRITE_LINE_MEMBER( data_write ) { };

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_p10;
	required_ioport m_p11;
	required_ioport m_p12;
	required_ioport m_p13;
	required_ioport m_p14;
	required_ioport m_p15;
	required_ioport m_p16;
	required_ioport m_p17;
	required_ioport m_p23;
	required_ioport m_p24;
	required_ioport m_p25;
	required_ioport m_p26;
	required_ioport m_p27;

	UINT8 m_p1;
	UINT8 m_p2;
	int m_sense;
};


// device type definition
extern const device_type PC_KBD_IBM_PC_XT_83;



#endif
