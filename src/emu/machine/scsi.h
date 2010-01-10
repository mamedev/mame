/***************************************************************************

 scsi.h - Header which defines the interface between SCSI device handlers
          and SCSI interfaces.

***************************************************************************/

#ifndef _SCSI_H_
#define _SCSI_H_


typedef int (*pSCSIDispatch)( int operation, void *file, INT64 intparm, void *ptrparm );

typedef struct _SCSIClass
{
	const struct _SCSIClass *baseClass;
	pSCSIDispatch dispatch;
	int sizeofData;
} SCSIClass;

typedef struct
{
	const SCSIClass *scsiClass;
	running_machine *machine;
} SCSIInstance;

typedef struct
{
	SCSIInstance *instance;
	const char *diskregion;
	running_machine *machine;
} SCSIAllocInstanceParams;

// commands accepted by a SCSI device's dispatch handler
enum
{
	SCSIOP_EXEC_COMMAND = 0,	// execute a command packet
	SCSIOP_SET_COMMAND,			// set a command packet
	SCSIOP_GET_COMMAND,			// get a command packet
	SCSIOP_READ_DATA,			// data transfer from the device
	SCSIOP_WRITE_DATA,			// data transfer to the device
	SCSIOP_ALLOC_INSTANCE,		// allocate an instance of the device
	SCSIOP_DELETE_INSTANCE,		// delete an instance of the device
	SCSIOP_GET_DEVICE,			// get the device's internal device (CDROM or HDD pointer)
	SCSIOP_SET_DEVICE,			// set the device's internal device (CDROM or HDD pointer)
	SCSIOP_RESET_DEVICE,		// reset the device
	SCSIOP_SET_PHASE,
	SCSIOP_GET_PHASE,
};

typedef struct scsiconfigitem
{
	int scsiID;
	const char *diskregion;
	const SCSIClass *scsiClass;
} SCSIConfigItem;

#define SCSI_MAX_DEVICES	(16)

typedef struct scsiconfigtable
{
	int devs_present;
	const SCSIConfigItem devices[SCSI_MAX_DEVICES];
} SCSIConfigTable;

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


#define SCSI_PHASE_DATAOUT ( 0 )
#define SCSI_PHASE_DATAIN ( 1 )
#define SCSI_PHASE_COMMAND ( 2 )
#define SCSI_PHASE_STATUS ( 3 )
#define SCSI_PHASE_MESSAGE_OUT ( 6 )
#define SCSI_PHASE_MESSAGE_IN ( 7 )

extern void SCSIAllocInstance( running_machine *machine, const SCSIClass *scsiClass, SCSIInstance **instance, const char *diskregion );
extern void SCSIDeleteInstance( SCSIInstance *instance );
extern void SCSISetDevice( SCSIInstance *instance, void *device );
extern void SCSIGetDevice( SCSIInstance *instance, void **device );
extern void SCSIReset( SCSIInstance *instance );
extern void SCSISetCommand( SCSIInstance *instance, UINT8 *command, int commandLength );
extern void SCSIGetCommand( SCSIInstance *instance, UINT8 **command, int *commandLength );
extern void SCSIExecCommand( SCSIInstance *instance, int *resultLength );
extern void SCSIWriteData( SCSIInstance *instance, void *data, int dataLength );
extern void SCSIReadData( SCSIInstance *instance, void *data, int dataLength );
extern void SCSISetPhase( SCSIInstance *instance, int phase );
extern void SCSIGetPhase( SCSIInstance *instance, int *phase );

extern SCSIInstance *SCSIMalloc( running_machine *machine, const SCSIClass *scsiClass );
extern int SCSIBase( const SCSIClass *scsiClass, int operation, void *file, INT64 intparm, UINT8 *ptrparm );
extern void *SCSIThis( const SCSIClass *scsiClass, SCSIInstance *instance );
extern int SCSISizeof( const SCSIClass *scsiClass );
extern int SCSILengthFromUINT8( UINT8 *length );
extern int SCSILengthFromUINT16( UINT8 *length );

#endif

// include these here to avoid changing the drivers.
#include "machine/scsicd.h"
#include "machine/scsihd.h"
