// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsihd.c - Implementation of a SCSI hard disk drive

***************************************************************************/

#include "scsihd.h"

// device type definition
const device_type SCSIHD = &device_creator<scsihd_device>;

scsihd_device::scsihd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: scsihle_device(mconfig, SCSIHD, "SCSI HD", tag, owner, clock, "scsihd", __FILE__)
{
}

scsihd_device::scsihd_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	scsihle_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void scsihd_device::device_start()
{
	m_image = subdevice<harddisk_image_device>("image");

	scsihle_device::device_start();
}

static MACHINE_CONFIG_FRAGMENT(scsi_harddisk)
	MCFG_HARDDISK_ADD("image")
	MCFG_HARDDISK_INTERFACE("scsi_hdd")
MACHINE_CONFIG_END

machine_config_constructor scsihd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_harddisk);
}
