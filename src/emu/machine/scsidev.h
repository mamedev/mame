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
	virtual int GetDeviceID();

	// configuration helpers
	static void static_set_deviceid(device_t &device, int _scsiID);

protected:
	// device-level overrides
	virtual void device_start();

private:
	UINT8 command[16];
	int commandLength;
	int phase;
	int scsiID;
};

extern int SCSILengthFromUINT8( UINT8 *length );
extern int SCSILengthFromUINT16( UINT8 *length );

#define SCSI_PHASE_DATAOUT ( 0 )
#define SCSI_PHASE_DATAIN ( 1 )
#define SCSI_PHASE_COMMAND ( 2 )
#define SCSI_PHASE_STATUS ( 3 )
#define SCSI_PHASE_MESSAGE_OUT ( 6 )
#define SCSI_PHASE_MESSAGE_IN ( 7 )

// SCSI IDs
enum
{
	SCSI_ID_0 = 0,
	SCSI_ID_1,
	SCSI_ID_2,
	SCSI_ID_3,
	SCSI_ID_4,
	SCSI_ID_5,
	SCSI_ID_6,
	SCSI_ID_7
};

#define MCFG_SCSIDEV_ADD(_tag, _type, _id) \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	scsidev_device::static_set_deviceid(*device, _id);

#endif
