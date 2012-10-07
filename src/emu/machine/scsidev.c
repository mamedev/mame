/*

scsidev.c

Base class for SCSI devices.

*/

#include "machine/scsibus.h"
#include "machine/scsidev.h"

scsidev_device::scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void scsidev_device::device_start()
{
	data_out = SCSI_MASK_ALL;
}

void scsidev_device::scsi_out( UINT32 data, UINT32 mask )
{
	data_out = ( data_out & ~mask ) | ( data & mask );

	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	m_scsibus->scsi_update();
}
