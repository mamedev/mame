/*

scsibus.c

*/

#include "emu.h"
#include "machine/scsibus.h"

void scsibus_device::scsi_update()
{
	UINT32 newdata = SCSI_MASK_ALL;

	for( int i = 0; i < deviceCount; i++ )
	{
		newdata &= devices[ i ]->data_out;
	}

	UINT32 mask = data ^ newdata;

	if( mask != 0 )
	{
		data = newdata;

		for( int i = 0; i < deviceCount; i++ )
		{
			devices[ i ]->scsi_in( data, mask );
		}
	}
}

scsibus_device::scsibus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SCSIBUS, "SCSI bus", tag, owner, clock)
{
}

void scsibus_device::device_start()
{
	deviceCount = 0;

	for( device_t *device = first_subdevice(); device != NULL; device = device->next() )
	{
		scsidev_device *scsidev = dynamic_cast<scsidev_device *>(device);
		if( scsidev != NULL )
		{
			devices[ deviceCount++ ] = scsidev;
		}
	}

	data = SCSI_MASK_ALL;
}

const device_type SCSIBUS = &device_creator<scsibus_device>;
