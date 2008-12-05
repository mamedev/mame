/***************************************************************************

 scsidev.c - Base class for scsi devices.

***************************************************************************/

#include "scsidev.h"
#include "deprecat.h"

typedef struct
{
	UINT8 command[16];
	int commandLength;
	int phase;
} SCSIDev;

static int scsidev_exec_command( SCSIInstance *scsiInstance, UINT8 *statusCode )
{
	UINT8 *command;
	int commandLength;
//  SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		case 0x00: // TEST UNIT READY
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		default:
			logerror( "%08x: SCSIDEV unknown command %02x\n", cpu_get_pc(Machine->activecpu), command[ 0 ] );
			return 0;
	}
}

static void scsidev_read_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
//  SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		default:
			logerror( "%08x: SCSIDEV unknown read %02x\n", cpu_get_pc(Machine->activecpu), command[ 0 ] );
			break;
	}
}

static void scsidev_write_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
//  SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch( command[ 0 ] )
	{
		default:
			logerror( "%08x: SCSIDEV unknown write %02x\n", cpu_get_pc(Machine->activecpu), command[ 0 ] );
			break;
	}
}

static void scsidev_set_phase( SCSIInstance *scsiInstance, int phase )
{
	SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	our_this->phase = phase;
}

static int scsidev_get_phase( SCSIInstance *scsiInstance )
{
	SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	return our_this->phase;
}

static void scsidev_set_command( SCSIInstance *scsiInstance, void *command, int commandLength )
{
	SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );

	if( commandLength > sizeof( our_this->command ) )
	{
		/// TODO: output an error.
		return;
	}

	memcpy( our_this->command, command, commandLength );
	our_this->commandLength = commandLength;

	SCSISetPhase( scsiInstance, SCSI_PHASE_COMMAND );
}

static int scsidev_get_command( SCSIInstance *scsiInstance, void **command )
{
	SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );
	*command = our_this->command;
	return our_this->commandLength;
}

static void scsidev_alloc_instance( SCSIInstance *scsiInstance, const char *diskregion )
{
	SCSIDev *our_this = SCSIThis( &SCSIClassDevice, scsiInstance );

	state_save_register_item_array( Machine, "scsidev", diskregion, 0, our_this->command );
	state_save_register_item( Machine, "scsidev", diskregion, 0, our_this->commandLength );
	state_save_register_item( Machine, "scsidev", diskregion, 0, our_this->phase );
}

static int scsidev_dispatch( int operation, void *file, INT64 intparm, void *ptrparm )
{
	SCSIAllocInstanceParams *params;

	switch( operation )
	{
		case SCSIOP_EXEC_COMMAND:
			return scsidev_exec_command( file, ptrparm );

		case SCSIOP_READ_DATA:
			scsidev_read_data( file, ptrparm, intparm );
			break;

		case SCSIOP_WRITE_DATA:
			scsidev_write_data( file, ptrparm, intparm );
			break;

		case SCSIOP_SET_PHASE:
			scsidev_set_phase( file, intparm );
			return 0;

		case SCSIOP_GET_PHASE:
			return scsidev_get_phase( file );

		case SCSIOP_SET_COMMAND:
			scsidev_set_command( file, ptrparm, intparm );
			return 0;

		case SCSIOP_GET_COMMAND:
			return scsidev_get_command( file, ptrparm );

		case SCSIOP_ALLOC_INSTANCE:
			params = ptrparm;
			params->instance = SCSIMalloc( file );
			scsidev_alloc_instance( params->instance, params->diskregion );
			return 0;

		case SCSIOP_DELETE_INSTANCE:
			free( file );
			return 0;
	}
	return 0;
}

const SCSIClass SCSIClassDevice =
{
	NULL,
	scsidev_dispatch,
	sizeof( SCSIDev )
};
