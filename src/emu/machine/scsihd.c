/***************************************************************************

 scsihd.c - Implementation of a SCSI hard disk drive

***************************************************************************/

#include "driver.h"
#include "scsidev.h"
#include "harddisk.h"

#ifdef MESS
#include "devices/harddriv.h"
#endif
#include "scsihd.h"

typedef struct
{
	UINT32 lba;
	UINT32 blocks;
 	hard_disk_file *disk;
} SCSIHd;


// scsihd_exec_command

static int scsihd_exec_command( SCSIInstance *scsiInstance, UINT8 *statusCode )
{
	UINT8 *command;
	int commandLength;
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch ( command[0] )
	{
		case 0x03: // REQUEST SENSE
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x04: // FORMAT UNIT
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x08: // READ(6)
			our_this->lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			our_this->blocks = SCSILengthFromUINT8( &command[4] );

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return our_this->blocks * 512;

		case 0x0a: // WRITE(6)
			our_this->lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			our_this->blocks = SCSILengthFromUINT8( &command[4] );

			logerror("SCSIHD: WRITE to LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return our_this->blocks * 512;

		case 0x12: // INQUIRY
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x15: // MODE SELECT (used to set CDDA volume)
			logerror("SCSIHD: MODE SELECT length %x control %x\n", command[4], command[5]);
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x1a: // MODE SENSE(6)
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x25: // READ CAPACITY
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return 8;

		case 0x28: // READ(10)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = SCSILengthFromUINT16( &command[7] );

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return our_this->blocks * 512;

		case 0x2a: // WRITE (10)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = SCSILengthFromUINT16( &command[7] );

			logerror("SCSIHD: WRITE to LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );

			return our_this->blocks * 512;

		case 0xa8: // READ(12)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return our_this->blocks * 512;

		default:
			return SCSIBase( &SCSIClassHARDDISK, SCSIOP_EXEC_COMMAND, scsiInstance, 0, NULL );
	}
}

static void scsihd_read_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	int i;
	UINT8 *command;
	int commandLength;
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch ( command[0] )
	{
		case 0x03:	// REQUEST SENSE
			data[0] = 0x80;	// valid sense
			for (i = 1; i < 12; i++)
			{
				data[i] = 0;
			}
			break;

		case 0x12:	// INQUIRY
			memset( data, 0, dataLength );
			data[0] = 0x00; // device is direct-access (e.g. hard disk)
			data[1] = 0x00; // media is not removable
			data[2] = 0x05; // device complies with SPC-3 standard
			data[3] = 0x02; // response data format = SPC-3 standard
			// Apple HD SC setup utility needs to see this
			strcpy((char *)&data[8], " SEAGATE");
			strcpy((char *)&data[16], "          ST225N");
			strcpy((char *)&data[32], "1.0");
			break;

		case 0x1a:	// MODE SENSE (6 byte)
			// special Apple ID page.  this is a vendor-specific page,
			// so unless collisions occur there should be no need
			// to change it.
			if ((command[2] & 0x3f) == 0x30)
			{
				memset(data, 0, 40);
				data[0] = 0x14;
				strcpy((char *)&data[14], "APPLE COMPUTER, INC.");
			}
			break;

		case 0x08: // READ(6)
		case 0x28: // READ(10)
		case 0xa8: // READ(12)
			if ((our_this->disk) && (our_this->blocks))
			{
				while (dataLength > 0)
				{
					if (!hard_disk_read(our_this->disk, our_this->lba,  data))
					{
						logerror("SCSIHD: HD read error!\n");
					}
					our_this->lba++;
					our_this->blocks--;
					dataLength -= 512;
					data += 512;
				}
			}
			break;


		case 0x25: // READ CAPACITY
			{
				hard_disk_info *info;
				UINT32 temp;

				info = hard_disk_get_info(our_this->disk);

				logerror("SCSIHD: READ CAPACITY\n");

				// get # of sectors
				temp = info->cylinders * info->heads * info->sectors;
				temp--;

				data[0] = (temp>>24) & 0xff;
				data[1] = (temp>>16) & 0xff;
				data[2] = (temp>>8) & 0xff;
				data[3] = (temp & 0xff);
				data[4] = (info->sectorbytes>>24)&0xff;
				data[5] = (info->sectorbytes>>16)&0xff;
				data[6] = (info->sectorbytes>>8)&0xff;
				data[7] = (info->sectorbytes & 0xff);
			}
			break;

		default:
			SCSIBase( &SCSIClassHARDDISK, SCSIOP_READ_DATA, scsiInstance, dataLength, data );
			break;
	}
}

static void scsihd_write_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch ( command[0] )
	{
		case 0x0a: // WRITE(6)
			if ((our_this->disk) && (our_this->blocks))
			{
				while (dataLength > 0)
				{
					if (!hard_disk_write(our_this->disk, our_this->lba, data))
					{
						logerror("SCSIHD: HD write error!\n");
					}
					our_this->lba++;
					our_this->blocks--;
					dataLength -= 512;
					data += 512;
				}
			}
			break;

		default:
			SCSIBase( &SCSIClassHARDDISK, SCSIOP_WRITE_DATA, scsiInstance, dataLength, data );
			break;
	}
}

static void scsihd_alloc_instance( SCSIInstance *scsiInstance, const char *diskregion )
{
	running_machine *machine = scsiInstance->machine;
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );

	our_this->lba = 0;
	our_this->blocks = 0;

	state_save_register_item( machine, "scsihd", diskregion, 0, our_this->lba );
	state_save_register_item( machine, "scsihd", diskregion, 0, our_this->blocks );

#ifdef MESS
	/* TODO: get rid of this ifdef MESS section */
	our_this->disk = mess_hd_get_hard_disk_file( devtag_get_device( machine, diskregion ) );
#else
	our_this->disk = hard_disk_open(get_disk_handle( machine, diskregion ));
#endif

	if (!our_this->disk)
	{
		logerror("SCSIHD: no HD found!\n");
	}
}

static void scsihd_delete_instance( SCSIInstance *scsiInstance )
{
#ifndef MESS
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );

	if( our_this->disk )
	{
		hard_disk_close( our_this->disk );
	}
#endif
}

static void scsihd_get_device( SCSIInstance *scsiInstance, hard_disk_file **disk )
{
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );
	*disk = our_this->disk;
}

static void scsihd_set_device( SCSIInstance *scsiInstance, hard_disk_file *disk )
{
	SCSIHd *our_this = (SCSIHd *)SCSIThis( &SCSIClassHARDDISK, scsiInstance );
	our_this->disk = disk;
}

static int scsihd_dispatch(int operation, void *file, INT64 intparm, void *ptrparm)
{
	SCSIAllocInstanceParams *params;

	switch (operation)
	{
		case SCSIOP_EXEC_COMMAND:
			return scsihd_exec_command( (SCSIInstance *)file, (UINT8 *)ptrparm );

		case SCSIOP_READ_DATA:
			scsihd_read_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_WRITE_DATA:
			scsihd_write_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_ALLOC_INSTANCE:
			params = (SCSIAllocInstanceParams *)ptrparm;
			SCSIBase( &SCSIClassHARDDISK, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
			scsihd_alloc_instance( params->instance, params->diskregion );
			return 0;

		case SCSIOP_DELETE_INSTANCE:
			scsihd_delete_instance( (SCSIInstance *)file );
			break;

		case SCSIOP_GET_DEVICE:
			scsihd_get_device( (SCSIInstance *)file, (hard_disk_file **)ptrparm );
			return 0;

		case SCSIOP_SET_DEVICE:
			scsihd_set_device( (SCSIInstance *)file, (hard_disk_file *)ptrparm );
			return 0;
	}

	return SCSIBase( &SCSIClassHARDDISK, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
}

const SCSIClass SCSIClassHARDDISK =
{
	&SCSIClassDevice,
	scsihd_dispatch,
	sizeof( SCSIHd )
};
