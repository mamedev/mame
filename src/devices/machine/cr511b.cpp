// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    CR-511-B CD-ROM drive

    CD-ROM drive with a custom MKE/Panasonic interface as used in the
    Commodore CDTV. Similar to the interface on early SoundBlaster cards.

    Hardware:
    - MN188161REB1
    - M50423FP
    - 4464S-08LL
    - LC8951

    TODO:
    - Subcode P-W data
    - Timing for status data or status change

***************************************************************************/

#include "emu.h"
#include "cr511b.h"

#define LOG_CMD    (1 << 1)
#define LOG_PARAM  (1 << 2)
#define LOG_DATA   (1 << 3)
#define LOG_SUBQ   (1 << 4)
#define LOG_SUBQ2  (1 << 5) // log subq data to popmessage

#define VERBOSE (LOG_GENERAL | LOG_CMD | LOG_PARAM)

#define LOGPARAM LOGMASKED(LOG_PARAM, "-> Param: %02x %02x %02x %02x %02x %02x\n", \
	m_input_fifo[1], m_input_fifo[2], m_input_fifo[3], \
	m_input_fifo[4], m_input_fifo[5], m_input_fifo[6])

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CR511B, cr511b_device, "cr511b", "CR-511-B CD-ROM drive")

cr511b_device::cr511b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cdrom_image_device(mconfig, CR511B, tag, owner, clock),
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

void cr511b_device::device_add_mconfig(machine_config &config)
{
	CDDA(config, m_cdda);
	m_cdda->add_route(0, DEVICE_SELF, 1.0, 0);
	m_cdda->add_route(1, DEVICE_SELF, 1.0, 1);
	m_cdda->set_cdrom_tag(*this);
	m_cdda->audio_end_cb().set(FUNC(cr511b_device::audio_end_cb));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cr511b_device::device_start()
{
	cdrom_image_device::device_start();

	m_frame_timer = timer_alloc(FUNC(cr511b_device::frame_cb), this);
	m_stch_timer = timer_alloc(FUNC(cr511b_device::stch), this);
	m_sten_timer = timer_alloc(FUNC(cr511b_device::sten), this);

	std::fill(std::begin(m_input_fifo), std::end(m_input_fifo), 0x00);
	std::fill(std::begin(m_output_fifo), std::end(m_output_fifo), 0x00);

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

void cr511b_device::device_reset()
{
	cdrom_image_device::device_reset();

	m_input_fifo_pos = 0;
	m_output_fifo_pos = 0;
	m_output_fifo_length = 0;

	m_status_ready = false;
	m_data_ready = false;

	m_status = STATUS_READY;

	if (exists())
		m_status |= STATUS_MEDIA;

	m_sten_cb(1);
	m_stch_cb(0);
}

std::pair<std::error_condition, std::string> cr511b_device::call_load()
{
	auto ret = cdrom_image_device::call_load();

	if (!ret.first)
		status_change(m_status | STATUS_MEDIA);

	return ret;
}

void cr511b_device::call_unload()
{
	status_change(STATUS_READY);

	cdrom_image_device::call_unload();
}

uint32_t cr511b_device::lba_to_msf(int32_t lba)
{
	uint32_t msf = 0;

	lba += 2 * 75; // lba 0 is equivalent to msf 00:02:00

	msf |= ((lba / (60 * 75)) & 0xff) << 16;
	msf |= (((lba / 75) % 60) & 0xff) << 8;
	msf |= ((lba % 75) & 0xff) << 0;

	return msf;
}

int32_t cr511b_device::msf_to_lba(uint32_t msf)
{
	uint32_t lba = 0;

	lba += ((msf >> 16) & 0xff) * 60 * 75;
	lba += ((msf >> 8) & 0xff) * 75;
	lba += ((msf >> 0) & 0xff);

	lba -= 2 * 75;  // msf 00:02:00 is equivalent to lba 0

	return lba;
}

int cr511b_device::size_to_track_type()
{
	switch (m_sector_size)
	{
		case 2048: return cdrom_file::CD_TRACK_MODE1;
	}

	// have only seen 2048 so far
	fatalerror("Unknown sector mode: %d\n", m_sector_size);
}

TIMER_CALLBACK_MEMBER(cr511b_device::frame_cb)
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

TIMER_CALLBACK_MEMBER(cr511b_device::stch)
{
	m_stch_cb(1);
	m_stch_cb(0);
}

void cr511b_device::status_change(uint8_t status)
{
	if (m_status != status)
	{
		m_status = status;

		if (m_status & STATUS_MOTOR)
			m_frame_timer->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
		else
			m_frame_timer->adjust(attotime::never);

		m_stch_timer->adjust(attotime::from_usec(64 * 3)); // TODO
	}
}

TIMER_CALLBACK_MEMBER(cr511b_device::sten)
{
	m_status_ready = true;

	m_sten_cb(0);
	m_sten_cb(1);
}

void cr511b_device::status_enable(uint8_t output_length)
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

void cr511b_device::audio_end_cb(int state)
{
	if (!state)
		return;

	LOGMASKED(LOG_CMD, "Playing audio finished\n", state);

	status_change(m_status & ~STATUS_PLAYING);
}

uint8_t cr511b_device::read()
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
				m_sten_cb(1);
			}
			else
			{
				m_status_ready = false;
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

void cr511b_device::write(uint8_t data)
{
	// verify that we're enabled and are in command mode
	if (!m_enabled || !m_cmd)
	{
		LOG("Invalid write: %02x (enabled: %d, cmd: %d)\n", data, m_enabled, m_cmd);
		return;
	}

	m_input_fifo[m_input_fifo_pos++] = data;

	switch (m_input_fifo[0])
	{
		case 0x01: if (m_input_fifo_pos == 7) cmd_seek(); break;
		case 0x02: if (m_input_fifo_pos == 7) cmd_read(); break;
		case 0x04: if (m_input_fifo_pos == 7) cmd_motor_on(); break;
		case 0x05: if (m_input_fifo_pos == 7) cmd_motor_off(); break;
		case 0x09: if (m_input_fifo_pos == 7) cmd_play_lba(); break;
		case 0x0a: if (m_input_fifo_pos == 7) cmd_play_msf(); break;
		case 0x0b: if (m_input_fifo_pos == 7) cmd_play_track(); break;
		case 0x81: if (m_input_fifo_pos == 1) cmd_read_status(); break;
		case 0x82: if (m_input_fifo_pos == 7) cmd_read_error(); break;
		case 0x84: if (m_input_fifo_pos == 7) cmd_set_mode(); break;
		case 0x87: if (m_input_fifo_pos == 7) cmd_read_subq(); break;
		case 0x89: if (m_input_fifo_pos == 7) cmd_read_disc_info(); break;
		case 0x8a: if (m_input_fifo_pos == 7) cmd_read_toc(); break;
		case 0x8b: if (m_input_fifo_pos == 7) cmd_pause(); break;
		case 0xa3: if (m_input_fifo_pos == 7) cmd_front_panel(); break;

		default:
			LOG("Unknown command: %02x\n", m_input_fifo[0]);
			status_enable(0);
			break;
	}
}

void cr511b_device::cmd_w(int state)
{
	m_cmd = !bool(state); // active low
}

void cr511b_device::enable_w(int state)
{
	m_enabled = !bool(state); // active low
}

void cr511b_device::cmd_seek()
{
	LOGMASKED(LOG_CMD, "Command: Seek\n");
	LOGPARAM;

	// TODO: Does this enable STATUS_SUCCESS?

	status_change(m_status | STATUS_MOTOR);
	status_enable(0);
}

void cr511b_device::cmd_read()
{
	LOGMASKED(LOG_CMD, "Command: Read\n");
	LOGPARAM;

	m_transfer_lba = (m_input_fifo[1] << 16) | (m_input_fifo[2] << 8) | (m_input_fifo[3] << 0);
	m_transfer_sectors = (m_input_fifo[4] << 8) | (m_input_fifo[5] << 0);
	m_transfer_length = m_transfer_sectors * m_sector_size;

	LOGMASKED(LOG_CMD, "-> LBA %d, sectors %d\n", m_transfer_lba, m_transfer_sectors);

	m_cdda->stop_audio();

	uint8_t status = m_status;

	status &= ~STATUS_PLAYING;
	status |= STATUS_MOTOR;

	status_change(status);
	status_enable(0);
}

void cr511b_device::cmd_motor_on()
{
	LOGMASKED(LOG_CMD, "Command: Motor On\n");
	LOGPARAM;

	// TODO: Does this enable STATUS_SUCCESS?

	status_change(m_status | STATUS_MOTOR);
	status_enable(0);
}

void cr511b_device::cmd_motor_off()
{
	LOGMASKED(LOG_CMD, "Command: Motor Off\n");
	LOGPARAM;

	// TODO: Does this enable STATUS_SUCCESS?

	status_change(m_status & ~STATUS_MOTOR);
	status_enable(0);
}

void cr511b_device::cmd_play_lba()
{
	LOGMASKED(LOG_CMD, "Command: Play LBA\n");
	LOGPARAM;

	// haven't found anything that uses it yet
	fatalerror("Play LBA: Not implemented\n");
}

void cr511b_device::cmd_play_msf()
{
	LOGMASKED(LOG_CMD, "Command: Play MSF\n");
	LOGPARAM;

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

		uint8_t status = m_status;

		if (m_cdda->audio_active())
			status |= STATUS_SUCCESS;

		m_cdda->stop_audio();

		status &= ~STATUS_PLAYING;
		status &= ~STATUS_MOTOR;

		status_change(status);
	}
	else if (start_lba < end_lba)
	{
		LOGMASKED(LOG_CMD, "Playing audio %02d:%02d.%02d to %02d:%02d.%02d (LBA %d to %d)\n",
			m_input_fifo[1], m_input_fifo[2], m_input_fifo[3],
			m_input_fifo[4], m_input_fifo[5], m_input_fifo[6], start_lba, end_lba);

		m_cdda->start_audio(start_lba, end_lba - start_lba);

		uint8_t status = m_status;

		status |= STATUS_PLAYING;
		status |= STATUS_MOTOR;

		status_change(status);
	}
	else
	{
		LOGMASKED(LOG_CMD, "Invalid range %d to %d!\n", start_lba, end_lba);
		status_change(m_status | STATUS_ERROR);
	}

	status_enable(0);
}

void cr511b_device::cmd_play_track()
{
	LOGMASKED(LOG_CMD, "Command: Play Track\n");
	LOGPARAM;

	uint8_t start_track = m_input_fifo[1];
	uint8_t start_index = m_input_fifo[2]; // TODO
	uint8_t end_track = m_input_fifo[3];
	uint8_t end_index = m_input_fifo[4]; // TODO

	uint32_t start_lba = get_track_start(start_track - 1);
	uint32_t end_lba = get_track_start(end_track - 1) - 1;

	LOGMASKED(LOG_CMD, "Playing audio track %d-%d to %d-%d (LBA %d to %d)\n", start_track, start_index, end_track, end_index, start_lba, end_lba);

	m_cdda->start_audio(start_lba, end_lba - start_lba);

	status_change(m_status | STATUS_PLAYING | STATUS_MOTOR);
	status_enable(0);
}

void cr511b_device::cmd_read_status()
{
	LOGMASKED(LOG_CMD, "Command: Read Status\n");

	m_status &= ~STATUS_SUCCESS;

	m_output_fifo[0] = m_status;

	status_enable(1);
}

void cr511b_device::cmd_read_error()
{
	LOGMASKED(LOG_CMD, "Command: Read Error\n");
	LOGPARAM;

	m_status &= ~STATUS_ERROR;
	m_status |= STATUS_READY;
	m_status |= STATUS_SUCCESS;

	m_output_fifo[2] |= (m_status & 0x10);

	status_enable(6);
}

void cr511b_device::cmd_version()
{
	LOGMASKED(LOG_CMD, "Command: Version\n");
	LOGPARAM;

	// haven't found anything that uses it yet
	fatalerror("Version: Not implemented\n");
}

void cr511b_device::cmd_set_mode()
{
	LOGMASKED(LOG_CMD, "Command: Set Mode\n");
	LOGPARAM;

	// 01: unknown (value seen: 02)
	// 02: sector size hi
	// 03: sector size lo
	// 04: unknown (value seen 00)
	// 05: unknown (value seen 0f)
	// 06: unknown (value seen 00)

	m_sector_size = (m_input_fifo[2] << 8) | m_input_fifo[3];

	LOGMASKED(LOG_CMD, "Sector size = %d\n", m_sector_size);

	m_status |= STATUS_SUCCESS; // ?

	status_enable(0);
}

void cr511b_device::cmd_read_subq()
{
	LOGMASKED(LOG_SUBQ, "Command: Read SubQ\n");
	if (VERBOSE & LOG_SUBQ)
		LOGPARAM;

	// 01: bit 1 - msf or lba
	// 02: unknown
	// 03: unknown
	// 04: unknown
	// 05: unknown
	// 06: unknown

	bool msf = bool(BIT(m_input_fifo[1], 1));

	if (m_cdda->audio_active())
	{
		uint32_t lba = m_cdda->get_audio_lba();
		uint8_t track = get_track(lba);

		uint32_t disc_pos = lba;
		uint32_t track_pos = lba - get_track_start(track);

		if (msf)
		{
			disc_pos = lba_to_msf(disc_pos);
			track_pos = lba_to_msf(track_pos - 150);

			if (VERBOSE & LOG_SUBQ2)
				popmessage("Playing track %d at %02d:%02d.%02d (disc %02d:%02d.%02d)", track + 1,
					track_pos >> 16 & 0xff, track_pos >> 8 & 0xff, track_pos & 0xff,
					disc_pos >> 16 & 0xff, disc_pos >> 8 & 0xff, disc_pos & 0xff);
		}
		else
		{
			if (VERBOSE & LOG_SUBQ2)
				popmessage("Playing track %d at %d (disc %d)", track, track_pos, disc_pos);
		}

		m_output_fifo[1] = get_adr_control(track);
		m_output_fifo[2] = track + 1;
		m_output_fifo[3] = 0x01; // TODO: index

		m_output_fifo[4] = disc_pos >> 24;
		m_output_fifo[5] = disc_pos >> 16;
		m_output_fifo[6] = disc_pos >> 8;
		m_output_fifo[7] = disc_pos >> 0;
		m_output_fifo[8] = track_pos >> 24;
		m_output_fifo[9] = track_pos >> 16;
		m_output_fifo[10] = track_pos >> 8;
		m_output_fifo[11] = track_pos >> 0;
		m_output_fifo[12] = 0; // TODO: upc flag

		if (m_cdda->audio_paused())
			m_output_fifo[0] = AUDIO_STATUS_PAUSED;
		else
			m_output_fifo[0] = AUDIO_STATUS_PLAY;
	}
	else if (m_cdda->audio_ended())
		m_output_fifo[0] = AUDIO_STATUS_COMPLETED;
	else
		m_output_fifo[0] = AUDIO_STATUS_NO_STATUS;

	LOGMASKED(LOG_SUBQ, "-> Audio status = %02x\n", m_output_fifo[0]);

	status_enable(13);
}

void cr511b_device::cmd_read_disc_info()
{
	LOGMASKED(LOG_CMD, "Command: Read Disc Info\n");
	LOGPARAM;

	uint8_t last_track = get_last_track();
	uint32_t last_lba = get_track_start(0xaa);
	uint32_t last_msf = lba_to_msf(last_lba);

	m_output_fifo[0] = 1; // first track
	m_output_fifo[1] = last_track;
	m_output_fifo[2] = last_msf >> 16;
	m_output_fifo[3] = last_msf >> 8;
	m_output_fifo[4] = last_msf >> 0;

	status_change(m_status | STATUS_MOTOR | STATUS_SUCCESS);
	status_enable(5);
}

void cr511b_device::cmd_read_toc()
{
	LOGMASKED(LOG_CMD, "Command: Read TOC\n");
	LOGPARAM;

	// 01: bit 1 - msf or lba
	// 02: track
	// 03: unused?
	// 04: unused?
	// 05: unused?
	// 06: unused?

	bool msf = bool(BIT(m_input_fifo[1], 1));
	uint8_t track = m_input_fifo[2];

	uint8_t status = m_status;

	status |= STATUS_MOTOR;

	if (track > get_last_track())
	{
		LOGMASKED(LOG_CMD, "Invalid track requested: %d\n", track);

		status |= STATUS_ERROR;
		status_enable(0);
	}
	else if (track == 0)
	{
		uint32_t track_start = get_track_start(0xaa);

		LOGMASKED(LOG_CMD, "Track 0 requested, lead out start %d\n", track_start);

		if (msf)
			track_start = lba_to_msf(track_start);

		m_output_fifo[1] = get_adr_control(0xaa);
		m_output_fifo[2] = 1; // first track
		m_output_fifo[3] = get_last_track();
		m_output_fifo[4] = track_start >> 24;
		m_output_fifo[5] = track_start >> 16;
		m_output_fifo[6] = track_start >> 8;
		m_output_fifo[7] = track_start >> 0;

		status_enable(8);
	}
	else
	{
		uint32_t track_start = get_track_start(track - 1);

		LOGMASKED(LOG_CMD, "Track %d requested, start %d\n", track, track_start);

		if (msf)
			track_start = lba_to_msf(track_start);

		m_output_fifo[1] = get_adr_control(track - 1);
		m_output_fifo[2] = track;
		m_output_fifo[3] = 0;
		m_output_fifo[4] = track_start >> 24;
		m_output_fifo[5] = track_start >> 16;
		m_output_fifo[6] = track_start >> 8;
		m_output_fifo[7] = track_start >> 0;

		status_enable(8);
	}

	status_change(status);
}

void cr511b_device::cmd_pause()
{
	LOGMASKED(LOG_CMD, "Command: Pause\n");
	LOGPARAM;

	// 01: 00 = pause audio, other values?
	// 02: unused?
	// 03: unused?
	// 04: unused?
	// 05: unused?
	// 06: unused?

	m_cdda->pause_audio(m_input_fifo[1] == 0x00);

	status_enable(0);
}

void cr511b_device::cmd_front_panel()
{
	LOGMASKED(LOG_CMD, "Command: Front Panel\n");
	LOGPARAM;

	// enables direct control of the drive by the front panel buttons: stop, play/pause, ff, rew

	// 01: enable/disable
	// 02: unused?
	// 03: unused?
	// 04: unused?
	// 05: unused?
	// 06: unused?

	status_enable(0);
}
