// license:BSD-3-Clause
// copyright-holders:smf
#include "t10sbc.h"

void t10sbc::t10_start(device_t &device)
{
	m_device = &device;
	t10spc::t10_start(device);

	device.save_item( NAME( m_lba ) );
	device.save_item( NAME( m_blocks ) );
}

void t10sbc::t10_reset()
{
	t10spc::t10_reset();

	m_lba = 0;
	m_blocks = 0;
	m_sector_bytes = 512;

	m_disk = m_image->get_hard_disk_file();
	if (!m_disk)
	{
		m_device->logerror("T10SBC %s: no HD found!\n", m_image->owner()->tag());
	}
	else
	{
		// get hard disk sector size from CHD metadata
		const hard_disk_info *hdinfo = hard_disk_get_info(m_disk);
		m_sector_bytes = hdinfo->sectorbytes;
	}
}

// scsihd_exec_command
void t10sbc::ExecCommand()
{
	switch ( command[0] )
	{
	case T10SBC_CMD_FORMAT_UNIT:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SBC_CMD_SEEK_6:
		m_lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];

		m_device->logerror("S1410: SEEK to LBA %x\n", m_lba);

		m_phase = SCSI_PHASE_STATUS;
		m_transfer_length = 0;
		break;

	case T10SBC_CMD_READ_6:
		m_lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
		m_blocks = SCSILengthFromUINT8( &command[4] );

		m_device->logerror("T10SBC: READ at LBA %x for %x blocks\n", m_lba, m_blocks);

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10SBC_CMD_WRITE_6:
		m_lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];
		m_blocks = SCSILengthFromUINT8( &command[4] );

		m_device->logerror("T10SBC: WRITE to LBA %x for %x blocks\n", m_lba, m_blocks);

		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10SPC_CMD_INQUIRY:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case T10SPC_CMD_MODE_SELECT_6:
		m_device->logerror("T10SBC: MODE SELECT length %x control %x\n", command[4], command[5]);
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case T10SPC_CMD_MODE_SENSE_6:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case T10SBC_CMD_READ_CAPACITY:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 8;
		break;

	case T10SBC_CMD_READ_10:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = SCSILengthFromUINT16( &command[7] );

		m_device->logerror("T10SBC: READ at LBA %x for %x blocks\n", m_lba, m_blocks);

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10SBC_CMD_WRITE_10:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = SCSILengthFromUINT16( &command[7] );

		m_device->logerror("T10SBC: WRITE to LBA %x for %x blocks\n", m_lba, m_blocks);

		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10SBC_CMD_READ_12:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

		m_device->logerror("T10SBC: READ at LBA %x for %x blocks\n", m_lba, m_blocks);

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	default:
		t10spc::ExecCommand();
		break;
	}
}

void t10sbc::ReadData( UINT8 *data, int dataLength )
{
	// if we're a drive without a disk, return all zeroes
	if (!m_disk)
	{
		memset(data, 0, dataLength);
		return;
	}

	switch ( command[0] )
	{
	case T10SPC_CMD_INQUIRY:
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

	case T10SPC_CMD_MODE_SENSE_6:
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

	case T10SBC_CMD_READ_6:
	case T10SBC_CMD_READ_10:
	case T10SBC_CMD_READ_12:
		if ((m_disk) && (m_blocks))
		{
			while (dataLength > 0)
			{
				if (!hard_disk_read(m_disk, m_lba,  data))
				{
					m_device->logerror("T10SBC: HD read error!\n");
				}
				m_lba++;
				m_blocks--;
				dataLength -= m_sector_bytes;
				data += m_sector_bytes;
			}
		}
		break;

	case T10SBC_CMD_READ_CAPACITY: // READ CAPACITY
		{
			hard_disk_info *info;
			UINT32 temp;

			info = hard_disk_get_info(m_disk);

			m_device->logerror("T10SBC: READ CAPACITY\n");

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
	if (!m_disk)
	{
		return;
	}

	switch ( command[0] )
	{
	case T10SPC_CMD_MODE_SELECT_6:
		break;

	case T10SBC_CMD_WRITE_6:
	case T10SBC_CMD_WRITE_10:
		if ((m_disk) && (m_blocks))
		{
			while (dataLength > 0)
			{
				if (!hard_disk_write(m_disk, m_lba, data))
				{
					m_device->logerror("T10SBC: HD write error!\n");
				}
				m_lba++;
				m_blocks--;
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
	*(hard_disk_file **)_disk = m_disk;
}

void t10sbc::SetDevice( void *_disk )
{
	m_disk = (hard_disk_file *)_disk;
}
