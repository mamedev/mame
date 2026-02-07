// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

CR-560-B CD-ROM drive

Panasonic pre-IDE MKE interface, akin to CR-511-B (which this is based off) and Creative
Soundblaster.

NOTES:
- all commands but cmd_reset and cmd_read_subq acknowledge command byte issued
  in first output return byte;
- 3do disks are not ISO9660 compatible. First sector at LBA 0 / MSF 00:02:00 is described at:
  https://github.com/trapexit/portfolio_os/blob/bee7b0c8e4287083c73cb5497b658880c9e7af8e/utils/discdata.h#L52

TODO:
- Enough to load a few sectors and nothing else;
- Find common points with other MKE drives, generate a streamlined interface;

**************************************************************************************************/

#include "emu.h"
#include "cr560b.h"

#define LOG_CMD    (1 << 1)
#define LOG_TOC    (1 << 2)
#define LOG_PARAM  (1 << 3)
#define LOG_DATA   (1 << 4)
#define LOG_SUBQ   (1 << 5)
#define LOG_SUBQ2  (1 << 6) // log subq data to popmessage

#define VERBOSE (LOG_GENERAL | LOG_TOC | LOG_CMD | LOG_PARAM)
//#define VERBOSE (LOG_TOC)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#define LOGPARAM LOGMASKED(LOG_PARAM, "-> Param: %02x %02x %02x %02x %02x %02x\n", \
	m_input_fifo[1], m_input_fifo[2], m_input_fifo[3], \
	m_input_fifo[4], m_input_fifo[5], m_input_fifo[6])

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CR560B, cr560b_device, "cr560b", "CR-560-B CD-ROM drive")

cr560b_device::cr560b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cdrom_image_device(mconfig, CR560B, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cdda(*this, "cdda"),
	m_stch_cb(*this),
	m_sten_cb(*this),
	m_drq_cb(*this),
	m_dten_cb(*this),
	m_scor_cb(*this),
	m_input_fifo_pos(0),
	m_output_fifo_pos(0),
	m_output_fifo_length(0),
	m_status(0),
	m_enabled(false),
	m_cmd(false),
	m_status_ready(false),
	m_data_ready(false)
{
	set_interface("cdrom");
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void cr560b_device::device_add_mconfig(machine_config &config)
{
	CDDA(config, m_cdda);
	m_cdda->add_route(0, DEVICE_SELF, 1.0, 0);
	m_cdda->add_route(1, DEVICE_SELF, 1.0, 1);
	m_cdda->set_cdrom_tag(*this);
	m_cdda->audio_end_cb().set(FUNC(cr560b_device::audio_end_cb));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cr560b_device::device_start()
{
	cdrom_image_device::device_start();

	m_frame_timer = timer_alloc(FUNC(cr560b_device::frame_cb), this);
	m_stch_timer = timer_alloc(FUNC(cr560b_device::stch), this);
	m_sten_timer = timer_alloc(FUNC(cr560b_device::sten), this);

	std::fill(std::begin(m_input_fifo), std::end(m_input_fifo), 0x00);
	std::fill(std::begin(m_output_fifo), std::end(m_output_fifo), 0x00);
	std::fill(std::begin(m_transfer_buffer), std::end(m_transfer_buffer), 0xff);

	// register for save states
	save_item(NAME(m_input_fifo));
	save_item(NAME(m_input_fifo_pos));
	save_item(NAME(m_output_fifo));
	save_item(NAME(m_output_fifo_pos));
	save_item(NAME(m_output_fifo_length));
	save_item(NAME(m_status));
	save_item(NAME(m_sector_size));
	save_item(NAME(m_transfer_lba));
	save_item(NAME(m_transfer_sectors));
	save_item(NAME(m_transfer_length));
	save_item(NAME(m_transfer_buffer));
	save_item(NAME(m_transfer_buffer_pos));
	save_item(NAME(m_enabled));
	save_item(NAME(m_cmd));
	save_item(NAME(m_status_ready));
	save_item(NAME(m_data_ready));
}

void cr560b_device::device_reset()
{
	cdrom_image_device::device_reset();

	m_input_fifo_pos = 0;
	m_output_fifo_pos = 0;
	m_output_fifo_length = 0;
	m_sector_size = 2048;

	m_status_ready = false;
	m_data_ready = false;
	m_cdrom_speed = 1;

	m_status = 0; //STATUS_READY;

	if (exists())
		m_status |= STATUS_MEDIA;

	m_sten_cb(1);
	m_stch_cb(0);
}

std::pair<std::error_condition, std::string> cr560b_device::call_load()
{
	auto ret = cdrom_image_device::call_load();

	if (!ret.first)
	{
		status_change(m_status | STATUS_MEDIA | STATUS_DOOR_CLOSED);
	}

	return ret;
}

void cr560b_device::call_unload()
{
	status_change(0);

	cdrom_image_device::call_unload();
}

uint32_t cr560b_device::lba_to_msf(int32_t lba)
{
	uint32_t msf = 0;

	lba += 2 * 75; // lba 0 is equivalent to msf 00:02:00

	msf |= ((lba / (60 * 75)) & 0xff) << 16;
	msf |= (((lba / 75) % 60) & 0xff) << 8;
	msf |= ((lba % 75) & 0xff) << 0;

	return msf;
}

int32_t cr560b_device::msf_to_lba(uint32_t msf)
{
	int32_t lba = 0;

	lba += ((msf >> 16) & 0xff) * 60 * 75;
	lba += ((msf >> 8) & 0xff) * 75;
	lba += ((msf >> 0) & 0xff);

	lba -= 2 * 75;  // msf 00:02:00 is equivalent to lba 0

	if (lba < 0)
		throw emu_fatalerror("Requested lba in lead-in %08x %08x\n", msf, lba);

	return lba;
}

int cr560b_device::size_to_track_type()
{
	switch (m_sector_size)
	{
		case 2048: return cdrom_file::CD_TRACK_MODE1;
		// TODO: really from flags
		// Also: Clio and Madam sets 2448 in DMA params, off by 96 (subcode ...)
		case 2352: return cdrom_file::CD_TRACK_AUDIO;
	}

	// TODO: 3do definitely supports other formats
	throw emu_fatalerror("Unknown sector mode: %d\n", m_sector_size);
}

TIMER_CALLBACK_MEMBER(cr560b_device::frame_cb)
{
	if (m_transfer_sectors > 0)
	{
		// old data hasn't been read completely yet
		if (m_data_ready)
			return;

		LOGMASKED(LOG_DATA, "Reading sector: %d\n", m_transfer_lba);

		read_data(m_transfer_lba, m_transfer_buffer, size_to_track_type());

		// prepare for next sector
		m_transfer_lba++;
		m_transfer_sectors--;
		m_transfer_buffer_pos = 0;

		// signal that we have data
		m_data_ready = true;
		m_drq_cb(1);
	}
	else if (m_status & STATUS_PLAYING)
	{
		// TODO: subcode handling
		m_scor_cb(0);
		m_scor_cb(1);
	}
}

TIMER_CALLBACK_MEMBER(cr560b_device::stch)
{
	m_stch_cb(1);
	m_stch_cb(0);
}

void cr560b_device::status_change(uint8_t status)
{
	if (m_status != status)
	{
		m_status = status;

		if (m_status & STATUS_MOTOR)
		{
			m_frame_timer->adjust(attotime::from_hz(75 * m_cdrom_speed), 0, attotime::from_hz(75 * m_cdrom_speed));
		}
		else
			m_frame_timer->adjust(attotime::never);

		m_stch_timer->adjust(attotime::from_usec(64 * 3)); // TODO
	}
}

TIMER_CALLBACK_MEMBER(cr560b_device::sten)
{
	m_status_ready = true;

	m_sten_cb(0);
}

void cr560b_device::status_enable(uint8_t output_length)
{
	m_input_fifo_pos = 0;
	m_output_fifo_pos = 0;
	m_output_fifo_length = output_length;

	// do we have status data to send?
	if (m_output_fifo_length > 0)
	{
		if (m_input_fifo[0] != 0x87 || (VERBOSE & LOG_SUBQ))
			LOGMASKED(LOG_CMD, "-> Output: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n", m_output_fifo[0], m_output_fifo[1], m_output_fifo[2], m_output_fifo[3], m_output_fifo[4], m_output_fifo[5], m_output_fifo[6], m_output_fifo[7], m_output_fifo[8], m_output_fifo[9], m_output_fifo[10], m_output_fifo[11]);

		m_sten_timer->adjust(attotime::from_usec(64 * 4)); // TODO
	}
}

void cr560b_device::audio_end_cb(int state)
{
	if (!state)
		return;

	LOGMASKED(LOG_CMD, "Playing audio finished\n", state);

	status_change(m_status & ~STATUS_PLAYING);
}

uint8_t cr560b_device::read()
{
	uint8_t data = 0xff;

	if (!m_enabled)
	{
		LOG("Read while not enabled!\n");
		return data;
	}

	if (m_cmd)
	{
		// command mode
		if (m_status_ready)
		{
			data = m_output_fifo[m_output_fifo_pos];

			// clear old data once read
			m_output_fifo[m_output_fifo_pos++] = 0x00;

			LOGMASKED(LOG_DATA, "Data from drive: %02x (%d of %d)\n", data, m_output_fifo_pos, m_output_fifo_length);

			// more data?
			if (m_output_fifo_pos < m_output_fifo_length)
			{
				m_sten_cb(0);
			}
			else
			{
				m_status_ready = false;
				m_sten_cb(1);
			}
		}
	}
	else
	{
		// data mode
		if (m_data_ready)
		{
			data = m_transfer_buffer[m_transfer_buffer_pos];
			m_transfer_length--;

			LOGMASKED(LOG_DATA, "Data = %02x, pos %d, length %d\n", data, m_transfer_buffer_pos, m_transfer_length);

			// finished transferring this sector?
			if (++m_transfer_buffer_pos == m_sector_size)
			{
				m_data_ready = false;
				m_drq_cb(0);

				if (m_transfer_sectors == 0)
				{
					LOGMASKED(LOG_DATA, "Read done\n");
					status_change(m_status | STATUS_SUCCESS);
				}
			}
		}
	}

	return data;
}

void cr560b_device::write(uint8_t data)
{
	// verify that we're enabled and are in command mode
	if (!m_enabled || !m_cmd)
	{
		LOG("Invalid write: %02x (enabled: %d, cmd: %d)\n", data, m_enabled, m_cmd);
		return;
	}

	m_input_fifo[m_input_fifo_pos++] = data;

	// all commands except for abort takes 7 input bytes
	if (m_input_fifo_pos == 1 && m_input_fifo[0] == 0x08)
	{
		LOG("Abort command %08x (tbd)\n", m_input_fifo[0]);
		//cmd_abort();
		status_enable(0);
		return;
	}

	if (m_input_fifo_pos != 7)
		return;

	switch (m_input_fifo[0])
	{
		case 0x01: cmd_seek(); break;
		case 0x02: cmd_motor_on(); break;
		case 0x03: cmd_motor_off(); break;
		//case 0x04: cmd_diag(); break;
		case 0x05: cmd_read_status(); break;
		// TODO: drawer_open is issued by fz10e just after purging the disc avatars
		//case 0x06: cmd_drawer_open(); break;
		//case 0x07: cmd_drawer_close(); break;
		case 0x09: cmd_set_mode(); break;
		//case 0x0a: cmd_reset(); break;
		// TODO: flush requested by Audio CD player
		//case 0x0b: cmd_flush(); break;

		// these two are unconfirmed
		//case 0x0c: cmd_lock(); break;
		//case 0x0d: cmd_pause_resume(); break;

		case 0x0e: cmd_play_msf(); break;
		case 0x0f: cmd_play_track(); break;
		case 0x10: cmd_read(); break;
		//case 0x11: cmd_read_subq(); break;

		case 0x80: cmd_data_path_check(); break;
		//case 0x81: cmd_read_status(); break;
		case 0x82: cmd_read_error(); break;
		case 0x83: cmd_version(); break;
		//case 0x84: cmd_mode_sense(); break;
		case 0x85: cmd_read_capacity(); break;
		//case 0x86: cmd_read_header(); break;
		//case 0x87: cmd_read_subq(); break;
		//case 0x88: cmd_read_upc(); break;
		//case 0x89: cmd_read_isrc(); break;
		//case 0x8a: cmd_read_disc_code(); break;
		case 0x8b: cmd_read_disc_info(); break;
		case 0x8c: cmd_read_toc(); break;
		case 0x8d: cmd_read_session_info(); break;
		//case 0x8e: cmd_read_device_driver(); break;

		default:
			LOG("Unknown command: %02x\n", m_input_fifo[0]);
			status_enable(0);
			break;
	}
}

void cr560b_device::cmd_w(int state)
{
	m_cmd = !bool(state); // active low
}

void cr560b_device::enable_w(int state)
{
	m_enabled = !bool(state); // active low
}

/******************
 *
 * Commands
 *
 *****************/

// TODO: unverified in this implementation
void cr560b_device::cmd_seek()
{
	LOGMASKED(LOG_CMD, "Command: Seek\n");
	LOGPARAM;

	const u8 seek_mode = m_input_fifo[1];
	const u32 param = (m_input_fifo[2] << 16) | (m_input_fifo[3] << 8) | (m_input_fifo[4]);

	switch(seek_mode)
	{
		case 0x00:
			LOGMASKED(LOG_CMD, "-> To LBA %06x\n", param);
			break;
		case 0x02:
			LOGMASKED(LOG_CMD, "-> To MSF %06x\n", param);
			break;
		default:
			LOGMASKED(LOG_CMD, "-> Unknown mode %02x!\n", seek_mode);
			break;
	}

	// TODO: Does this enable STATUS_SUCCESS?
	u8 status = m_status;
	status |= STATUS_MOTOR;
	m_output_fifo[0] = 0x01;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// TODO: unverified in this implementation
// labeled as "No Operation" in 3do Technical Manual
void cr560b_device::cmd_read_status()
{
	LOGMASKED(LOG_CMD, "Command: Read Status\n");

	u8 status = m_status;
	status &= ~STATUS_SUCCESS;
	m_output_fifo[0] = 0x05;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// TODO: unverified in this implementation
// Spinup
void cr560b_device::cmd_motor_on()
{
	LOGMASKED(LOG_CMD, "Command: Motor On\n");
	LOGPARAM;

	// TODO: Does this enable STATUS_SUCCESS?
	u8 status = m_status;
	status |= STATUS_SUCCESS;
	m_output_fifo[0] = 0x02;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// TODO: unverifed in this implementation
// Spindown
void cr560b_device::cmd_motor_off()
{
	LOGMASKED(LOG_CMD, "Command: Motor Off\n");
	LOGPARAM;

	// TODO: Does this enable STATUS_SUCCESS?
	u8 status = m_status;
	status &= ~STATUS_MOTOR;
	m_output_fifo[0] = 0x03;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

void cr560b_device::cmd_set_mode()
{
	LOGMASKED(LOG_CMD, "Command: Set Mode\n");
	LOGPARAM;

	u8 status = m_status;

	switch(m_input_fifo[1])
	{
		case 0x00:
			// density codes:
			// 0x00: default density
			// 0x01: data
			// 0x81: mode2 XA
			// 0x82: digital audio
			m_sector_size = (m_input_fifo[3] << 8) | m_input_fifo[4];
			LOGMASKED(LOG_CMD, "-> density code %02x sector size %d flags %02x\n", m_input_fifo[2], m_sector_size, m_input_fifo[5]);
			break;
		case 0x01:
			LOGMASKED(LOG_CMD, "-> Error recovery type %02x retry count %02x\n", m_input_fifo[2], m_input_fifo[3]);
			break;
		case 0x02:
			LOGMASKED(LOG_CMD, "-> Stop time %02x\n", m_input_fifo[2]);
			break;
		case 0x03:
			m_cdrom_speed = BIT(m_input_fifo[2], 7) + 1;
			LOGMASKED(LOG_CMD, "-> Set speed %dx vpitch %02x%02x\n", m_cdrom_speed, m_input_fifo[3], m_input_fifo[4]);
			if (m_cdrom_speed == 2)
				status |= STATUS_DOUBLE_SPEED;
			else
				status &= ~STATUS_DOUBLE_SPEED;
			break;
		case 0x04:
			LOGMASKED(LOG_CMD, "-> Chunk size %02x\n", m_input_fifo[2]);
			break;
		//case 5: set volume (never seen it so far, not in Portfolio OS?)
		default:
			LOGMASKED(LOG_CMD, "-> <unknown %02x> !?\n", m_input_fifo[1]);
			break;
	}

	status |= STATUS_SUCCESS;
	m_output_fifo[0] = 0x09;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// TODO: unverified in this implementation
// TODO: needs to check against 2x speed
void cr560b_device::cmd_play_msf()
{
	LOGMASKED(LOG_CMD, "Command: Play MSF\n");
	LOGPARAM;

	uint8_t status = m_status;

	uint32_t start = (m_input_fifo[1] << 16) | (m_input_fifo[2] << 8) | (m_input_fifo[3] << 0);
	uint32_t end = (m_input_fifo[4] << 16) | (m_input_fifo[5] << 8) | (m_input_fifo[6] << 0);

	int32_t start_lba = msf_to_lba(start);
	int32_t end_lba = msf_to_lba(end);

	// play to the end of the disc?
	if (end == 0xffffff)
		end_lba = get_track_start(0xaa) - 1;

	if (start == 0 && end == 0)
	{
		LOGMASKED(LOG_CMD, "Stop audio\n");

		if (m_cdda->audio_active())
			status |= STATUS_SUCCESS;

		m_cdda->stop_audio();

		status &= ~STATUS_PLAYING;
		status &= ~STATUS_MOTOR;
	}
	else if (start_lba < end_lba)
	{
		LOGMASKED(LOG_CMD, "Playing audio %02d:%02d.%02d to %02d:%02d.%02d (LBA %06x to %06x)\n",
			m_input_fifo[1], m_input_fifo[2], m_input_fifo[3],
			m_input_fifo[4], m_input_fifo[5], m_input_fifo[6], start_lba, end_lba);

		m_cdda->start_audio(start_lba, end_lba - start_lba);

		status |= STATUS_PLAYING;
		status |= STATUS_MOTOR;
	}
	else
	{
		LOGMASKED(LOG_CMD, "Invalid range %d to %d!\n", start_lba, end_lba);

		status |= STATUS_ERROR;
	}

	m_output_fifo[0] = 0x0e;
	m_output_fifo[1] = status;
	status_change(status);
	status_enable(2);
}

// TODO: unverified in this implementation
// TODO: needs to check against 2x speed
void cr560b_device::cmd_play_track()
{
	LOGMASKED(LOG_CMD, "Command: Play Track\n");
	LOGPARAM;

	uint8_t start_track = m_input_fifo[1];
	uint8_t start_index = m_input_fifo[2]; // TODO
	uint8_t end_track = m_input_fifo[3];
	uint8_t end_index = m_input_fifo[4]; // TODO

	uint32_t start_lba = get_track_start(start_track - 1);
	uint32_t end_lba = get_track_start(end_track - 1) - 1;

	LOGMASKED(LOG_CMD, "Playing audio track %d-%d to %d-%d (LBA %06x to %06x)\n", start_track, start_index, end_track, end_index, start_lba, end_lba);

	m_cdda->start_audio(start_lba, end_lba - start_lba);

	u8 status = m_status;
	status |= STATUS_PLAYING;
	status |= STATUS_MOTOR;

	m_output_fifo[0] = 0x0f;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// TODO: unverified in this implementation (sure needs data ready in Clio)
void cr560b_device::cmd_read()
{
	LOGMASKED(LOG_CMD, "Command: Read\n");
	LOGPARAM;

	u8 read_mode = m_input_fifo[4];

	if (read_mode & 0xfe)
		throw emu_fatalerror("Unknown read mode %02x", read_mode);

	// compared to CR-511 this is shuffled around to accomodate the transfer mode
	u32 start_sector = (m_input_fifo[1] << 16) | (m_input_fifo[2] << 8) | (m_input_fifo[3] << 0);

	if (read_mode == 0)
	{
		LOGMASKED(LOG_CMD, "MSF mode %06x ", start_sector);
		start_sector = msf_to_lba(start_sector);
	}

	m_transfer_lba = start_sector;
	m_transfer_sectors = (m_input_fifo[5] << 8) | (m_input_fifo[6] << 0);
	m_transfer_length = m_transfer_sectors * m_sector_size;

	LOGMASKED(LOG_CMD, "-> LBA %06x, sectors %d\n", m_transfer_lba, m_transfer_sectors);

	m_cdda->stop_audio();

	uint8_t status = m_status;

	status &= ~STATUS_PLAYING;
	status |= STATUS_MOTOR;

	m_output_fifo[0] = 0x10;
	m_output_fifo[1] = status;

	status_change(status);
	status_enable(2);
}

// checked in BIOS if a CD is inserted
void cr560b_device::cmd_data_path_check()
{
	LOGMASKED(LOG_CMD, "Command: Data path check\n");
	LOGPARAM;

	m_output_fifo[0]  = 0x80;
	m_output_fifo[1]  = 0xaa;
	m_output_fifo[2]  = 0x55;
	m_output_fifo[3]  = m_status;

	status_enable(4);
}

// TODO: most unknowns, just checked by BIOS at startup
void cr560b_device::cmd_read_error()
{
	LOGMASKED(LOG_CMD, "Command: Read error\n");
	LOGPARAM;

	uint8_t status = m_status;
	//status |= STATUS_READY;

	m_output_fifo[0]  = 0x82;
	m_output_fifo[1]  = 0x00;
	m_output_fifo[2]  = 0x00;
	m_output_fifo[3]  = 0x00;
	m_output_fifo[4]  = 0x00;
	m_output_fifo[5]  = 0x00;
	m_output_fifo[6]  = 0x00;
	m_output_fifo[7]  = 0x00;
	m_output_fifo[8]  = 0x00;
	// perhaps just the status byte?
	m_output_fifo[9]  = exists() ? 1 : 0;

	status_change(status);
	status_enable(10);
}

void cr560b_device::cmd_version()
{
	LOGMASKED(LOG_CMD, "Command: Version\n");
	LOGPARAM;

	m_output_fifo[0]  = 0x83;
	m_output_fifo[1]  = 0x00; // manufacturer ID
	m_output_fifo[2]  = 0x10; // ^
	m_output_fifo[3]  = 0x00; // manufacturer number
	m_output_fifo[4]  = 0x01; // ^
	m_output_fifo[5]  = 0x00;
	m_output_fifo[6]  = 0x00;
	m_output_fifo[7]  = 0x00; // revision number
	m_output_fifo[8]  = 0x00; // ^
	m_output_fifo[9]  = 0x00; // flag
	m_output_fifo[10] = 0x00; // ^?
	m_output_fifo[11] = m_status;

//  status_change(status);
	status_enable(12);
}

// same as cmd_read_session_info on regular single session disk
void cr560b_device::cmd_read_capacity()
{
	LOGMASKED(LOG_CMD, "Command: Read Capacity\n");
	LOGPARAM;

	u8 status = m_status;
	// TODO: unconfirmed here
	//bool msf = bool(BIT(m_input_fifo[1], 1));

	m_output_fifo[0] = 0x85;
	if (exists())
	{
		uint32_t track_start = get_track_start(0xaa);

		LOGMASKED(LOG_TOC, "Lead out start %06x\n", track_start);

		//if (msf)
		track_start = lba_to_msf(track_start);

		LOGMASKED(LOG_TOC, "-> MSF %06x (%d)\n", track_start, track_start);

		status |= STATUS_READY;

		m_output_fifo[1] = 0;
		m_output_fifo[2] = track_start >> 16;
		m_output_fifo[3] = track_start >> 8;
		m_output_fifo[4] = track_start >> 0;
		m_output_fifo[5] = 0;
		m_output_fifo[6] = 0;
		m_output_fifo[7] = status;

		status_enable(8);
	}
	else
	{
		status &= ~STATUS_READY;
		status |= STATUS_ERROR;
		m_output_fifo[1] = status;
	}

	status_change(status);
}


void cr560b_device::cmd_read_disc_info()
{
	LOGMASKED(LOG_CMD, "Command: Read Disc Info\n");
	LOGPARAM;

	uint8_t status = m_status;

	m_output_fifo[0] = 0x8b;

	if (exists())
	{
		uint8_t last_track = get_last_track();
		uint32_t last_lba = get_track_start(0xaa);
		uint32_t last_msf = lba_to_msf(last_lba);

		status |= STATUS_MOTOR | STATUS_SUCCESS;
		status |= STATUS_READY;

		LOGMASKED(LOG_TOC, "First track %d last track %d, lead out start %06x\n", 1, last_track, last_msf);

		// TODO: determine disk type
		// 0x00: DA or CD-Rom
		// 0x10: CD-i
		// 0x20: CD-Rom XA
		m_output_fifo[1] = 0x00;
		m_output_fifo[2] = 1; // first track
		m_output_fifo[3] = last_track;
		m_output_fifo[4] = last_msf >> 16;
		m_output_fifo[5] = last_msf >> 8;
		m_output_fifo[6] = last_msf >> 0;
		m_output_fifo[7] = status;
		status_change(status);
		status_enable(8);
	}
	else
	{
		status &= ~STATUS_READY;
		status |= STATUS_ERROR;
		m_output_fifo[1] = status;
		status_change(status);
		status_enable(2);
	}
}

// almost identical vs. CR-511B
void cr560b_device::cmd_read_toc()
{
	LOGMASKED(LOG_CMD, "Command: Read TOC\n");
	LOGPARAM;

	// 01: bit 1 - msf or lba
	// 02: track
	// 03: unused?
	// 04: unused?
	// 05: unused?
	// 06: unused?

	// TODO: unconfirmed here
	//bool msf = bool(BIT(m_input_fifo[1], 1));
	uint8_t track = m_input_fifo[2];

	uint8_t status = m_status;

	status |= STATUS_MOTOR;

	m_output_fifo[0] = 0x8c;
	if (track > get_last_track() || !exists())
	{
		LOGMASKED(LOG_CMD, "Invalid track requested: %d\n", track);

		status &= ~STATUS_READY;
		status |= STATUS_ERROR;
		m_output_fifo[1] = status;
		status_enable(2);
	}
	else if (track == 0)
	{
		uint32_t track_start = get_track_start(0xaa);

		LOGMASKED(LOG_TOC, "TOC: Track 0 requested, lead out start %06x\n", track_start);

		//if (msf)
		track_start = lba_to_msf(track_start);

		LOGMASKED(LOG_TOC, "-> MSF %06x (%d)\n", track_start, track_start);

		status |= STATUS_READY;

		m_output_fifo[1] = 0;
		m_output_fifo[2] = get_adr_control(0xaa);
		m_output_fifo[3] = 1; // first track
		m_output_fifo[4] = get_last_track();
		m_output_fifo[5] = track_start >> 16;
		m_output_fifo[6] = track_start >> 8;
		m_output_fifo[7] = track_start >> 0;
		m_output_fifo[8] = 0;
		m_output_fifo[9] = status;

		status_enable(10);
	}
	else
	{
		uint32_t track_start = get_track_start(track - 1);

		LOGMASKED(LOG_TOC, "Track %d requested, start %06x\n", track, track_start);

		status |= STATUS_READY;

		//if (msf)
		track_start = lba_to_msf(track_start);

		const u8 adr = get_adr_control(track - 1);
		LOGMASKED(LOG_TOC, "-> MSF %06x (%d) adr %02x\n", track_start, track_start, adr);

		m_output_fifo[1] = 0;
		m_output_fifo[2] = adr;
		m_output_fifo[3] = track;
		m_output_fifo[4] = 0;
		m_output_fifo[5] = track_start >> 16;
		m_output_fifo[6] = track_start >> 8;
		m_output_fifo[7] = track_start >> 0;
		m_output_fifo[8] = 0;
		m_output_fifo[9] = status;

		status_enable(10);
	}

	status_change(status);
}

// TODO: shouldn't matter for stock 3do games beyond reading lead-out but yes it's a thing
void cr560b_device::cmd_read_session_info()
{
	LOGMASKED(LOG_CMD, "Command: Read Session Info\n");
	LOGPARAM;

	u8 status = m_status;
	// TODO: unconfirmed here
	//bool msf = bool(BIT(m_input_fifo[1], 1));

	m_output_fifo[0] = 0x8d;
	if (exists())
	{
		uint32_t track_start = get_track_start(0xaa);

		LOGMASKED(LOG_TOC, "Session: Lead out start %06x\n", track_start);

		//if (msf)
		track_start = lba_to_msf(track_start);

		LOGMASKED(LOG_TOC, "-> MSF %06x (%d)\n", track_start, track_start);

		status |= STATUS_READY;

		m_output_fifo[1] = 0;
		m_output_fifo[2] = track_start >> 16;
		m_output_fifo[3] = track_start >> 8;
		m_output_fifo[4] = track_start >> 0;
		m_output_fifo[5] = 0;
		m_output_fifo[6] = 0;
		m_output_fifo[7] = status;

		status_enable(8);
	}
	else
	{
		status |= STATUS_ERROR;
		m_output_fifo[1] = status;
	}

	status_change(status);
}
