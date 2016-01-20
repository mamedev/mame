// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mator Systems SHARK Intelligent Winchester Disc Subsystem emulation

    35MB PRIAM DISKOS 3450 8" Winchester Hard Disk (-chs 525,5,? -ss ?)

**********************************************************************/

#pragma once

#ifndef __SHARK__
#define __SHARK__

#include "ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mshark_device

class mshark_device :  public device_t,
						public device_ieee488_interface
{
public:
	// construction/destruction
	mshark_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type SHARK;



#endif
