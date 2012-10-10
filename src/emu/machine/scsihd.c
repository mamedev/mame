/***************************************************************************

 scsihd.c - Implementation of a SCSI hard disk drive

***************************************************************************/

#include "emu.h"
#include "machine/scsihle.h"
#include "harddisk.h"
#include "imagedev/harddriv.h"
#include "scsihd.h"

// device type definition
const device_type SCSIHD = &device_creator<scsihd_device>;

scsihd_device::scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihle_device(mconfig, SCSIHD, "SCSIHD", tag, owner, clock)
{
}

scsihd_device::scsihd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	scsihle_device(mconfig, type, name, tag, owner, clock)
{
}

void scsihd_device::device_start()
{
	scsihle_device::device_start();

	save_item( NAME( lba ) );
	save_item( NAME( blocks ) );
}

void scsihd_device::device_reset()
{
	scsihle_device::device_reset();

	is_image_device = true;
	disk = subdevice<harddisk_image_device>("image")->get_hard_disk_file();
	if( !disk )
	{
		// try to locate the CHD from a DISK_REGION
		chd_file *handle = get_disk_handle(machine(), tag());
		if (handle != NULL)
		{
			is_image_device = false;
			disk = hard_disk_open(handle);
		}
	}

	lba = 0;
	blocks = 0;
	sectorbytes = 512;

	if (!disk)
	{
		logerror("%s SCSIHD: no HD found!\n", tag());
	}
	else
	{
		// get hard disk sector size from CHD metadata
		const hard_disk_info *hdinfo = hard_disk_get_info(disk);
		sectorbytes = hdinfo->sectorbytes;
	}
}

void scsihd_device::device_stop()
{
	if (!is_image_device)
	{
		if( disk )
		{
			hard_disk_close( disk );
		}
	}
}

static MACHINE_CONFIG_FRAGMENT(scsi_harddisk)
	MCFG_HARDDISK_ADD("image")
MACHINE_CONFIG_END

machine_config_constructor scsihd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_harddisk);
}

// scsihd_exec_command
void scsihd_device::ExecCommand( int *transferLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch ( command[0] )
	{
		case 0x03: // REQUEST SENSE
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x04: // FORMAT UNIT
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x08: // READ(6)
			lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			blocks = SCSILengthFromUINT8( &command[4] );

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", lba, blocks);

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = blocks * sectorbytes;
			break;

		case 0x0a: // WRITE(6)
			lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			blocks = SCSILengthFromUINT8( &command[4] );

			logerror("SCSIHD: WRITE to LBA %x for %x blocks\n", lba, blocks);

			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = blocks * sectorbytes;
			break;

		case 0x12: // INQUIRY
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x15: // MODE SELECT (used to set CDDA volume)
			logerror("SCSIHD: MODE SELECT length %x control %x\n", command[4], command[5]);
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x1a: // MODE SENSE(6)
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x25: // READ CAPACITY
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = 8;
			break;

		case 0x28: // READ(10)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", lba, blocks);

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = blocks * sectorbytes;
			break;

		case 0x2a: // WRITE (10)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			logerror("SCSIHD: WRITE to LBA %x for %x blocks\n", lba, blocks);

			SetPhase( SCSI_PHASE_DATAOUT );

			*transferLength = blocks * sectorbytes;
			break;

		case 0xa8: // READ(12)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

			logerror("SCSIHD: READ at LBA %x for %x blocks\n", lba, blocks);

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = blocks * sectorbytes;
			break;

		default:
			scsihle_device::ExecCommand( transferLength );
			break;
	}
}

void scsihd_device::ReadData( UINT8 *data, int dataLength )
{
	int i;
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	// if we're a drive without a disk, return all zeroes
	if (!disk)
	{
		memset(data, 0, dataLength);
		return;
	}

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
			if ((disk) && (blocks))
			{
				while (dataLength > 0)
				{
					if (!hard_disk_read(disk, lba,  data))
					{
						logerror("SCSIHD: HD read error!\n");
					}
					lba++;
					blocks--;
					dataLength -= sectorbytes;
					data += sectorbytes;
				}
			}
			break;


		case 0x25: // READ CAPACITY
			{
				hard_disk_info *info;
				UINT32 temp;

				info = hard_disk_get_info(disk);

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
			scsihle_device::ReadData( data, dataLength );
			break;
	}
}

void scsihd_device::WriteData( UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	if (!disk)
	{
		return;
	}

	switch ( command[0] )
	{
		case 0x0a: // WRITE(6)
		case 0x2a: // WRITE(10)
			if ((disk) && (blocks))
			{
				while (dataLength > 0)
				{
					if (!hard_disk_write(disk, lba, data))
					{
						logerror("SCSIHD: HD write error!\n");
					}
					lba++;
					blocks--;
					dataLength -= sectorbytes;
					data += sectorbytes;
				}
			}
			break;

		default:
			scsihle_device::WriteData( data, dataLength );
			break;
	}
}


void scsihd_device::GetDevice( void **_disk )
{
	*(hard_disk_file **)_disk = disk;
}

void scsihd_device::SetDevice( void *_disk )
{
	disk = (hard_disk_file *)_disk;
}

int scsihd_device::GetSectorBytes()
{
	return sectorbytes;
}
