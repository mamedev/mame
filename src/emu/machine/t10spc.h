/***************************************************************************

t10spc.h

***************************************************************************/

#ifndef _T10SPC_H_
#define _T10SPC_H_

#include "emu.h"

class t10spc
{
public:
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

#define SCSI_CMD_TEST_UNIT_READY ( 0x00 )
#define SCSI_CMD_RECALIBRATE ( 0x01 )
#define SCSI_CMD_REQUEST_SENSE ( 0x03 )
#define SCSI_CMD_MODE_SELECT ( 0x15 )
#define SCSI_CMD_SEND_DIAGNOSTIC ( 0x1d )

#endif
