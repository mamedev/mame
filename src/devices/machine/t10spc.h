// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

t10spc.h

***************************************************************************/

#ifndef _T10SPC_H_
#define _T10SPC_H_

#include "emu.h"

class t10spc
{
public:
	t10spc() : commandLength(0), m_transfer_length(0), m_phase(0), m_status_code(), m_sense_key(0), m_sense_asc(0), m_sense_ascq(0), m_sense_information(0), m_sector_bytes(0), m_device(nullptr)
	{
	}

	virtual ~t10spc() {};
	virtual void SetDevice( void *device ) = 0;
	virtual void GetDevice( void **device ) = 0;

	virtual void SetCommand( UINT8 *command, int commandLength );
	virtual void ExecCommand();
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );
	void GetLength( int *transferLength ) { *transferLength = m_transfer_length; }
	virtual void GetPhase( int *phase ) { *phase = m_phase; }

protected:
	virtual void t10_start(device_t &device);
	virtual void t10_reset();

	int SCSILengthFromUINT8( UINT8 *length ) { if( *length == 0 ) { return 256; } return *length; }
	int SCSILengthFromUINT16( UINT8 *length ) { return ( *(length) << 8 ) | *(length + 1 ); }

	enum sense_key_t
	{
		SCSI_SENSE_KEY_NO_SENSE = 0,
		SCSI_SENSE_KEY_ILLEGAL_REQUEST = 5
	};

	enum sense_asc_ascq_t
	{
		SCSI_SENSE_ASC_ASCQ_NO_SENSE = 0x0,
		SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS = 0x0011,
		SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_PAUSED = 0x0012,
		SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED = 0x0013,
		SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_STOPPED_DUE_TO_ERROR = 0x0014,
		SCSI_SENSE_ASC_ASCQ_ILLEGAL_MODE_FOR_THIS_TRACK = 0x6400
	};

	enum status_code_t
	{
		SCSI_STATUS_CODE_GOOD = 0x00,
		SCSI_STATUS_CODE_CHECK_CONDITION = 0x02,
		SCSI_STATUS_CODE_CONDITION_MET = 0x04,
		SCSI_STATUS_CODE_BUSY = 0x08,
		SCSI_STATUS_CODE_INTERMEDIATE = 0x14,
		SCSI_STATUS_CODE_RESERVATION_CONFLICT = 0x18,
		SCSI_STATUS_CODE_COMMAND_TERMINATED = 0x22,
		SCSI_STATUS_CODE_TASK_SET_FULL = 0x28,
		SCSI_STATUS_CODE_ACA_ACTIVE = 0x30,
		SCSI_STATUS_CODE_TASK_ABORTED = 0x40
	};

	// these are defined here because t10mmc also needs them.
	enum
	{
		T10SBC_CMD_FORMAT_UNIT = 0x04,
		T10SBC_CMD_READ_6 = 0x08,
		T10SBC_CMD_WRITE_6 = 0x0a,
		T10SBC_CMD_SEEK_6 = 0x0b,
		T10SBC_CMD_READ_10 = 0x28,
		T10SBC_CMD_READ_CAPACITY = 0x25,
		T10SBC_CMD_WRITE_10 = 0x2a,
		T10SBC_CMD_SEEK_10 = 0x2b,
		T10SBC_CMD_READ_12 = 0xa8
	};

	enum
	{
		T10SPC_CMD_TEST_UNIT_READY = 0x00,
		T10SPC_CMD_RECALIBRATE = 0x01,
		T10SPC_CMD_REQUEST_SENSE = 0x03,
		T10SPC_CMD_INQUIRY = 0x12,
		T10SPC_CMD_MODE_SELECT_6 = 0x15,
		T10SPC_CMD_RESERVE_6 = 0x16,
		T10SPC_CMD_RELEASE_6 = 0x17,
		T10SPC_CMD_MODE_SENSE_6 = 0x1a,
		T10SPC_CMD_START_STOP_UNIT = 0x1b,
		T10SPC_CMD_RECEIVE_DIAGNOSTIC_RESULTS = 0x1c,
		T10SPC_CMD_SEND_DIAGNOSTIC = 0x1d,
		T10SPC_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL = 0x1e,
		T10SPC_CMD_WRITE_BUFFER = 0x3b,
		T10SPC_CMD_READ_BUFFER = 0x3c,
		T10SPC_CMD_LOG_SELECT = 0x4c,
		T10SPC_CMD_LOG_SENSE = 0x4d,
		T10SPC_CMD_MODE_SELECT_10 = 0x55,
		T10SPC_CMD_RESERVE_10 = 0x56,
		T10SPC_CMD_RELEASE_10 = 0x57,
		T10SPC_CMD_MODE_SENSE_10 = 0x5a,
		T10SPC_CMD_PERSISTENT_RESERVE_IN = 0x5e,
		T10SPC_CMD_PERSISTENT_RESERVE_OUT = 0x5f,
		T10SPC_CMD_EXTENDED_COPY = 0x83,
		T10SPC_CMD_RECEIVE_COPY_RESULTS = 0x84,
		T10SPC_CMD_REPORT_LUNS = 0xa0,
		T10SPC_CMD_REPORT_DEVICE_IDENTIFIER = 0xa3,
		T10SPC_CMD_SET_DEVICE_IDENTIFIER = 0xa4,
		T10SPC_CMD_MOVE_MEDIUM_ATTACHED = 0xa7,
		T10SPC_CMD_READ_ELEMENT_STATUS_ATTACHED = 0xb4
	};

	void set_sense(sense_key_t key, sense_asc_ascq_t asc_ascq);

	UINT8 command[ 32 ];
	int commandLength;
	int m_transfer_length;
	int m_phase;
	status_code_t m_status_code;
	UINT8 m_sense_key;
	UINT8 m_sense_asc;
	UINT8 m_sense_ascq;
	UINT32 m_sense_information;
	int m_sector_bytes;
	device_t *m_device;
};

#define SCSI_PHASE_DATAOUT ( 0 )
#define SCSI_PHASE_DATAIN ( 1 )
#define SCSI_PHASE_COMMAND ( 2 )
#define SCSI_PHASE_STATUS ( 3 )
#define SCSI_PHASE_MESSAGE_OUT ( 6 )
#define SCSI_PHASE_MESSAGE_IN ( 7 )
#define SCSI_PHASE_BUS_FREE ( 8 )
#define SCSI_PHASE_SELECT ( 9 )

#endif
