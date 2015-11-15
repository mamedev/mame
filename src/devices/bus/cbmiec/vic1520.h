// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1520 Plotter emulation

**********************************************************************/

#pragma once

#ifndef __VIC1520__
#define __VIC1520__

#include "emu.h"
#include "cbmiec.h"
#include "cpu/m6502/m6502.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1520_t

class vic1520_t :  public device_t,
					public device_cbm_iec_interface
{
public:
	// construction/destruction
	vic1520_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state);
	void cbm_iec_data(int state);
	void cbm_iec_reset(int state);
};


// device type definition
extern const device_type VIC1520;



#endif
