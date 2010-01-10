/*
 * Matsushita CR-589
 *
 */

#include "emu.h"
#include "cr589.h"

typedef struct
{
	int download;
	UINT8 buffer[ 65536 ];
	int bufferOffset;
} SCSICr589;

static const int identity_offset = 0x3ab;
static const char download_identity[] = "MATSHITA CD98Q4 DOWNLOADGS0N";

static int cr589_exec_command( SCSIInstance *scsiInstance, UINT8 *statusCode )
{
	UINT8 *command;
	int commandLength;
	SCSICr589 *our_this = (SCSICr589 *)SCSIThis( &SCSIClassCr589, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			our_this->bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];

		case 0x3c: // READ BUFFER
			our_this->bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return SCSILengthFromUINT16( &command[7] );

		default:
			return SCSIBase( &SCSIClassCr589, SCSIOP_EXEC_COMMAND, scsiInstance, 0, NULL );
	}
}

static void cr589_read_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	SCSICr589 *our_this = (SCSICr589 *)SCSIThis( &SCSIClassCr589, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		case 0x12: // INQUIRY
			SCSIBase( &SCSIClassCr589, SCSIOP_READ_DATA, scsiInstance, dataLength, data );

			if( our_this->download )
			{
				memcpy( &data[ 8 ], download_identity, 28 );
			}
			else
			{
				memcpy( &data[ 8 ], &our_this->buffer[ identity_offset ], 28 );
			}
			break;

		case 0x3c: // READ BUFFER
			memcpy( data, &our_this->buffer[ our_this->bufferOffset ], dataLength );
			our_this->bufferOffset += dataLength;
			break;

		default:
			SCSIBase( &SCSIClassCr589, SCSIOP_READ_DATA, scsiInstance, dataLength, data );
			break;
	}
}

static void cr589_write_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	SCSICr589 *our_this = (SCSICr589 *)SCSIThis( &SCSIClassCr589, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			memcpy( &our_this->buffer[ our_this->bufferOffset ], data + 32, dataLength - 32 );
			our_this->bufferOffset += dataLength;
			break;

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			if( memcmp( data, &our_this->buffer[ identity_offset ], 28 ) == 0 )
			{
				our_this->download = 1;
			}
			else if( memcmp( data, download_identity, 28 ) == 0 )
			{
				our_this->download = 0;
			}
			break;

		default:
			SCSIBase( &SCSIClassCr589, SCSIOP_WRITE_DATA, scsiInstance, dataLength, data );
			break;
	}
}

static void cr589_alloc_instance( SCSIInstance *scsiInstance, const char *diskregion )
{
	running_machine *machine = scsiInstance->machine;
	SCSICr589 *our_this = (SCSICr589 *)SCSIThis( &SCSIClassCr589, scsiInstance );

	our_this->download = 0;
	memcpy( &our_this->buffer[ identity_offset ], "MATSHITACD-ROM CR-589   GS0N", 28 );

	state_save_register_item( machine,  "cr589", diskregion, 0, our_this->download );
	state_save_register_item_array( machine,  "cr589", diskregion, 0, our_this->buffer );
	state_save_register_item( machine,  "cr589", diskregion, 0, our_this->bufferOffset );
}

static int cr589_dispatch( int operation, void *file, INT64 intparm, void *ptrparm )
{
	SCSIAllocInstanceParams *params;

	switch( operation )
	{
		case SCSIOP_EXEC_COMMAND:
			return cr589_exec_command( (SCSIInstance *)file, (UINT8 *)ptrparm );

		case SCSIOP_READ_DATA:
			cr589_read_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_WRITE_DATA:
			cr589_write_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_ALLOC_INSTANCE:
			params = (SCSIAllocInstanceParams *)ptrparm;
			SCSIBase( &SCSIClassCr589, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
			cr589_alloc_instance( params->instance, params->diskregion );
			return 0;
	}

	return SCSIBase( &SCSIClassCr589, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
}

const SCSIClass SCSIClassCr589 =
{
	&SCSIClassCDROM,
	cr589_dispatch,
	sizeof( SCSICr589 )
};
