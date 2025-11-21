// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"

#include "bus/nscsi/applecd.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/cdd2000.h"
#include "bus/nscsi/cdrn820s.h"
#include "bus/nscsi/cdu75s.h"
#include "bus/nscsi/cdu415.h"
#include "bus/nscsi/cdu561.h"
#include "bus/nscsi/cfp1080s.h"
#include "bus/nscsi/crd254sh.h"
#include "bus/nscsi/cw7501.h"
#include "bus/nscsi/dtc510.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/s1410.h"
#include "bus/nscsi/smoc501.h"
#include "bus/nscsi/tape.h"

void default_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
	device.option_add("s1410", NSCSI_S1410);
	device.option_add("dtc510", NSCSI_DTC510);
	device.option_add("cw7501", CW7501);
	device.option_add("cdr4210", CDR4210);
	device.option_add("cdrn820s", CDRN820S);
	device.option_add("cdd2000", CDD2000);
	device.option_add("cdu75s", CDU75S);
	device.option_add("cdu415", CDU415);
	device.option_add("cdu561_25", CDU561_25);
	device.option_add("crd254sh", CRD254SH);
	device.option_add("smoc501", SMOC501);
	device.option_add("aplcd150", APPLECD150);
	device.option_add("aplcdsc", NSCSI_CDROM_APPLE);
	device.option_add("cfp1080s", CFP1080S);
}

void mac_scsi_devices(device_slot_interface &device)
{
	default_scsi_devices(device);
	device.option_replace("cdrom", NSCSI_CDROM_APPLE);
}
