#include "scsi.h"

void SCSIAllocInstance( SCSIClass *scsiClass, SCSIInstance **instance, int diskId )
{
	scsiClass->dispatch( SCSIOP_ALLOC_INSTANCE, scsiClass, diskId, instance );
}

void SCSIDeleteInstance( SCSIInstance *instance )
{
	instance->scsiClass->dispatch( SCSIOP_DELETE_INSTANCE, instance, 0, NULL );
}

void SCSISetDevice( SCSIInstance *instance, void *device )
{
	instance->scsiClass->dispatch( SCSIOP_SET_DEVICE, instance, 0, device );
}

void SCSIGetDevice( SCSIInstance *instance, void **device )
{
	instance->scsiClass->dispatch( SCSIOP_GET_DEVICE, instance, 0, device );
}

void SCSIReset( SCSIInstance *instance )
{
	instance->scsiClass->dispatch( SCSIOP_RESET_DEVICE, instance, 0, NULL );
}

void SCSIExecCommand( SCSIInstance *instance, int *resultLength )
{
	*resultLength = instance->scsiClass->dispatch( SCSIOP_EXEC_COMMAND, instance, 0, NULL );
}

void SCSISetCommand( SCSIInstance *instance, UINT8 *command, int commandLength )
{
	instance->scsiClass->dispatch( SCSIOP_SET_COMMAND, instance, commandLength, command );
}

void SCSIGetCommand( SCSIInstance *instance, UINT8 **command, int *commandLength )
{
	*commandLength = instance->scsiClass->dispatch( SCSIOP_GET_COMMAND, instance, 0, command );
}

void SCSIWriteData( SCSIInstance *instance, void *data, int dataLength )
{
	instance->scsiClass->dispatch( SCSIOP_WRITE_DATA, instance, dataLength, data );
}

void SCSIReadData( SCSIInstance *instance, void *data, int dataLength )
{
	instance->scsiClass->dispatch( SCSIOP_READ_DATA, instance, dataLength, data );
}

void SCSISetPhase( SCSIInstance *instance, int phase )
{
	instance->scsiClass->dispatch( SCSIOP_SET_PHASE, instance, phase, NULL );
}

void SCSIGetPhase( SCSIInstance *instance, int *phase )
{
	*phase = instance->scsiClass->dispatch( SCSIOP_GET_PHASE, instance, 0, NULL );
}

int SCSIBase( SCSIClass *scsiClass, int operation, void *file, INT64 intparm, UINT8 *ptrparm )
{
	return scsiClass->baseClass->dispatch( operation, file, intparm, ptrparm );
}

SCSIInstance *SCSIMalloc( SCSIClass *scsiClass )
{
	SCSIInstance *scsiInstance = (SCSIInstance *) malloc_or_die( SCSISizeof( scsiClass ) );
	scsiInstance->scsiClass = scsiClass;
	return scsiInstance;
}

void *SCSIThis( SCSIClass *scsiClass, SCSIInstance *instance )
{
	if( instance != NULL )
	{
		int sizeofBase = sizeof( SCSIInstance );

		while( scsiClass->baseClass != NULL )
		{
			scsiClass = scsiClass->baseClass;
			sizeofBase += scsiClass->sizeofData;
		}

		return ( (UINT8*)instance ) + sizeofBase;
	}

	return NULL;
}

int SCSISizeof( SCSIClass *scsiClass )
{
	int sizeofData = sizeof( SCSIInstance );

	while( scsiClass != NULL )
	{
		sizeofData += scsiClass->sizeofData;
		scsiClass = scsiClass->baseClass;
	}

	return sizeofData;
}

int SCSILengthFromUINT8( UINT8 *length )
{
	if( *length == 0 )
	{
		return 256;
	}

	return *length;
}

int SCSILengthFromUINT16( UINT8 *length )
{
	return ( *(length) << 8 ) | *(length + 1 );
}
