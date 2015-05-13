// license:BSD-3-Clause
// copyright-holders:smf
#include "omti5100.h"

#define OMTI_STATUS_NOT_READY 0x04
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
	m_image = m_image0;
	scsihle_device::device_start();
}

void omti5100_device::ExecCommand()
{
	harddisk_image_device *image = ((command[1] >> 5) & 1) ? m_image1 : m_image0;
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
		default:
			if(!image)
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_sense_asc = OMTI_STATUS_NOT_READY;
				m_transfer_length = 0;
			}
			else
			{
				SetDevice(image);
				scsihd_device::ExecCommand();
			}
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

static MACHINE_CONFIG_FRAGMENT( omti5100 )
	MCFG_HARDDISK_ADD("image0")
	MCFG_HARDDISK_ADD("image1")
MACHINE_CONFIG_END


machine_config_constructor omti5100_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( omti5100 );
}
