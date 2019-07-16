// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/cw7501.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/s1410.h"

void default_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("s1410", NSCSI_S1410);
	device.option_add("cw7501", CW7501);
	device.option_add("cdr4210", CDR4210);
};
