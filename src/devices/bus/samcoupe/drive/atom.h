// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ATOM hard disk interface for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_DRIVE_ATOM_H
#define MAME_BUS_SAMCOUPE_DRIVE_ATOM_H

#pragma once

#include "drive.h"
#include "bus/ata/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_atom_hdd_device

class sam_atom_hdd_device : public device_t, public device_samcoupe_drive_interface
{
public:
	// construction/destruction
	sam_atom_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<ata_interface_device> m_ata;

	int m_address_latch;
	uint8_t m_read_latch, m_write_latch;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_ATOM_HDD, sam_atom_hdd_device)

#endif // MAME_BUS_SAMCOUPE_DRIVE_ATOM_H
