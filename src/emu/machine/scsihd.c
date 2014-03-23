// license:MAME
// copyright-holders:smf
/***************************************************************************

 scsihd.c - Implementation of a SCSI hard disk drive

***************************************************************************/

#include "scsihd.h"

// device type definition
const device_type SCSIHD = &device_creator<scsihd_device>;

scsihd_device::scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihle_device(mconfig, SCSIHD, "SCSIHD", tag, owner, clock, "scsihd", __FILE__)
{
}

scsihd_device::scsihd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	scsihle_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void scsihd_device::device_start()
{
	m_image = subdevice<harddisk_image_device>("image");

	scsihle_device::device_start();
}

harddisk_interface scsihd_device::hd_intf = { "scsi_hdd", NULL };

static MACHINE_CONFIG_FRAGMENT(scsi_harddisk)
	MCFG_HARDDISK_CONFIG_ADD("image", scsihd_device::hd_intf)
MACHINE_CONFIG_END

machine_config_constructor scsihd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_harddisk);
}
