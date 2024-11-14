// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "hdd.h"


//**************************************************************************
//  IDE HARD DISK DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(IDE_HARDDISK, ide_hdd_device, "idehd", "IDE Hard Disk")

//-------------------------------------------------
//  ide_hdd_device - constructor
//-------------------------------------------------

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ide_hdd_device(mconfig, IDE_HARDDISK, tag, owner, clock)
{
}

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	ide_hdd_device_base(mconfig, type, tag, owner, clock),
	device_ata_interface(mconfig, *this)
{
}


//**************************************************************************
//  ATA COMPACTFLASH CARD DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATA_CF, ata_cf_device, "atacf", "ATA CompactFlash Card")

//-------------------------------------------------
//  ata_cf_device - constructor
//-------------------------------------------------

ata_cf_device::ata_cf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cf_device_base(mconfig, ATA_CF, tag, owner, clock),
	device_ata_interface(mconfig, *this)
{
}
