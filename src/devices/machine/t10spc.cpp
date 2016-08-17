// license:BSD-3-Clause
// copyright-holders:smf
#include "t10spc.h"

void t10spc::t10_start(device_t &device)
{
	m_device = &device;
	device.save_item(NAME(command));
	device.save_item(NAME(commandLength));
	device.save_item(NAME(m_transfer_length));
	device.save_item(NAME(m_phase));
	device.save_item(NAME(m_sense_key));
	device.save_item(NAME(m_sense_asc));
	device.save_item(NAME(m_sense_ascq));
	device.save_item(NAME(m_sense_information));
}

void t10spc::t10_reset()
{
	m_phase = SCSI_PHASE_BUS_FREE;
	m_status_code = SCSI_STATUS_CODE_GOOD;
	m_sense_key = 0;
	m_sense_asc = 0;
	m_sense_ascq = 0;
	m_sense_information = 0;
}

void t10spc::set_sense(sense_key_t key, sense_asc_ascq_t asc_ascq)
{
	m_sense_key = key;
	m_sense_asc = (asc_ascq >> 8) & 0xff;
	m_sense_ascq = asc_ascq & 0xff;
	m_sense_information = 0;
}

void t10spc::ExecCommand()
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_TEST_UNIT_READY:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SPC_CMD_RECALIBRATE:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SPC_CMD_REQUEST_SENSE:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		if (command[4] == 0)
		{
			m_transfer_length = 4;
		}
		else if (command[4] > 18)
		{
			m_transfer_length = 18;
		}
		else
		{
			m_transfer_length = command[ 4 ];
		}
		break;

	case T10SPC_CMD_SEND_DIAGNOSTIC:
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT16(&command[3]);
		break;

	default:
		m_device->logerror( "SCSIDEV unknown command %02x\n", command[ 0 ] );
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;
	}
}

void t10spc::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_REQUEST_SENSE:
		if (command[4] == 0)
		{
			data[0] = m_sense_asc & 0x7f;
			data[1] = (m_sense_information >> 16) & 0x1f;
			data[2] = (m_sense_information >> 8) & 0xff;
			data[3] = (m_sense_information >> 0) & 0xff;
		}
		else
		{
			data[0] = 0x70;
			data[1] = 0;
			data[2] = m_sense_key & 0xf;
			data[3] = (m_sense_information >> 24) & 0xff;
			data[4] = (m_sense_information >> 16) & 0xff;
			data[5] = (m_sense_information >> 8) & 0xff;
			data[6] = (m_sense_information >> 0) & 0xff;
			data[7] = 10;
			data[8] = 0;
			data[9] = 0;
			data[10] = 0;
			data[11] = 0;
			data[12] = m_sense_asc;
			data[13] = m_sense_ascq;
			data[14] = 0;
			data[15] = 0;
			data[16] = 0;
			data[17] = 0;
		}

		set_sense(SCSI_SENSE_KEY_NO_SENSE, SCSI_SENSE_ASC_ASCQ_NO_SENSE);
		break;

	default:
		m_device->logerror( "SCSIDEV unknown read %02x\n", command[ 0 ] );
		break;
	}
}

void t10spc::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_SEND_DIAGNOSTIC:
		break;

	default:
		m_device->logerror( "SCSIDEV unknown write %02x\n", command[ 0 ] );
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
