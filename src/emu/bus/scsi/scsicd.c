// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsicd.c - Implementation of a SCSI CD-ROM device

***************************************************************************/

#include "scsicd.h"

// device type definition
const device_type SCSICD = &device_creator<scsicd_device>;

scsicd_device::scsicd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	scsihle_device(mconfig, SCSICD, "SCSI CD", tag, owner, clock, "scsicd", __FILE__)
{
}

scsicd_device::scsicd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	scsihle_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void scsicd_device::device_start()
{
	m_image = subdevice<cdrom_image_device>("image");
	m_cdda = subdevice<cdda_device>("cdda");

	scsihle_device::device_start();
}

static MACHINE_CONFIG_FRAGMENT(scsi_cdrom)
	MCFG_CDROM_ADD("image")
	MCFG_CDROM_INTERFACE("cdrom")
	MCFG_SOUND_ADD("cdda", CDDA, 0)
MACHINE_CONFIG_END

machine_config_constructor scsicd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_cdrom);
}
