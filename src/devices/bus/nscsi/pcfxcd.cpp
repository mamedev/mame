// license:BSD-3-Clause
// copyright-holders:Damian Rogers
/***************************************************************************

    NEC CD-ROM DRIVE:FX (PC-FX internal CD-ROM drive)

    A SCSI-2 CD-ROM drive with the NEC vendor audio commands d8h/d9h/dah/
    ddh/deh (10-byte CDBs from c0h up) and their LBA, BCD M:S:F and BCD
    track address modes.

    The vendor commands are shared with the PC-8801-30, PC Engine CD-ROM²
    and CDR-35 drives, but the responses here are the DRIVE:FX ones; a
    common NEC drive base is left for the rework of the PC-8801-30 /
    PC Engine host path (mamedev/mame#11078).

    TODO:
    - Scan (d2h)
    - Seek times
    - Real sub-channel data, Q is synthesized from the play position and
      R-W are zero; no sub-code while paused, stopped or reading data

***************************************************************************/

#include "emu.h"
#include "pcfxcd.h"

#include "multibyte.h"

#include <algorithm>

#define LOG_CMD            (1U << 1)
#define LOG_CDDA           (1U << 2)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCMD(...)         LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCDDA(...)        LOGMASKED(LOG_CDDA, __VA_ARGS__)

namespace {

// double speed: 150 sectors a second, read ahead into a 64 KB buffer; sectors are
// handed over as the drive reads them, each as a burst at the bus rate
const attotime SECTOR_TIME = attotime::from_hz(150);
// asynchronous SCSI, 1.5 MB/s (GMAKER hardware manual)
const attotime BUS_BYTE_TIME = attotime::from_hz(1'500'000);
constexpr u32 READ_AHEAD_SECTORS = 32;
constexpr u32 SECTOR_BYTES = 2048;

// NEC additional sense code for audio commands while no audio is playing (the BIOS CD
// player expects it from pause/resume when stopped, and retries the command on any other)
constexpr u16 NEC_SKC_AUDIO_NOT_PLAYING = 0x2c00;

} // anonymous namespace

DEFINE_DEVICE_TYPE(NSCSI_CDROM_PCFX, nscsi_cdrom_pcfx_device, "nscsi_cdrom_pcfx", "NEC CD-ROM DRIVE:FX")

nscsi_cdrom_pcfx_device::nscsi_cdrom_pcfx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nscsi_cdrom_device(mconfig, NSCSI_CDROM_PCFX, tag, owner, clock)
	, m_cdda_status(CDDA_STOPPED)
	, m_cdda_loop(false)
	, m_cdda_start(0)
	, m_cdda_end(0)
	, m_subcode_cb(*this)
	, m_subcode_timer(nullptr)
	, m_subcode_symbol(0)
	, m_command(0)
	, m_read_left(0)
	, m_read_buffered(0)
	, m_read_done(attotime::zero)
	, m_sector_wait(attotime::zero)
{
}

void nscsi_cdrom_pcfx_device::device_add_mconfig(machine_config &config)
{
	nscsi_cdrom_device::device_add_mconfig(config);
	cdda->audio_end_cb().set(FUNC(nscsi_cdrom_pcfx_device::nec_audio_end));
}

void nscsi_cdrom_pcfx_device::device_start()
{
	nscsi_cdrom_device::device_start();

	m_subcode_timer = timer_alloc(FUNC(nscsi_cdrom_pcfx_device::subcode_tick), this);
	std::fill(std::begin(m_subq), std::end(m_subq), 0);

	save_item(NAME(m_cdda_status));
	save_item(NAME(m_cdda_loop));
	save_item(NAME(m_cdda_start));
	save_item(NAME(m_cdda_end));
	save_item(NAME(m_subcode_symbol));
	save_item(NAME(m_subq));
	save_item(NAME(m_command));
	save_item(NAME(m_read_left));
	save_item(NAME(m_read_buffered));
	save_item(NAME(m_read_done));
	save_item(NAME(m_sector_wait));
}

void nscsi_cdrom_pcfx_device::device_reset()
{
	nscsi_cdrom_device::device_reset();

	// a machine reset: nscsi_full_device::device_reset() does not call scsi_abort()
	set_cdda_status(CDDA_STOPPED);
	m_cdda_loop = false;
	m_cdda_start = m_cdda_end = 0;
	m_command = 0;
	m_read_left = m_read_buffered = 0;
	m_sector_wait = attotime::zero;
}

// time until the next sector is available: zero while the read-ahead buffer
// holds one, otherwise the remainder of the sector being read
attotime nscsi_cdrom_pcfx_device::next_sector_wait()
{
	if (!m_read_left && !m_read_buffered)
		return attotime::zero;

	const attotime now = machine().time();
	while (m_read_left && m_read_done <= now)
	{
		if (m_read_buffered == READ_AHEAD_SECTORS)
		{
			// the buffer was full, reading resumes now
			m_read_done = now + SECTOR_TIME;
			break;
		}
		m_read_buffered++;
		m_read_left--;
		m_read_done += SECTOR_TIME;
	}

	if (m_read_buffered)
	{
		m_read_buffered--;
		return attotime::zero;
	}

	// wait for the sector being read
	const attotime wait = m_read_done - now;
	m_read_left--;
	m_read_done += SECTOR_TIME;
	return wait;
}

// sectors go over the bus as a burst at the bus rate; at the end of a sector, the
// period is the wait for the next one instead
attotime nscsi_cdrom_pcfx_device::scsi_data_byte_period()
{
	const attotime wait = m_sector_wait;
	m_sector_wait = attotime::zero;
	return wait.is_zero() ? BUS_BYTE_TIME : wait;
}

attotime nscsi_cdrom_pcfx_device::scsi_data_command_delay()
{
	switch (m_command)
	{
		case SC_READ_6: case SC_READ_10: case SC_READ_12:
			return next_sector_wait();   // the first sector; TODO: seek time by distance

		default:
			return attotime::zero;
	}
}

uint8_t nscsi_cdrom_pcfx_device::scsi_get_data(int id, int pos)
{
	// last byte of a sector: the wait for the next one is applied after this byte, before
	// the next sector's first REQ
	if (id == 2 && (pos + 1) % SECTOR_BYTES == 0 && (m_read_left || m_read_buffered))
		m_sector_wait = next_sector_wait();
	return nscsi_cdrom_device::scsi_get_data(id, pos);
}

// any byte received from the initiator (a message after ATN, or the next CDB) cancels a
// pending sector wait that was set on the last data byte of a sector
void nscsi_cdrom_pcfx_device::scsi_put_data(int buf, int offset, uint8_t data)
{
	m_sector_wait = attotime::zero;
	nscsi_cdrom_device::scsi_put_data(buf, offset, data);
}

// ABORT or BUS DEVICE RESET stops audio playback and the read in progress
void nscsi_cdrom_pcfx_device::scsi_abort()
{
	nscsi_cdrom_device::scsi_abort();

	LOGCDDA("Abort: audio stop\n");
	cdda->stop_audio();
	set_cdda_status(CDDA_STOPPED);

	m_read_left = m_read_buffered = 0;
	m_sector_wait = attotime::zero;
}

// start a read of the given number of sectors, the first due one sector time from now
void nscsi_cdrom_pcfx_device::start_read(u32 blocks)
{
	m_read_left = blocks;
	m_read_buffered = 0;
	m_read_done = machine().time() + SECTOR_TIME;
	m_sector_wait = attotime::zero;
}

void nscsi_cdrom_pcfx_device::scsi_command()
{
	// latched here: a data-in command below overwrites m_scsi_cmdbuf with its response before
	// scsi_data_command_delay() is called, so byte 0 of the CDB would no longer be byte 0 of the CDB
	m_command = m_scsi_cmdbuf[0];

	// a new command cancels a read; READs below start their own
	m_read_left = m_read_buffered = 0;
	m_sector_wait = attotime::zero;

	if (!image->exists() && m_scsi_cmdbuf[0] != SC_INQUIRY && m_scsi_cmdbuf[0] != SC_REQUEST_SENSE)
	{
		return_no_cd();   // NOT READY / MEDIUM NOT PRESENT
		return;
	}

	switch (m_scsi_cmdbuf[0])
	{
		case SC_INQUIRY:
		{
			// games check for this drive by its INQUIRY data (batlheat)
			static constexpr u8 INQUIRY[36] =
			{
				0x05, 0x80, 0x02, 0x00, 0x1f, 0x00, 0x00, 0x00,
				'N', 'E', 'C', ' ', ' ', ' ', ' ', ' ',
				'C', 'D', '-', 'R', 'O', 'M', ' ', 'D', 'R', 'I', 'V', 'E', ':', 'F', 'X', ' ',
				'1', '.', '0', ' '
			};
			const int size = std::min<int>(m_scsi_cmdbuf[4], sizeof(INQUIRY));
			std::copy_n(INQUIRY, size, m_scsi_cmdbuf);
			scsi_data_in(SBUF_MAIN, size);
			scsi_status_complete(SS_GOOD);
			break;
		}

		case SC_READ_6:
			cdda->stop_audio();
			set_cdda_status(CDDA_STOPPED);
			start_read(m_scsi_cmdbuf[4] ? m_scsi_cmdbuf[4] : 256);
			nscsi_cdrom_device::scsi_command();
			break;

		case SC_READ_10:
			cdda->stop_audio();
			set_cdda_status(CDDA_STOPPED);
			start_read(get_u16be(&m_scsi_cmdbuf[7]));
			nscsi_cdrom_device::scsi_command();
			break;

		case SC_READ_12:
		{
			cdda->stop_audio();
			set_cdda_status(CDDA_STOPPED);
			const u32 blocks = get_u32be(&m_scsi_cmdbuf[6]);
			// same path as the base READ(10): the LBA is already at the same offset, just
			// narrow the block count to 16 bits (clamped, not truncated) and relabel the CDB
			put_u16be(&m_scsi_cmdbuf[7], std::min<u32>(blocks, 0xffff));
			m_scsi_cmdbuf[0] = SC_READ_10;
			start_read(get_u16be(&m_scsi_cmdbuf[7]));
			nscsi_cdrom_device::scsi_command();
			break;
		}

		case SC_READ_HEADER:  // 44h, used by the BIOS POST
		{
			const u32 lba = get_u32be(&m_scsi_cmdbuf[2]);
			const bool msf = BIT(m_scsi_cmdbuf[1], 1);
			const int size = std::min<int>(get_u16be(&m_scsi_cmdbuf[7]), 8);

			if (lba >= image->get_track_start(0xaa))
			{
				report_condition(SK_ILLEGAL_REQUEST, SKC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE);
				break;
			}

			const int track_type = image->get_track_type(image->get_track(lba));
			if (track_type == cdrom_file::CD_TRACK_AUDIO)
			{
				report_condition(SK_ILLEGAL_REQUEST, SKC_ILLEGAL_MODE_FOR_THIS_TRACK);
				break;
			}

			// header of the addressed data sector: mode, then its address (as t10mmc.cpp)
			const bool mode1 = track_type == cdrom_file::CD_TRACK_MODE1 || track_type == cdrom_file::CD_TRACK_MODE1_RAW;
			std::fill_n(m_scsi_cmdbuf, 8, 0);
			m_scsi_cmdbuf[0] = mode1 ? 1 : 2;
			put_u32be(&m_scsi_cmdbuf[4], msf ? to_msf(lba + 150) : lba);
			scsi_data_in(SBUF_MAIN, size);
			scsi_status_complete(SS_GOOD);
			break;
		}

		// the SCSI-2 audio commands go through the NEC play state, so read sub-channel Q and
		// the sub-code interface see them (Photo CD viewer music uses 47h)
		case SC_PLAY_AUDIO_MSF:
		{
			// binary M:S:F
			const s32 start = (m_scsi_cmdbuf[3] * 60 + m_scsi_cmdbuf[4]) * 75 + m_scsi_cmdbuf[5] - 150;
			const s32 end = (m_scsi_cmdbuf[6] * 60 + m_scsi_cmdbuf[7]) * 75 + m_scsi_cmdbuf[8] - 150;
			if (start < 0 || end < start || u32(start) >= image->get_track_start(0xaa))
				report_bad_cdb_field();
			else
				scsi_play_audio(start, end);
			break;
		}

		case SC_PLAY_AUDIO_TRACK_INDEX:
		{
			// plays up to the start of the end track (games play one track with n/01 to
			// n+1/01 and wait for it to stop); TODO: the index fields are ignored
			const int last_track = image->get_last_track();
			const int start_track = m_scsi_cmdbuf[4];
			const int end_track = m_scsi_cmdbuf[7];
			if (!start_track || start_track > last_track || end_track < start_track)
				report_bad_cdb_field();
			else
				scsi_play_audio(image->get_track_start(start_track - 1), image->get_track_start((end_track > last_track) ? 0xaa : (end_track - 1)));
			break;
		}

		case SC_PAUSE_RESUME:
			if (m_cdda_status == CDDA_STOPPED)
			{
				report_condition(SK_ILLEGAL_REQUEST, NEC_SKC_AUDIO_NOT_PLAYING);
				break;
			}
			cdda->pause_audio(BIT(m_scsi_cmdbuf[8], 0) ^ 1);
			set_cdda_status(BIT(m_scsi_cmdbuf[8], 0) ? CDDA_PLAYING : CDDA_PAUSED);
			scsi_status_complete(SS_GOOD);
			break;

		case 0x0d:  // NEC: no operation
		case 0xdb:  // NEC: set stop time
			scsi_status_complete(SS_GOOD);
			break;

		case 0xd8: nec_set_audio_start_position(); break;
		case 0xd9: nec_set_audio_end_position(); break;
		case 0xda: nec_pause(); break;
		case 0xdd: nec_read_subq(); break;
		case 0xde: nec_get_dir_info(); break;

		default:
			nscsi_cdrom_device::scsi_command();
			break;
	}
}

u8 nscsi_cdrom_pcfx_device::to_bcd(u32 v)
{
	return ((v / 10) << 4) | (v % 10);
}

u8 nscsi_cdrom_pcfx_device::from_bcd(u8 v)
{
	return (v >> 4) * 10 + (v & 0xf);
}

// absolute M:S:F (BCD) of an LBA, including the 2 second offset
void nscsi_cdrom_pcfx_device::put_msf(u8 *dest, s32 lba)
{
	lba += 150;
	dest[0] = to_bcd(lba / (60 * 75));
	dest[1] = to_bcd((lba / 75) % 60);
	dest[2] = to_bcd(lba % 75);
}

// vendor specific commands (c0h-ffh) are 10 bytes long
bool nscsi_cdrom_pcfx_device::scsi_command_done(u8 command, u8 length)
{
	if (command >= 0xc0)
		return length == 10;
	return nscsi_cdrom_device::scsi_command_done(command, length);
}

void nscsi_cdrom_pcfx_device::nec_audio_end(int state)
{
	if (!state || m_cdda_status != CDDA_PLAYING)
		return;

	if (m_cdda_loop)
	{
		LOGCDDA("Audio end: repeat %d-%d\n", m_cdda_start, m_cdda_end);
		cdda->start_audio(m_cdda_start, m_cdda_end - m_cdda_start);
	}
	else
	{
		LOGCDDA("Audio end: stop\n");
		set_cdda_status(CDDA_STOPPED);
	}
}

// address in the NEC audio commands: byte 9 bits 7-6 select LBA, BCD M:S:F or BCD track
bool nscsi_cdrom_pcfx_device::nec_address(const u8 *cdb, u32 &lba)
{
	const int last_track = image->get_last_track();

	switch (cdb[9] & 0xc0)
	{
		case 0x00:
			lba = get_u24be(&cdb[3]);
			return true;

		case 0x40:
			lba = ((from_bcd(cdb[2]) * 60) + from_bcd(cdb[3])) * 75 + from_bcd(cdb[4]) - 150;
			return true;

		case 0x80:
		{
			const int track = from_bcd(cdb[2]);
			if (!track || track > last_track + 1)
				return false;
			lba = image->get_track_start((track == last_track + 1) ? 0xaa : (track - 1));
			return true;
		}

		default:
			return false;
	}
}

void nscsi_cdrom_pcfx_device::nec_play(u32 start, u32 end)
{
	m_cdda_start = start;
	m_cdda_end = end;
	if (end > start)
		cdda->start_audio(start, end - start);
	else
		cdda->stop_audio();
}

// play from start up to end (exclusive) for the SCSI-2 audio commands
void nscsi_cdrom_pcfx_device::scsi_play_audio(u32 start, u32 end)
{
	if (start != end)
	{
		if (image->get_track_type(image->get_track(start)) != cdrom_file::CD_TRACK_AUDIO)
		{
			report_condition(SK_ILLEGAL_REQUEST, SKC_ILLEGAL_MODE_FOR_THIS_TRACK);
			return;
		}

		LOGCMD("SCSI audio play %d-%d\n", start, end);
		m_cdda_loop = false;
		cdda->set_output_gain(ALL_OUTPUTS, 1.0);
		cdda->pause_audio(0);
		nec_play(start, end);
		set_cdda_status(CDDA_PLAYING);
	}
	scsi_status_complete(SS_GOOD);
}

// d8h: set audio playback start position, then play to the end of the disc (byte 1 bit 0) or pause there
void nscsi_cdrom_pcfx_device::nec_set_audio_start_position()
{
	u8 cdb[10];
	std::copy_n(m_scsi_cmdbuf, 10, cdb);

	u32 lba;
	if (!nec_address(cdb, lba) || lba > image->get_track_start(0xaa))
	{
		report_bad_cdb_field();
		return;
	}

	LOGCMD("0xd8 SET AUDIO PLAYBACK START POSITION (NEC): mode %02x lba %d play %d\n", cdb[9] & 0xc0, lba, BIT(cdb[1], 0));

	m_cdda_loop = false;
	cdda->set_output_gain(ALL_OUTPUTS, 1.0);
	nec_play(lba, image->get_track_start(0xaa));
	if (BIT(cdb[1], 0))
	{
		set_cdda_status(CDDA_PLAYING);
	}
	else
	{
		cdda->pause_audio(1);
		set_cdda_status(CDDA_PAUSED);
	}

	scsi_status_complete(SS_GOOD);
}

// d9h: set audio playback end position and play from the start position, byte 1 bits 2-0:
// 0 = muted, 1 = left only, 2 = right only, 3 = stereo, 4 = repeat, 5 = play while busy
// (left/right only can play in stereo on the PC-FX, depending on the drive; play while busy is not emulated)
void nscsi_cdrom_pcfx_device::nec_set_audio_end_position()
{
	u8 cdb[10];
	std::copy_n(m_scsi_cmdbuf, 10, cdb);

	if (m_cdda_status == CDDA_STOPPED)
	{
		report_condition(SK_ILLEGAL_REQUEST, NEC_SKC_AUDIO_NOT_PLAYING);
		return;
	}

	u32 lba;
	if (!nec_address(cdb, lba))
	{
		report_bad_cdb_field();
		return;
	}

	const u32 start = (m_cdda_status != CDDA_PLAYING) ? cdda->get_audio_lba() : m_cdda_start;
	LOGCMD("0xd9 SET AUDIO PLAYBACK END POSITION (NEC): mode %02x play mode %d, %d-%d\n", cdb[9] & 0xc0, cdb[1] & 7, start, lba);

	m_cdda_loop = (cdb[1] & 7) == 4;
	cdda->set_output_gain(ALL_OUTPUTS, ((cdb[1] & 7) == 0) ? 0.0 : 1.0);
	cdda->pause_audio(0);
	nec_play(start, lba);
	m_cdda_start = start;
	set_cdda_status(CDDA_PLAYING);

	scsi_status_complete(SS_GOOD);
}

// dah: still (pause during play)
void nscsi_cdrom_pcfx_device::nec_pause()
{
	if (m_cdda_status == CDDA_STOPPED)
	{
		report_condition(SK_ILLEGAL_REQUEST, NEC_SKC_AUDIO_NOT_PLAYING);
		return;
	}

	LOGCMD("0xda PAUSE (NEC)\n");
	cdda->pause_audio(1);
	set_cdda_status(CDDA_STILL);

	scsi_status_complete(SS_GOOD);
}

// ddh: read sub-channel Q, byte 1 = allocation length
void nscsi_cdrom_pcfx_device::nec_read_subq()
{
	u8 cdb[10];
	std::copy_n(m_scsi_cmdbuf, 10, cdb);

	u8 q[12];
	make_subq((m_cdda_status == CDDA_STOPPED) ? m_cdda_start : cdda->get_audio_lba(), q);

	// status, then Q without its zero byte and CRC
	m_scsi_cmdbuf[0] = m_cdda_status;
	std::copy_n(&q[0], 6, &m_scsi_cmdbuf[1]);
	std::copy_n(&q[7], 3, &m_scsi_cmdbuf[7]);

	scsi_data_in(SBUF_MAIN, std::min<int>(cdb[1], 10));
	scsi_status_complete(SS_GOOD);
}

// Q for an LBA: control/ADR 1, track, index, relative M:S:F (counting down in the pregap), 0,
// absolute M:S:F, CRC-16 (CCITT, inverted)
void nscsi_cdrom_pcfx_device::make_subq(u32 lba, u8 *q)
{
	const u32 track = image->get_track(lba);
	const u32 start = image->get_track_start(track);
	const bool pregap = lba < start;

	// get_adr_control() is ADR in the high nibble, as for READ TOC, Q has the control field first
	const u8 adr_control = image->get_adr_control(track);
	q[0] = ((adr_control & 0x0f) << 4) | (adr_control >> 4);
	q[1] = to_bcd(track + 1);
	q[2] = pregap ? 0 : to_bcd(image->get_track_index(lba));
	put_msf(&q[3], s32(pregap ? (start - lba) : (lba - start)) - 150);
	q[6] = 0;
	put_msf(&q[7], lba);

	u16 crc = 0;
	for (int i = 0; i < 10; i++)
	{
		crc ^= u16(q[i]) << 8;
		for (int bit = 0; bit < 8; bit++)
			crc = (crc << 1) ^ (BIT(crc, 15) ? 0x1021 : 0);
	}
	put_u16be(&q[10], ~crc);
}

void nscsi_cdrom_pcfx_device::set_cdda_status(u8 status)
{
	m_cdda_status = status;
	if (status != CDDA_PLAYING)
		m_subcode_timer->adjust(attotime::never);
	else if (m_subcode_timer->expire().is_never())
	{
		// 98 symbols per sector
		const attotime period = attotime::from_hz(75 * 98);
		m_subcode_symbol = 0;
		m_subcode_timer->adjust(period, 0, period);
	}
}

// one sub-code symbol: S0, S1 (SCOR set), then the 96 P-W symbols of the block; P is not
// passed on, Q comes from the play position at the start of the block, R-W are zero
TIMER_CALLBACK_MEMBER(nscsi_cdrom_pcfx_device::subcode_tick)
{
	u8 symbol;
	if (m_subcode_symbol == 0)
	{
		make_subq(cdda->get_audio_lba(), m_subq);
		symbol = 0x00;
	}
	else if (m_subcode_symbol == 1)
		symbol = 0x80;
	else
	{
		const int n = m_subcode_symbol - 2;
		symbol = BIT(m_subq[n >> 3], 7 - (n & 7)) << 6;
	}
	m_subcode_cb(symbol);

	if (++m_subcode_symbol == 98)
		m_subcode_symbol = 0;
}

// deh: get directory (TOC) information, byte 1 = type
void nscsi_cdrom_pcfx_device::nec_get_dir_info()
{
	u8 cdb[10];
	std::copy_n(m_scsi_cmdbuf, 10, cdb);

	LOGCMD("0xde GET DIR INFO (NEC): type %d track %02x\n", cdb[1] & 3, cdb[2]);

	const int last_track = image->get_last_track();
	int size = 0;
	std::fill_n(m_scsi_cmdbuf, 2 + 10 * 103, 0);

	switch (cdb[1] & 3)
	{
		case 0: // first and last track
			m_scsi_cmdbuf[0] = to_bcd(1);
			m_scsi_cmdbuf[1] = to_bcd(last_track);
			size = 4;
			break;

		case 1: // lead-out
			put_msf(&m_scsi_cmdbuf[0], image->get_track_start(0xaa));
			size = 4;
			break;

		case 2: // track start and control
		{
			const int track = from_bcd(cdb[2]);
			if (track < 1 || track > last_track)
			{
				report_bad_cdb_field();
				return;
			}
			put_msf(&m_scsi_cmdbuf[0], image->get_track_start(track - 1));
			m_scsi_cmdbuf[3] = image->get_adr_control(track - 1) & 0x0f;
			size = 4;
			break;
		}

		case 3: // lead-in Q entries: A0, A1, A2 then each track, or only the one in byte 2
		{
			const u8 match = cdb[2];
			if (match != 0x00 && match != 0xa0 && match != 0xa1 && match != 0xa2 && match != 0xb0)
			{
				report_bad_cdb_field();
				return;
			}

			int offset = 2;
			s32 lead_in = -150;
			auto entry = [this, &offset, &lead_in] (u8 point, s32 plba, u8 control)
			{
				u8 *const e = &m_scsi_cmdbuf[offset];
				e[0] = control << 4;
				e[1] = 0;
				e[2] = point;
				put_msf(&e[3], lead_in++);
				e[6] = 0;
				put_msf(&e[7], plba);
				offset += 10;
			};
			auto control = [this] (int track) { return image->get_adr_control(track) & 0x0f; };

			if (!match || match == 0xa0)
				entry(0xa0, 1 * 75 * 60 - 150, control(0));
			if (!match || match == 0xa1)
				entry(0xa1, last_track * 75 * 60 - 150, control(last_track - 1));
			if (!match || match == 0xa2)
				entry(0xa2, image->get_track_start(0xaa), control(last_track - 1));
			if (!match)
			{
				for (int track = 1; track <= last_track; track++)
					entry(to_bcd(track), image->get_track_start(track - 1), control(track - 1));
			}
			if (match == 0xb0)
				offset += 0x14;

			put_u16be(&m_scsi_cmdbuf[0], offset - 2);
			size = offset;
			break;
		}
	}

	scsi_data_in(SBUF_MAIN, size);
	scsi_status_complete(SS_GOOD);
}
