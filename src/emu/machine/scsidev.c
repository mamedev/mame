/***************************************************************************

 scsidev.c - Base class for scsi devices.

***************************************************************************/

#include "emu.h"
#include "scsidev.h"

scsidev_device::scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void scsidev_device::device_start()
{
	save_item( NAME( command ) );
	save_item( NAME( commandLength ) );
	save_item( NAME( phase ) );
}

void scsidev_device::ExecCommand( int *transferLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
		case 0x00: // TEST UNIT READY
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		default:
			logerror( "%s: SCSIDEV unknown command %02x\n", machine().describe_context(), command[ 0 ] );
			*transferLength = 0;
			break;
	}
}

void scsidev_device::ReadData( UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
		default:
			logerror( "%s: SCSIDEV unknown read %02x\n", machine().describe_context(), command[ 0 ] );
			break;
	}
}

void scsidev_device::WriteData( UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
		default:
			logerror( "%s: SCSIDEV unknown write %02x\n", machine().describe_context(), command[ 0 ] );
			break;
	}
}

void scsidev_device::SetPhase( int _phase )
{
	phase = _phase;
}

void scsidev_device::GetPhase( int *_phase)
{
	*_phase = phase;
}

void scsidev_device::SetCommand( UINT8 *_command, int _commandLength )
{
	if( _commandLength > sizeof( command ) )
	{
		/// TODO: output an error.
		return;
	}

	memcpy( command, _command, _commandLength );
	commandLength = _commandLength;

	SetPhase( SCSI_PHASE_COMMAND );
}

void scsidev_device::GetCommand( UINT8 **_command, int *_commandLength )
{
	*_command = command;
	*_commandLength = commandLength;
}
