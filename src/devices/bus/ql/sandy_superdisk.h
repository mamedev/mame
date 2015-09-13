// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy Super Disk emulation

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

	WRITE_LINE_MEMBER( busy_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data);
	virtual void write(address_space &space, offs_t offset, UINT8 data);

private:
	void check_interrupt();

	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;
	required_memory_region m_rom;

	int m_busy;
	int m_fd6;
};


// device type definition
extern const device_type SANDY_SUPER_DISK;


#endif
