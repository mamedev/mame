// license:BSD-3-Clause
// copyright-holders:AJR

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/s1410.h"

void default_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("s1410", NSCSI_S1410);
};
