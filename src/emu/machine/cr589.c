/*
 * Matsushita CR-589
 *
 */

#include "emu.h"
#include "machine/cr589.h"

// device type definition
const device_type CR589 = &device_creator<cr589_device>;

cr589_device::cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsicd_device(mconfig, CR589, "CR589", tag, owner, clock)
{
}

static const int identity_offset = 0x3ab;
static const char download_identity[] = "MATSHITA CD98Q4 DOWNLOADGS0N";

void cr589_device::device_start()
{
	scsicd_device::device_start();

	download = 0;
	memcpy( &buffer[ identity_offset ], "MATSHITACD-ROM CR-589   GS0N", 28 );

	save_item(NAME(download));
	save_item(NAME(buffer));
	save_item(NAME(bufferOffset));
}

void cr589_device::ExecCommand( int *transferLength )
{
	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
			break;

		case 0x3c: // READ BUFFER
			bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
			break;

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = SCSILengthFromUINT16( &command[7] );
			break;

		default:
			scsicd_device::ExecCommand( transferLength );
	}
}

void cr589_device::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case 0x12: // INQUIRY
			scsicd_device::ReadData( data, dataLength );

			if( download )
			{
				memcpy( &data[ 8 ], download_identity, 28 );
			}
			else
			{
				memcpy( &data[ 8 ], &buffer[ identity_offset ], 28 );
			}
			break;

		case 0x3c: // READ BUFFER
			memcpy( data, &buffer[ bufferOffset ], dataLength );
			bufferOffset += dataLength;
			break;

		default:
			scsicd_device::ReadData( data, dataLength );
			break;
	}
}

void cr589_device::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			memcpy( &buffer[ bufferOffset ], data + 32, dataLength - 32 );
			bufferOffset += dataLength;
			break;

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			if( memcmp( data, &buffer[ identity_offset ], 28 ) == 0 )
			{
				download = 1;
			}
			else if( memcmp( data, download_identity, 28 ) == 0 )
			{
				download = 0;
			}
			break;

		default:
			scsicd_device::WriteData( data, dataLength );
			break;
	}
}
