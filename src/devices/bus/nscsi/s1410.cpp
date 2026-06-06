// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "bus/nscsi/s1410.h"

#include "multibyte.h"

#include <cmath>

#define LOG_COMMAND (1U << 1)
#define LOG_DATA    (1U << 2)

#define VERBOSE 0

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_S1410, nscsi_s1410_device, "scsi_s1410", "Xebec S1410 5.25 Inch Winchester Disk Controller")

nscsi_s1410_device::nscsi_s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_S1410, tag, owner, clock)
{
}

void nscsi_s1410_device::device_reset()
{
	nscsi_harddisk_device::device_reset();

	// initialize drive characteristics
	params[0] = 0;
	params[1] = 153;
	params[2] = 4;
	params[3] = 0;
	params[4] = 128;
	params[5] = 0;
	params[6] = 64;
	params[7] = 11;

	m_last_cylinder = -1;
}

void nscsi_s1410_device::set_seek_timing(uint32_t track_us, uint32_t average_us, uint32_t full_us, uint32_t rpm, uint8_t interleave)
{
	m_seek_track_us = track_us;
	m_seek_range_us = (full_us > track_us) ? (full_us - track_us) : 0;
	m_rpm           = rpm ? rpm : 3600;
	m_interleave    = interleave ? interleave : 1;
	// Fit seek(d) = track + range * (d/ncyl)^exp so an average-length seek
	// (~1/3 of full stroke) costs average_us.
	const double num = double(average_us) - double(track_us);
	const double den = double(full_us)    - double(track_us);
	m_seek_exp = (num > 0.0 && den > 0.0) ? std::log(num / den) / std::log(1.0 / 3.0) : 1.0;
	m_seek_model = true;
}

bool nscsi_s1410_device::scsi_command_done(uint8_t command, uint8_t length)
{
	if(!length)
		return false;
	switch(command >> 5) {
	case 0: return length == 6;
	case 1: return true;
	case 2: return true;
	case 3: return true;
	case 4: return true;
	case 5: return true;
	case 6: return true;
	case 7: return length == 6;
	}
	return true;
}

void nscsi_s1410_device::scsi_command()
{
	memset(m_scsi_sense_buffer, 0, sizeof(m_scsi_sense_buffer));

	switch(m_scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_REZERO_UNIT:
	case SC_REASSIGN_BLOCKS:
	case SC_READ:
	case SC_WRITE:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_SEEK:
		{
			const auto &info = image->get_info();
			int max_lba = (info.cylinders * info.heads * info.sectors) - 1;
			lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			if (lba <= max_lba) {
				scsi_status_complete(SS_GOOD);
			} else {
				scsi_status_complete(SS_SEEK_ERROR);
				m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			}
		}
		break;

	case SC_REQUEST_SENSE:
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT\n");
		{
			const auto &info = image->get_info();
			auto block = std::make_unique<uint8_t[]>(info.sectorbytes);
			memset(&block[0], 0x6c, info.sectorbytes);
			lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			for(; lba < (info.cylinders * info.heads * info.sectors); lba++) {
				image->write(lba, block.get());
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_TRACK: {
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}

		lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
		blocks = (bytes_per_sector == 256) ? 32 : 17;

		int track_length = blocks*bytes_per_sector;
		auto block = std::make_unique<uint8_t[]>(track_length);
		memset(&block[0], 0x6c, track_length);

		if(!image->write(lba, &block[0])) {
			logerror("%s: HD WRITE ERROR !\n", tag());
			scsi_status_complete(SS_FORMAT_ERROR);
			m_scsi_sense_buffer[0] = SK_FORMAT_ERROR;
		} else {
			scsi_status_complete(SS_GOOD);
		}
		}
		break;

	case SC_FORMAT_ALT_TRACK:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}

		scsi_data_out(2, 3);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INIT_DRIVE_PARAMS:
		scsi_data_out(2, 8);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_SECTOR_BUFFER:
		scsi_data_out(2, bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_SECTOR_BUFFER:
		scsi_data_in(2, bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_CHECK_TRACK_FORMAT:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_ECC_BURST:
	case SC_RAM_DIAG:
	case SC_DRIVE_DIAG:
	case SC_CONTROLLER_DIAG:
	case SC_READ_LONG:
	case SC_WRITE_LONG:
		scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), m_scsi_cmdbuf[0]);
		break;
	}
}

uint8_t nscsi_s1410_device::scsi_get_data(int id, int pos)
{
	switch(m_scsi_cmdbuf[0]) {
	case SC_READ_SECTOR_BUFFER:
		return block[pos];

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_s1410_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if(id != 2 && !pos) {
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}

	switch(m_scsi_cmdbuf[0]) {
	case SC_FORMAT_ALT_TRACK:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		break;

	case SC_INIT_DRIVE_PARAMS:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		params[pos] = data;
		break;

	case SC_WRITE_SECTOR_BUFFER:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		block[pos] = data;
		break;

	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}

// Byte transfer rate (5Mb/s)
attotime nscsi_s1410_device::scsi_data_byte_period()
{
	return attotime::from_nsec(1600);
}

// Command execution delay
attotime nscsi_s1410_device::scsi_data_command_delay()
{
	switch(m_scsi_cmdbuf[0]) {
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
	{
		// Legacy default (unchanged unless a driver calls set_seek_timing()):
		// a flat average-seek delay charged on every command, matching the
		// older NEC D5126A approximation used by the existing s1410 users.
		if (!m_seek_model)
			return attotime::from_msec(85);

		// A real drive seeks only when the target cylinder changes; sequential
		// I/O on one cylinder pays only rotational latency, scaled by the
		// format interleave.  Track the head cylinder and bill streaming or a
		// real seek accordingly.  For the ST-506/ST-412 the model fits
		// track-to-track 3 ms, average 85 ms and full-stroke 205 ms with
		// exp ~= 0.82, capturing the drive's "burst mode" multi-cylinder seek
		// (faster than naive 3 ms/cyl stepping for long seeks, but still cheap
		// for short ones).
		uint32_t spt = 1, spc = 1, ncyl = 1;
		if (image->exists())
		{
			const auto &info = image->get_info();
			spt  = info.sectors ? info.sectors : 1;
			spc  = info.heads * spt;  if (!spc)  spc = 1;
			ncyl = info.cylinders ? info.cylinders : 1;
		}
		const int target = int((get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff) / spc);
		const int dist = (m_last_cylinder < 0) ? int(ncyl)
			: (target > m_last_cylinder ? target - m_last_cylinder
						  : m_last_cylinder - target);
		m_last_cylinder = target;

		const uint32_t rev_us = 60u * 1000000u / m_rpm;
		uint32_t us;
		if (dist == 0)
			us = (rev_us * m_interleave) / spt;
		else
			us = rev_us / 2 + m_seek_track_us
				+ uint32_t(double(m_seek_range_us) * std::pow(double(dist) / double(ncyl), m_seek_exp));
		return attotime::from_usec(us);
	}

	default:
		return attotime::zero;
	}
}
