// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsihd.c - Implementation of a SCSI hard disk drive

***************************************************************************/

#include "emu.h"
#include "scsihd.h"

// device type definition
DEFINE_DEVICE_TYPE(SCSIHD, scsihd_device, "scsihd", "SCSI HD")

scsihd_device::scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scsihd_device(mconfig, SCSIHD, tag, owner, clock)
{
}

scsihd_device::scsihd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	scsihle_device(mconfig, type, tag, owner, clock)
{
}

void scsihd_device::device_start()
{
	m_image = subdevice<harddisk_image_device>("image");

	scsihle_device::device_start();
}

void scsihd_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, "image", "scsi_hdd");
}
