// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sandy Super Disk emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SANDY_SUPER_DISK__
#define __SANDY_SUPER_DISK__

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "formats/ql_dsk.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sandy_super_disk_device

class sandy_super_disk_t : public device_t,
			   			   public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	sandy_super_disk_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ql_expansion_card_interface overrides

private:
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type SANDY_SUPER_DISK;


#endif
