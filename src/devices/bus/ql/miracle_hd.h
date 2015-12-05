// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Hard Disk emulation

**********************************************************************/

#pragma once

#ifndef __MIRACLE_HARD_DISK__
#define __MIRACLE_HARD_DISK__

#include "rom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> miracle_hard_disk_t

class miracle_hard_disk_t : public device_t,
							public device_ql_rom_cartridge_card_interface
{
public:
	// construction/destruction
	miracle_hard_disk_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	miracle_hard_disk_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_ql_rom_cartridge_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data);
	virtual void write(address_space &space, offs_t offset, UINT8 data);
};



// device type definition
extern const device_type MIRACLE_HARD_DISK;



#endif
