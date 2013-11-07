#include "t10sbc.h"

void t10sbc::t10_start(device_t &device)
{
	t10spc::t10_start(device);

	device.save_item( NAME( lba ) );
	device.save_item( NAME( blocks ) );
}

void t10sbc::t10_reset()
{
	t10spc::t10_reset();

	lba = 0;
	blocks = 0;
	m_sector_bytes = 512;

	disk = m_image->get_hard_disk_file();
	if (!disk)
	{
		logerror("T10SBC %s: no HD found!\n", m_image->owner()->tag());
	}
	else
	{
		// get hard disk sector size from CHD metadata
		const hard_disk_info *hdinfo = hard_disk_get_info(disk);
		m_sector_bytes = hdinfo->sectorbytes;
	}
}

// scsihd_exec_command
void t10sbc::ExecCommand()
{
	switch ( command[0] )
	{
		case 0x04: // FORMAT UNIT
			m_phase = SCSI_PHASE_STATUS;
			m_transfer_length = 0;
			break;

		case 0x08: // READ(6)
			lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			blocks = SCSILengthFromUINT8( &command[4] );

			logerror("T10SBC: READ at LBA %x for %x blocks\n", lba, blocks);

			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = blocks * m_sector_bytes;
			break;

		case 0x0a: // WRITE(6)
			lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
			blocks = SCSILengthFromUINT8( &command[4] );

			logerror("T10SBC: WRITE to LBA %x for %x blocks\n", lba, blocks);

			m_phase = SCSI_PHASE_DATAOUT;
			m_transfer_length = blocks * m_sector_bytes;
			break;

		case 0x12: // INQUIRY
			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x15: // MODE SELECT (used to set CDDA volume)
			logerror("T10SBC: MODE SELECT length %x control %x\n", command[4], command[5]);
			m_phase = SCSI_PHASE_DATAOUT;
			m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x1a: // MODE SENSE(6)
			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x25: // READ CAPACITY
			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = 8;
			break;

		case 0x28: // READ(10)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			logerror("T10SBC: READ at LBA %x for %x blocks\n", lba, blocks);

			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = blocks * m_sector_bytes;
			break;

		case 0x2a: // WRITE (10)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			logerror("T10SBC: WRITE to LBA %x for %x blocks\n", lba, blocks);

			m_phase = SCSI_PHASE_DATAOUT;
			m_transfer_length = blocks * m_sector_bytes;
			break;

		case 0xa8: // READ(12)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

			logerror("T10SBC: READ at LBA %x for %x blocks\n", lba, blocks);

			m_phase = SCSI_PHASE_DATAIN;
			m_transfer_length = blocks * m_sector_bytes;
			break;

		default:
			t10spc::ExecCommand();
			break;
	}
}

void t10sbc::ReadData( UINT8 *data, int dataLength )
{
	// if we're a drive without a disk, return all zeroes
	if (!disk)
	{
		memset(data, 0, dataLength);
		return;
	}

	switch ( command[0] )
	{
		case 0x12:  // INQUIRY
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

		case 0x1a:  // MODE SENSE (6 byte)
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
						logerror("T10SBC: HD read error!\n");
					}
					lba++;
					blocks--;
					dataLength -= m_sector_bytes;
					data += m_sector_bytes;
				}
			}
			break;


		case 0x25: // READ CAPACITY
			{
				hard_disk_info *info;
				UINT32 temp;

				info = hard_disk_get_info(disk);

				logerror("T10SBC: READ CAPACITY\n");

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
			t10spc::ReadData( data, dataLength );
			break;
	}
}

void t10sbc::WriteData( UINT8 *data, int dataLength )
{
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
						logerror("T10SBC: HD write error!\n");
					}
					lba++;
					blocks--;
					dataLength -= m_sector_bytes;
					data += m_sector_bytes;
				}
			}
			break;

		default:
			t10spc::WriteData( data, dataLength );
			break;
	}
}

void t10sbc::GetDevice( void **_disk )
{
	*(hard_disk_file **)_disk = disk;
}

void t10sbc::SetDevice( void *_disk )
{
	disk = (hard_disk_file *)_disk;
}
