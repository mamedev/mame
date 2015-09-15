// license:BSD-3-Clause
// copyright-holders:smf
#include "omti5100.h"

#define OMTI_STATUS_SEEK_FAIL 0x02
#define OMTI_STATUS_NOT_READY 0x04
#define OMTI_FORMAT_TRACK 0x06
#define OMTI_READ_DATA_BUFFER 0xec
#define OMTI_ASSIGN_DISK_PARAM 0xc2

const device_type OMTI5100 = &device_creator<omti5100_device>;

#if 0
ROM_START( omti5100 )
	ROM_REGION(0x1000, "mcu", 0) // Hitachi Z8
	ROM_LOAD("100240-N.7a", 0x0000, 0x1000, CRC(d227d6cb) SHA1(3d6140764d3d043428c941826370ebf1597c63bd))
ROM_END

const rom_entry *omti5100_device::device_rom_region() const
{
	return ROM_NAME( omti5100 );
}
#endif

omti5100_device::omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihd_device(mconfig, OMTI5100, "OMTI 5100", tag, owner, clock, "omti5100", __FILE__),
		m_image0(*this, "image0"),
		m_image1(*this, "image1")
{
}

void omti5100_device::device_start()
{
	if(!m_image0->get_hard_disk_file())
		m_image = m_image1;
	m_image = m_image0;
	scsihle_device::device_start();
}

void omti5100_device::ExecCommand()
{
	int drive = (command[1] >> 5) & 1;
	hard_disk_file *image = (drive ? m_image1 : m_image0)->get_hard_disk_file();
	if(!image)
	{
		if(command[0] == T10SPC_CMD_REQUEST_SENSE)
			return scsihd_device::ExecCommand();

		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
		m_sense_asc = OMTI_STATUS_NOT_READY;
		m_transfer_length = 0;
		return;
	}
	hard_disk_info *info = hard_disk_get_info(image);
	switch(command[0])
	{
		case OMTI_READ_DATA_BUFFER:
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 512;
			break;
		case OMTI_ASSIGN_DISK_PARAM:
			m_phase = SCSI_PHASE_DATAOUT;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 10;
			break;

		case T10SBC_CMD_READ_6:
		{
			int track = ((command[1]&0x1f)<<16 | command[2]<<8 | command[3]) / (m_param[drive].sectors ? m_param[drive].sectors : 1);
			int heads = m_param[drive].heads ? m_param[drive].heads : 1;
			if(((track % heads) > info->heads) || (track >= (info->cylinders * heads)))
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_sense_asc = OMTI_STATUS_SEEK_FAIL;
				m_transfer_length = 0;
			}
			else
			{
				SetDevice(image);
				scsihd_device::ExecCommand();
			}
			break;
		}
		case OMTI_FORMAT_TRACK:
		{
			int track = ((command[1]&0x1f)<<16 | command[2]<<8 | command[3]) / m_param[drive].sectors;
			if(((track % m_param[drive].heads) <= info->heads) && (track < (info->cylinders * m_param[drive].heads)))
			{
				dynamic_buffer sector(info->sectorbytes);
				memset(&sector[0], 0xe5, info->sectorbytes);
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_GOOD;
				m_transfer_length = 0;
				for(int i = 0; i < info->sectors; i++)
					hard_disk_write(image, track * info->sectors + i, &sector[0]);
			}
			else
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_sense_asc = OMTI_STATUS_SEEK_FAIL;
				m_transfer_length = 0;
			}
			break;
		}
		default:
			SetDevice(image);
			scsihd_device::ExecCommand();
			break;
	}
}

void omti5100_device::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case OMTI_READ_DATA_BUFFER:
			data[0] = '5';
			data[1] = '1';
			data[2] = '0';
			data[3] = '0';
			break;

		default:
			scsihd_device::ReadData( data, dataLength );
			break;
	}
}

void omti5100_device::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case OMTI_ASSIGN_DISK_PARAM:
		{
			int drive = ((command[1] >> 5) & 1);
			hard_disk_file *image = (drive ? m_image1 : m_image0)->get_hard_disk_file();
			m_param[drive].heads = data[3] + 1;
			m_param[drive].cylinders = ((data[4] << 8) | data[5]) + 1;
			if(!data[8] && image)
			{
				switch(hard_disk_get_info(image)->sectorbytes)
				{
					case 128:
						m_param[drive].sectors = 53;
						break;
					case 256:
						m_param[drive].sectors = 32;
						break;
					case 512:
						m_param[drive].sectors = 18; // XXX: check this!
						break;
					case 1024:
						m_param[drive].sectors = 9;
						break;
				}
			}
			else
				m_param[drive].sectors = data[8] + 1;
			break;
		}

		default:
			scsihd_device::WriteData( data, dataLength );
			break;
	}
}

static MACHINE_CONFIG_FRAGMENT( omti5100 )
	MCFG_HARDDISK_ADD("image0")
	MCFG_HARDDISK_ADD("image1")
MACHINE_CONFIG_END


machine_config_constructor omti5100_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( omti5100 );
}
