#include "t10spc.h"

#define SCSI_SENSE_SIZE             4

void t10spc::t10_start(device_t &device)
{
	device.save_item(NAME(command));
	device.save_item(NAME(commandLength));
	device.save_item(NAME(m_phase));
	device.save_item(NAME(m_transfer_length));
}

void t10spc::t10_reset()
{
	m_phase = SCSI_PHASE_BUS_FREE;
}

void t10spc::ExecCommand()
{
	switch( command[ 0 ] )
	{
	case SCSI_CMD_TEST_UNIT_READY:
		m_phase = SCSI_PHASE_STATUS;
		m_transfer_length = 0;
		break;

	case SCSI_CMD_RECALIBRATE:
		m_phase = SCSI_PHASE_STATUS;
		m_transfer_length = 0;
		break;

	case SCSI_CMD_REQUEST_SENSE:
		m_phase = SCSI_PHASE_DATAOUT;
		m_transfer_length = SCSI_SENSE_SIZE;
		break;

	case SCSI_CMD_SEND_DIAGNOSTIC:
		m_phase = SCSI_PHASE_DATAOUT;
		m_transfer_length = SCSILengthFromUINT16(&command[3]);
		break;

	default:
		logerror( "SCSIDEV unknown command %02x\n", command[ 0 ] );
		m_transfer_length = 0;
		break;
	}
}

void t10spc::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case SCSI_CMD_REQUEST_SENSE:
		data[ 0 ] = SCSI_SENSE_NO_SENSE;
		data[ 1 ] = 0x00;
		data[ 2 ] = 0x00;
		data[ 3 ] = 0x00;
		break;
	default:
		logerror( "SCSIDEV unknown read %02x\n", command[ 0 ] );
		break;
	}
}

void t10spc::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case SCSI_CMD_SEND_DIAGNOSTIC:
		break;

	default:
		logerror( "SCSIDEV unknown write %02x\n", command[ 0 ] );
		break;
	}
}

void t10spc::SetCommand( UINT8 *_command, int _commandLength )
{
	if( _commandLength > sizeof( command ) )
	{
		/// TODO: output an error.
		return;
	}

	memcpy( command, _command, _commandLength );
	commandLength = _commandLength;

	m_phase = SCSI_PHASE_COMMAND;
}
