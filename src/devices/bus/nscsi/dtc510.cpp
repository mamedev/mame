// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

TODO:
- stub on top of s1410;
- pc88va2: irq hangs after pceva2tb HDFORM C: when executing 0x08 command (SC_READ);
- pc9801: hangs similarly around SC_READ;
- Hookup to MZ-2500 SASI i/f;
- Hookup to OMTI-5000 series;
- Support LUN properly (should be two disks/tapes for one controller);
- Handle success/error state properly (page 7-4/62)

References:
- http://www.bitsavers.org/pdf/dtc/DTC-500/09-00231A_DTC_500DB_Intelligent_Controllers_Mar86.pdf
- http://www.bitsavers.org/pdf/sms/omti_5x00/3001206_OMTI_5000_Series_Reference_Aug85.pdf
- DTC-510B manual

**************************************************************************************************/

#include "emu.h"
#include "bus/nscsi/dtc510.h"

#include "multibyte.h"

#include <cmath>

#define LOG_COMMAND (1U << 1) // raw for now
//#define LOG_DATA    (1U << 2)
#define LOG_C2      (1U << 3) // log assign parameter data

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGCOMMAND(...)    LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGC2(...)         LOGMASKED(LOG_C2, __VA_ARGS__)


DEFINE_DEVICE_TYPE(NSCSI_DTC510, nscsi_dtc510_device, "scsi_dtc510", "Data Technology DTC-510 5.25 Inch Winchester Disk Controller")

nscsi_dtc510_device::nscsi_dtc510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_DTC510, tag, owner, clock)
{
	std::fill(std::begin(m_param), std::end(m_param), 0);
}

void nscsi_dtc510_device::device_reset()
{
	nscsi_harddisk_device::device_reset();
	m_last_cylinder = -1;
}

void nscsi_dtc510_device::set_seek_timing(uint32_t track_us, uint32_t average_us, uint32_t full_us, uint32_t rpm, uint8_t interleave)
{
	m_seek_track_us = track_us;
	m_seek_range_us = (full_us > track_us) ? (full_us - track_us) : 0;
	m_rpm           = rpm ? rpm : 3600;
	m_interleave    = interleave ? interleave : 1;
	// Fit seek(d) = track + range * (d/ncyl)^exp so an average-length seek (~1/3 of
	// full stroke) costs average_us.
	const double num = double(average_us) - double(track_us);
	const double den = double(full_us)    - double(track_us);
	m_seek_exp = (num > 0.0 && den > 0.0) ? std::log(num / den) / std::log(1.0 / 3.0) : 1.0;
	m_seek_model = true;
}

bool nscsi_dtc510_device::scsi_command_done(uint8_t command, uint8_t length)
{
	switch(command >> 5) {
	case 0: return length == 6;
	case 1: return length == ((command == 0x26) ? 12 : 10);
	case 2: return length == 6;
	case 6: return length == 6;
	case 7: return length == 6;
	}
	return false;
}

void nscsi_dtc510_device::scsi_command()
{
	// TODO: don't clear sense buffer on every command!
	// (mutuated from s1410)
	memset(m_scsi_sense_buffer, 0, sizeof(m_scsi_sense_buffer));

	LOGCOMMAND("CMD %02x %02x %02x %02x %02x %02x\n",
		m_scsi_cmdbuf[0], m_scsi_cmdbuf[1], m_scsi_cmdbuf[2],
		m_scsi_cmdbuf[3], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]
	);

//  if (m_scsi_cmdbuf[0] == 0x01 && m_scsi_cmdbuf[1] == 0x01 && m_scsi_cmdbuf[5] == 0x01)
//      machine().debug_break();

	switch(m_scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_FORMAT_BAD_TRACK:
	case SC_READ:
	case SC_WRITE:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_RECALIBRATE:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			// lba = 0;
			m_last_cylinder = 0;   // recalibrate returns the heads to track 0
			scsi_status_complete(SS_GOOD);
		}
		break;

	case SC_SEEK:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			//lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			scsi_status_complete(SS_GOOD);
		}
		break;

	case SC_REQUEST_SENSE:
		// TODO: hangs if a request sense is issued
		//m_scsi_sense_buffer[0] = SK_NO_ERROR;
		//m_scsi_sense_buffer[1] = (m_seek >> 16) & 0x1f;
		//m_scsi_sense_buffer[2] = (m_seek >> 8) & 0xff;
		//m_scsi_sense_buffer[3] = (m_seek >> 0) & 0xff;
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT\n");
		{
			const auto &info = image->get_info();
			auto block = std::make_unique<uint8_t[]>(info.sectorbytes);
			const u8 fill_byte = m_scsi_cmdbuf[3] == 0 ? 0xe5 : m_scsi_cmdbuf[3];
			memset(&block[0], fill_byte, info.sectorbytes);
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
			memset(&block[0], 0xe5, track_length);

			if(!image->write(lba, &block[0])) {
				logerror("%s: HD WRITE ERROR !\n", tag());
				scsi_status_complete(SS_FORMAT_ERROR);
				m_scsi_sense_buffer[0] = SK_FORMAT_ERROR;
			} else {
				scsi_status_complete(SS_GOOD);
			}
		}
		break;

	case SC_ASSIGN_ALT_TRACK:
	{
		// TODO: unimplemented
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}

		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_WRITE_FILE_MARK:
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

	case SC_ASSIGN_DISK_PARAMETERS:
		scsi_data_out(4, 10);
		//scsi_status_complete(SS_GOOD);
		break;

	case SC_RETENTION:
	case SC_RAM_DIAGNOSTICS:
	case SC_WRITE_ECC:
		scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), m_scsi_cmdbuf[0]);
		scsi_status_complete(SS_CHECK_CONDITION);
		break;
	}
}

uint8_t nscsi_dtc510_device::scsi_get_data(int id, int pos)
{
	switch(m_scsi_cmdbuf[0]) {

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_dtc510_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if (m_scsi_cmdbuf[0] == SC_ASSIGN_DISK_PARAMETERS && id == 4)
	{
		m_param[pos] = data;
		if (pos == 9)
		{
			LOGC2("Step Pulse Width %02x\n", m_param[0]);
			LOGC2("Step Period %02x\n", m_param[1]);
			LOGC2("Step Mode %02x\n", m_param[2]);
			LOGC2("Number of Heads %d\n", m_param[3] + 1);
			LOGC2("Cylinder address %d\n", ((m_param[4] << 8) | m_param[5]) + 1);
			LOGC2("WSI %d\n", (((m_param[7] & 3) << 8) | m_param[6]) + 1);
			LOGC2("H/S %d TYPE %d\n", BIT(m_param[7], 3), (m_param[7] >> 4) & 3);
			LOGC2("Sectors per track %d\n", m_param[8] + 1);
			LOGC2("<reserved %02x>\n", m_param[9]);

			scsi_status_complete(SS_GOOD);
		}

		return;
	}

	if(id != 2 && !pos) {
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}

	switch(m_scsi_cmdbuf[0]) {
	// ...
	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}

// Byte transfer rate (5Mb/s)
attotime nscsi_dtc510_device::scsi_data_byte_period()
{
	// Too slow, won't complete a DMA cycle when fetching IPL for pc9801
	//  return attotime::from_nsec(1600);
	return attotime::from_nsec(1100);
}

// Command execution delay.
attotime nscsi_dtc510_device::scsi_data_command_delay()
{
	switch(m_scsi_cmdbuf[0]) {
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
	case SC_ASSIGN_ALT_TRACK:
	case SC_RETENTION:
		// Legacy default (unchanged unless a driver calls set_seek_timing()): a flat
		// average-seek delay charged on every command.
		if (!m_seek_model)
			return attotime::from_msec(85);
		else
		{
			// A real drive seeks only when the target cylinder changes; sequential I/O on
			// one cylinder pays only rotational latency, scaled by the format interleave.
			// Track the head cylinder and bill streaming or a real seek accordingly.
			u32 spt = 1, spc = 1, ncyl = 1;
			if (image->exists())
			{
				const auto &info = image->get_info();
				spt  = info.sectors ? info.sectors : 1;          // sectors per track
				spc  = info.heads * spt;  if (!spc)  spc  = 1;   // sectors per cylinder
				ncyl = info.cylinders ? info.cylinders : 1;
			}
			const int target = int((get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff) / spc);
			const int dist = (m_last_cylinder < 0) ? int(ncyl)
				: (target > m_last_cylinder ? target - m_last_cylinder
							  : m_last_cylinder - target);
			m_last_cylinder = target;

			const u32 rev_us = 60u * 1000000u / m_rpm;   // one revolution
			u32 us;
			if (dist == 0)
				// Same cylinder: consecutive logical sectors arrive after the interleave
				// gap (interleave revolutions to read one track of spt sectors).
				us = (rev_us * m_interleave) / spt;
			else
				// Cylinder change: ~half a revolution rotational latency + a distance-
				// scaled seek between the track-to-track and full-stroke times.
				us = rev_us / 2 + m_seek_track_us
					+ u32(double(m_seek_range_us) * std::pow(double(dist) / double(ncyl), m_seek_exp));
			return attotime::from_usec(us);
		}

	default:
		return attotime::zero;
	}
}
