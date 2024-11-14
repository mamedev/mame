// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy Super Disk emulation

**********************************************************************/

#ifndef MAME_BUS_QL_SANDY_SUPERDISK_H
#define MAME_BUS_QL_SANDY_SUPERDISK_H

#pragma once

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sandy_super_disk_device

class sandy_super_disk_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	sandy_super_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	void busy_w(int state);

	static void floppy_formats(format_registration &fr);

	void check_interrupt();

	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;
	required_memory_region m_rom;

	int m_busy;
	int m_fd6;
};


// device type definition
DECLARE_DEVICE_TYPE(SANDY_SUPER_DISK, sandy_super_disk_device)


#endif // MAME_BUS_QL_SANDY_SUPERDISK_H
