// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mator Systems SHARK Intelligent Winchester Disc Subsystem emulation

    35MB PRIAM DISKOS 3450 8" Winchester Hard Disk (-chs 525,5,? -ss ?)

**********************************************************************/

#ifndef MAME_BUS_IEEE488_SHARK_H
#define MAME_BUS_IEEE488_SHARK_H

#pragma once

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
	mshark_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void mshark_io(address_map &map) ATTR_COLD;
	void mshark_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MSHARK, mshark_device)

#endif // MAME_BUS_IEEE488_SHARK_H
