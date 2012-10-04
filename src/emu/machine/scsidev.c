/***************************************************************************

 scsidev.c - Base class for SCSI devices.

***************************************************************************/

#include "machine/scsidev.h"

scsidev_device::scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}
