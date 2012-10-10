/*

scsidev.c

Base class for SCSI devices.

*/

#include "machine/scsibus.h"
#include "machine/scsidev.h"

#define LOG ( 0 )

scsidev_device::scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void scsidev_device::device_start()
{
	data_out = 0;
}

void scsidev_device::scsi_out( UINT32 data, UINT32 mask )
{
#if LOG
	printf( "%s scsi_out", tag() );

	printf( " rst " );
	if( ( mask & SCSI_MASK_RST ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_RST ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " atn " );
	if( ( mask & SCSI_MASK_ATN ) != 0 )
	{
		printf( " %d", (int)( ( data & SCSI_MASK_ATN ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " ack " );
	if( ( mask & SCSI_MASK_ACK ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_ACK ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " req " );
	if( ( mask & SCSI_MASK_REQ ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_REQ ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " msg " );
	if( ( mask & SCSI_MASK_MSG ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_MSG ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " io " );
	if( ( mask & SCSI_MASK_IO ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_IO ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " cd " );
	if( ( mask & SCSI_MASK_CD ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_CD ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " sel " );
	if( ( mask & SCSI_MASK_SEL ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_SEL ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " bsy " );
	if( ( mask & SCSI_MASK_BSY ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_BSY ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " p " );
	if( ( mask & SCSI_MASK_DATAP ) != 0 )
	{
		printf( "%d", (int)( ( data & SCSI_MASK_DATAP ) != 0 ) );
	}
	else
	{
		printf( "-" );
	}

	printf( " " );

	if( ( mask & SCSI_MASK_DATAH ) != 0 )
	{
		printf( "%02x", ( data & SCSI_MASK_DATAH ) >> 8 );
	}
	else
	{
		printf( "--" );
	}

	if( ( mask & SCSI_MASK_DATA ) != 0 )
	{
		printf( "%02x", data & SCSI_MASK_DATA );
	}
	else
	{
		printf( "--" );
	}

	printf( "\n" );
#endif

	data_out = ( data_out & ~mask ) | ( data & mask );

	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	m_scsibus->scsi_update();
}
