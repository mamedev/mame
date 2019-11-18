// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsicd.c - Implementation of a SCSI CD-ROM device

***************************************************************************/

#include "emu.h"
#include "scsicd.h"

// device type definition
DEFINE_DEVICE_TYPE(SCSICD, scsicd_device, "scsicd", "SCSI CD")

scsicd_device::scsicd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd_device(mconfig, SCSICD, tag, owner, clock)
{
}

scsicd_device::scsicd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	scsihle_device(mconfig, type, tag, owner, clock)
{
}

void scsicd_device::device_start()
{
	m_image = subdevice<cdrom_image_device>("image");
	m_cdda = subdevice<cdda_device>("cdda");

	scsihle_device::device_start();
}

void scsicd_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, "image").set_interface("cdrom");
	CDDA(config, "cdda");
}
