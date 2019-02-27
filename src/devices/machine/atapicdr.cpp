// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "atapicdr.h"

#define SCSI_SENSE_ASC_MEDIUM_NOT_PRESENT 0x3a
#define SCSI_SENSE_ASC_NOT_READY_TO_READY_TRANSITION 0x28
#define T10MMC_GET_EVENT_STATUS_NOTIFICATION 0x4a

// device type definition
DEFINE_DEVICE_TYPE(ATAPI_CDROM,       atapi_cdrom_device,       "cdrom",       "ATAPI CD-ROM")
DEFINE_DEVICE_TYPE(ATAPI_FIXED_CDROM, atapi_fixed_cdrom_device, "cdrom_fixed", "ATAPI fixed CD-ROM")

atapi_cdrom_device::atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, ATAPI_CDROM, tag, owner, clock)
{
}

atapi_cdrom_device::atapi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	atapi_hle_device(mconfig, type, tag, owner, clock)
{
}

atapi_fixed_cdrom_device::atapi_fixed_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, ATAPI_FIXED_CDROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atapi_cdrom_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, "image").set_interface("cdrom");
	CDDA(config, "cdda");
}

void atapi_cdrom_device::device_start()
{
	m_image = subdevice<cdrom_image_device>("image");
	m_cdda = subdevice<cdda_device>("cdda");

	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[ 0 ] = 0x8500; // ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command

	m_identify_buffer[ 23 ] = ('1' << 8) | '.';
	m_identify_buffer[ 24 ] = ('0' << 8) | ' ';
	m_identify_buffer[ 25 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 26 ] = (' ' << 8) | ' ';

	m_identify_buffer[ 27 ] = ('M' << 8) | 'A';
	m_identify_buffer[ 28 ] = ('M' << 8) | 'E';
	m_identify_buffer[ 29 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 30 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 31 ] = ('V' << 8) | 'i';
	m_identify_buffer[ 32 ] = ('r' << 8) | 't';
	m_identify_buffer[ 33 ] = ('u' << 8) | 'a';
	m_identify_buffer[ 34 ] = ('l' << 8) | ' ';
	m_identify_buffer[ 35 ] = ('C' << 8) | 'D';
	m_identify_buffer[ 36 ] = ('R' << 8) | 'O';
	m_identify_buffer[ 37 ] = ('M' << 8) | ' ';
	m_identify_buffer[ 38 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 39 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 40 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 41 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 42 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 43 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 44 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 45 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 46 ] = (' ' << 8) | ' ';

	m_identify_buffer[ 49 ] = 0x0600; // Word 49=Capabilities, IORDY may be disabled (bit_10), LBA Supported mandatory (bit_9)

	atapi_hle_device::device_start();
}

void atapi_cdrom_device::device_reset()
{
	atapi_hle_device::device_reset();
	m_media_change = true;
}

void atapi_fixed_cdrom_device::device_reset()
{
	atapi_hle_device::device_reset();
	m_cdrom = m_image->get_cdrom_file();
	m_media_change = false;
}

void atapi_cdrom_device::process_buffer()
{
	if(m_cdrom != m_image->get_cdrom_file())
	{
		m_media_change = true;
		SetDevice(m_image->get_cdrom_file());
	}
	atapi_hle_device::process_buffer();
}

void atapi_cdrom_device::perform_diagnostic()
{
	m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
}

void atapi_cdrom_device::identify_packet_device()
{
}

void atapi_cdrom_device::ExecCommand()
{
	switch(command[0])
	{
		case T10SBC_CMD_READ_CAPACITY:
		case T10SBC_CMD_READ_10:
		case T10MMC_CMD_READ_SUB_CHANNEL:
		case T10MMC_CMD_READ_TOC_PMA_ATIP:
		case T10MMC_CMD_PLAY_AUDIO_10:
		case T10MMC_CMD_PLAY_AUDIO_TRACK_INDEX:
		case T10MMC_CMD_PAUSE_RESUME:
		case T10MMC_CMD_PLAY_AUDIO_12:
		case T10SBC_CMD_READ_12:
			if(!m_cdrom)
			{
				m_phase = SCSI_PHASE_STATUS;
				m_sense_key = SCSI_SENSE_KEY_MEDIUM_ERROR;
				m_sense_asc = SCSI_SENSE_ASC_MEDIUM_NOT_PRESENT;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				return;
			}
		default:
			if(m_media_change)
			{
				m_phase = SCSI_PHASE_STATUS;
				m_sense_key = SCSI_SENSE_KEY_UNIT_ATTENTION;
				m_sense_asc = SCSI_SENSE_ASC_NOT_READY_TO_READY_TRANSITION;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				return;
			}
			break;
		case T10SPC_CMD_INQUIRY:
			break;
		case T10SPC_CMD_REQUEST_SENSE:
			m_media_change = false;
			break;
	}
	t10mmc::ExecCommand();
}
