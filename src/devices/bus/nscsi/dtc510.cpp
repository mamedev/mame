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

#define LOG_COMMAND (1U << 1) // raw for now
//#define LOG_DATA    (1U << 2)
#define LOG_C2      (1U << 3) // log assign parameter data

#define VERBOSE (LOG_GENERAL | LOG_COMMAND)
//#define LOG_OUTPUT_FUNC osd_printf_warning

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
	memset(scsi_sense_buffer, 0, sizeof(scsi_sense_buffer));

	LOGCOMMAND("CMD %02x %02x %02x %02x %02x %02x\n",
		scsi_cmdbuf[0], scsi_cmdbuf[1], scsi_cmdbuf[2],
		scsi_cmdbuf[3], scsi_cmdbuf[4], scsi_cmdbuf[5]
	);

//  if (scsi_cmdbuf[0] == 0x01 && scsi_cmdbuf[1] == 0x01 && scsi_cmdbuf[5] == 0x01)
//      machine().debug_break();

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_FORMAT_BAD_TRACK:
	case SC_READ:
	case SC_WRITE:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_RECALIBRATE:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			// lba = 0;
			scsi_status_complete(SS_GOOD);
		}
		break;

	case SC_SEEK:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			//lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
			scsi_status_complete(SS_GOOD);
		}
		break;

	case SC_REQUEST_SENSE:
		// TODO: hangs if a request sense is issued
		//scsi_sense_buffer[0] = SK_NO_ERROR;
		//scsi_sense_buffer[1] = (m_seek >> 16) & 0x1f;
		//scsi_sense_buffer[2] = (m_seek >> 8) & 0xff;
		//scsi_sense_buffer[3] = (m_seek >> 0) & 0xff;
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT\n");
		{
			const auto &info = image->get_info();
			auto block = std::make_unique<uint8_t[]>(info.sectorbytes);
			const u8 fill_byte = scsi_cmdbuf[3] == 0 ? 0xe5 : scsi_cmdbuf[3];
			memset(&block[0], fill_byte, info.sectorbytes);
			lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
			for(; lba < (info.cylinders * info.heads * info.sectors); lba++) {
				image->write(lba, block.get());
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_TRACK: {
			if (scsi_cmdbuf[1] >> 5) {
				scsi_status_complete(SS_NOT_READY);
				scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
				return;
			}

			lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
			blocks = (bytes_per_sector == 256) ? 32 : 17;

			int track_length = blocks*bytes_per_sector;
			auto block = std::make_unique<uint8_t[]>(track_length);
			memset(&block[0], 0xe5, track_length);

			if(!image->write(lba, &block[0])) {
				logerror("%s: HD WRITE ERROR !\n", tag());
				scsi_status_complete(SS_FORMAT_ERROR);
				scsi_sense_buffer[0] = SK_FORMAT_ERROR;
			} else {
				scsi_status_complete(SS_GOOD);
			}
		}
		break;

	case SC_ASSIGN_ALT_TRACK:
	{
		// TODO: unimplemented
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
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
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
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
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), scsi_cmdbuf[0]);
		scsi_status_complete(SS_CHECK_CONDITION);
		break;
	}
}

uint8_t nscsi_dtc510_device::scsi_get_data(int id, int pos)
{
	switch(scsi_cmdbuf[0]) {

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_dtc510_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if (scsi_cmdbuf[0] == SC_ASSIGN_DISK_PARAMETERS && id == 4)
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

	switch(scsi_cmdbuf[0]) {
	// ...
	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}

// Byte transfer rate (5Mb/s)
attotime nscsi_dtc510_device::scsi_data_byte_period()
{
	// Too slow, won't complete a DMA cycle when fetching IPL for pc9801
	//	return attotime::from_nsec(1600);
	return attotime::from_nsec(1100);
}

// Command execution delay
attotime nscsi_dtc510_device::scsi_data_command_delay()
{
	switch(scsi_cmdbuf[0]) {
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
	case SC_ASSIGN_ALT_TRACK:
	case SC_RETENTION:
		// average seek time of NEC D5126A hard disk
		return attotime::from_msec(85);

	default:
		return attotime::zero;
	}
}
