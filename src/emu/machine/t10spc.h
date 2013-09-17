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

	UINT8 command[ 32 ];
	int commandLength;
	int m_transfer_length;
	int m_phase;
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

#define SCSI_SENSE_ADDR_VALID       0x80
#define SCSI_SENSE_NO_SENSE         0x00
#define SCSI_SENSE_NO_INDEX         0x01
#define SCSI_SENSE_SEEK_NOT_COMP    0x02
#define SCSI_SENSE_WRITE_FAULT      0x03
#define SCSI_SENSE_DRIVE_NOT_READY  0x04
#define SCSI_SENSE_NO_TRACK0        0x06
#define SCSI_SENSE_ID_CRC_ERROR     0x10
#define SCSI_SENSE_UNCORRECTABLE    0x11
#define SCSI_SENSE_ADDRESS_NF       0x12
#define SCSI_SENSE_RECORD_NOT_FOUND 0x14
#define SCSI_SENSE_SEEK_ERROR       0x15
#define SCSI_SENSE_DATA_CHECK_RETRY 0x18
#define SCSI_SENSE_ECC_VERIFY       0x19
#define SCSI_SENSE_INTERLEAVE_ERROR 0x1A
#define SCSI_SENSE_UNFORMATTED      0x1C
#define SCSI_SENSE_ILLEGAL_COMMAND  0x20
#define SCSI_SENSE_ILLEGAL_ADDRESS  0x21
#define SCSI_SENSE_VOLUME_OVERFLOW  0x23
#define SCSI_SENSE_BAD_ARGUMENT     0x24
#define SCSI_SENSE_INVALID_LUN      0x25
#define SCSI_SENSE_CART_CHANGED     0x28
#define SCSI_SENSE_ERROR_OVERFLOW   0x2C

#endif
