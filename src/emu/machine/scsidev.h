/***************************************************************************

 scsidev.h

***************************************************************************/

#ifndef _SCSIDEV_H_
#define _SCSIDEV_H_

// base handler
class scsidev_device : public device_t
{
public:
	// construction/destruction
	scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual void SetDevice( void *device ) = 0;
	virtual void GetDevice( void **device ) = 0;
	virtual void SetCommand( UINT8 *command, int commandLength );
	virtual void GetCommand( UINT8 **command, int *commandLength );
	virtual void ExecCommand( int *transferLength );
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );
	virtual void SetPhase( int phase );
	virtual void GetPhase( int *phase );

protected:
	// device-level overrides
	virtual void device_start();

private:
	UINT8 command[16];
	int commandLength;
	int phase;
};

extern int SCSILengthFromUINT8( UINT8 *length );
extern int SCSILengthFromUINT16( UINT8 *length );

#define SCSI_PHASE_DATAOUT ( 0 )
#define SCSI_PHASE_DATAIN ( 1 )
#define SCSI_PHASE_COMMAND ( 2 )
#define SCSI_PHASE_STATUS ( 3 )
#define SCSI_PHASE_MESSAGE_OUT ( 6 )
#define SCSI_PHASE_MESSAGE_IN ( 7 )

#endif
