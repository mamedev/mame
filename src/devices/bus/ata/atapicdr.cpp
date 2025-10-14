// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "atapicdr.h"
#include "gdrom.h"

#define SCSI_SENSE_ASC_MEDIUM_NOT_PRESENT 0x3a
#define SCSI_SENSE_ASC_NOT_READY_TO_READY_TRANSITION 0x28
#define T10MMC_GET_EVENT_STATUS_NOTIFICATION 0x4a

// device type definition
DEFINE_DEVICE_TYPE(ATAPI_CDROM,        atapi_cdrom_device,        "cdrom",        "ATAPI CD-ROM")
DEFINE_DEVICE_TYPE(ATAPI_DVDROM,       atapi_dvdrom_device,       "dvdrom",       "ATAPI CD/DVD-ROM")

atapi_cdrom_device::atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, ATAPI_CDROM, tag, owner, clock)
{
}

atapi_cdrom_device::atapi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	atapi_hle_device(mconfig, type, tag, owner, clock),
	device_ata_interface(mconfig, *this),
	m_ultra_dma_mode(0)
{
}

atapi_dvdrom_device::atapi_dvdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, ATAPI_DVDROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atapi_cdrom_device::device_add_mconfig(machine_config &config)
{
	if(type() == ATAPI_DVDROM)
		DVDROM(config, "image").set_interface("cdrom");
	else if(type() == ATAPI_GDROM)
		GDROM(config, "image").set_interface("cdrom");
	else
		CDROM(config, "image").set_interface("cdrom");
	CDDA(config, "cdda").set_cdrom_tag("image");
}

void atapi_cdrom_device::device_start()
{
	m_image = subdevice<cdrom_image_device>("image");
	m_cdda = subdevice<cdda_device>("cdda");

	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[0] = 0x8500; // ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command

	m_identify_buffer[23] = ('1' << 8) | '.';
	m_identify_buffer[24] = ('0' << 8) | ' ';
	m_identify_buffer[25] = (' ' << 8) | ' ';
	m_identify_buffer[26] = (' ' << 8) | ' ';

	m_identify_buffer[27] = ('M' << 8) | 'A';
	m_identify_buffer[28] = ('M' << 8) | 'E';
	m_identify_buffer[29] = (' ' << 8) | ' ';
	m_identify_buffer[30] = (' ' << 8) | ' ';
	m_identify_buffer[31] = ('V' << 8) | 'i';
	m_identify_buffer[32] = ('r' << 8) | 't';
	m_identify_buffer[33] = ('u' << 8) | 'a';
	m_identify_buffer[34] = ('l' << 8) | ' ';
	m_identify_buffer[35] = ('C' << 8) | 'D';
	m_identify_buffer[36] = ('R' << 8) | 'O';
	m_identify_buffer[37] = ('M' << 8) | ' ';
	m_identify_buffer[38] = (' ' << 8) | ' ';
	m_identify_buffer[39] = (' ' << 8) | ' ';
	m_identify_buffer[40] = (' ' << 8) | ' ';
	m_identify_buffer[41] = (' ' << 8) | ' ';
	m_identify_buffer[42] = (' ' << 8) | ' ';
	m_identify_buffer[43] = (' ' << 8) | ' ';
	m_identify_buffer[44] = (' ' << 8) | ' ';
	m_identify_buffer[45] = (' ' << 8) | ' ';
	m_identify_buffer[46] = (' ' << 8) | ' ';

	m_identify_buffer[49] = 0x0600; // Word 49=Capabilities, IORDY may be disabled (bit_10), LBA Supported mandatory (bit_9)

	m_identify_buffer[88] = m_ultra_dma_mode;

	atapi_hle_device::device_start();
}

void atapi_cdrom_device::set_ultra_dma_mode(uint16_t mode)
{
	m_ultra_dma_mode = mode;
}

void atapi_cdrom_device::device_reset()
{
	atapi_hle_device::device_reset();
	m_media_change = true;
	m_sequence_counter = m_image->sequence_counter();
}

void atapi_dvdrom_device::device_reset()
{
	atapi_hle_device::device_reset();
	m_media_change = true;
	m_sequence_counter = m_image->sequence_counter();
}

void atapi_cdrom_device::process_buffer()
{
	if( m_sequence_counter != m_image->sequence_counter() )
	{
		m_media_change = true;
		m_sequence_counter = m_image->sequence_counter();
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
		case T10MMC_CMD_READ_CD:
		case T10SBC_CMD_READ_12:
		case T10SBC_CMD_SEEK_10:
			if(!m_image->exists())
			{
				m_phase = SCSI_PHASE_STATUS;
				m_sense_key = SCSI_SENSE_KEY_MEDIUM_ERROR;
				m_sense_asc = SCSI_SENSE_ASC_MEDIUM_NOT_PRESENT;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				return;
			}
			[[fallthrough]];
		default:
			if(m_media_change)
			{
				m_phase = SCSI_PHASE_STATUS;
				m_sense_key = SCSI_SENSE_KEY_UNIT_ATTENTION;
				m_sense_asc = SCSI_SENSE_ASC_NOT_READY_TO_READY_TRANSITION;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;

				if (command[0] == T10SPC_CMD_TEST_UNIT_READY || // amiga(idefix97)/firebeat/namcos12/sprinter
					command[0] == T10SPC_CMD_MODE_SENSE_10) // stingnet
					m_media_change = false;
				return;
			}
			break;
		case T10SPC_CMD_INQUIRY:
			m_media_change = false; // mtouchxl(mitsumi)
			break;
		case T10SPC_CMD_REQUEST_SENSE:
			m_media_change = false;
			break;
	}
	t10mmc::ExecCommand();

	if (m_status_code == SCSI_STATUS_CODE_GOOD)
	{
		switch (command[0])
		{
			// Set the DSC (Drive Seek Complete) bit on commands that involve a drive seek.
			// truckk is known to rely on this flag being set after T10SBC_CMD_SEEK_10.
			case T10SBC_CMD_SEEK_10:
			case T10MMC_CMD_PLAY_AUDIO_10:
			case T10MMC_CMD_PLAY_AUDIO_12:
			case T10MMC_CMD_PLAY_AUDIO_MSF:
			case T10MMC_CMD_PLAY_AUDIO_TRACK_INDEX:
				m_status |= IDE_STATUS_DSC;
				break;
		}
	}
}
