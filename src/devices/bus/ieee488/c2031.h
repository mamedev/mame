// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2031 Single Disk Drive emulation

**********************************************************************/

#pragma once

#ifndef __C2031__
#define __C2031__

#include "emu.h"
#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/64h156.h"
#include "machine/6522via.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2031_device

class c2031_device :  public device_t,
						public device_ieee488_interface
{
public:
	// construction/destruction
	c2031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( via0_irq_w );
	DECLARE_READ8_MEMBER( via0_pa_r );
	DECLARE_WRITE8_MEMBER( via0_pa_w );
	DECLARE_READ8_MEMBER( via0_pb_r );
	DECLARE_WRITE8_MEMBER( via0_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_READ8_MEMBER( via1_pb_r );
	DECLARE_WRITE8_MEMBER( via1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( byte_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state);
	virtual void ieee488_ifc(int state);

	inline int get_device_number();

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<c64h156_device> m_ga;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;

	// IEEE-488 bus
	int m_nrfd_out;             // not ready for data
	int m_ndac_out;             // not data accepted
	int m_atna;                 // attention acknowledge
	int m_ifc;

	// interrupts
	int m_via0_irq;             // VIA #0 interrupt request
	int m_via1_irq;             // VIA #1 interrupt request
};


// device type definition
extern const device_type C2031;



#endif
