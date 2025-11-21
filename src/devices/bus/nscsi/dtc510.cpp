// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

TODO:
- stub on top of s1410;
- pc88va2: irq hangs after pceva2tb HDFORM C: when executing 0x08 command (SC_READ);
- pc9801: hangs similarly around SC_READ;
- Hookup to MZ-2500 SASI i/f;
- Hookup to OMTI-5000 series;

References:
- http://www.bitsavers.org/pdf/sms/omti_5x00/3001206_OMTI_5000_Series_Reference_Aug85.pdf
- DTC-510B manual

**************************************************************************************************/

#include "emu.h"
#include "bus/nscsi/dtc510.h"

#include "multibyte.h"

#define LOG_COMMAND (1U << 1)
#define LOG_DATA    (1U << 2)

#define VERBOSE (LOG_COMMAND | LOG_DATA)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

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
	case 1: return length == 6;
	case 6: return length == 6;
	case 7: return length == 6;
	}
	return false;
}

void nscsi_dtc510_device::scsi_command()
{
	memset(scsi_sense_buffer, 0, sizeof(scsi_sense_buffer));

//	printf("CMD %02x %02x\n", scsi_cmdbuf[0], scsi_cmdbuf[1]);

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_REZERO_UNIT:
	case SC_REASSIGN_BLOCKS:
	case SC_READ:
	case SC_WRITE:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_SEEK:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			scsi_status_complete(SS_GOOD);
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
			memset(&block[0], 0x6c, track_length);

			if(!image->write(lba, &block[0])) {
				logerror("%s: HD WRITE ERROR !\n", tag());
				scsi_status_complete(SS_FORMAT_ERROR);
				scsi_sense_buffer[0] = SK_FORMAT_ERROR;
			} else {
				scsi_status_complete(SS_GOOD);
			}
		}
		break;

	case SC_FORMAT_ALT_TRACK:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
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
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
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

	case 0xc2:

		scsi_data_out(4, 10);
		//scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), scsi_cmdbuf[0]);
		break;
	}
}

uint8_t nscsi_dtc510_device::scsi_get_data(int id, int pos)
{
	switch(scsi_cmdbuf[0]) {
	case SC_READ_SECTOR_BUFFER:
		return block[pos];

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_dtc510_device::scsi_put_data(int id, int pos, uint8_t data)
{

	if (scsi_cmdbuf[0] == 0xc2 && id == 4)
	{
		// printf("scsi_put_data %d %d %02x\n", id, pos, data);
		m_param[pos] = data;
		if (pos == 9)
		{
			scsi_status_complete(SS_GOOD);
		}

		return;
	}

	if(id != 2 && !pos) {
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}

	switch(scsi_cmdbuf[0]) {
	case SC_FORMAT_ALT_TRACK:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		break;

//	case SC_INIT_DRIVE_PARAMS:
//		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
//		params[pos] = data;
//		break;
//
	case SC_WRITE_SECTOR_BUFFER:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		block[pos] = data;
		break;

	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}

// Byte transfer rate (5Mb/s)
attotime nscsi_dtc510_device::scsi_data_byte_period()
{
	return attotime::from_nsec(1600);
}

// Command execution delay
attotime nscsi_dtc510_device::scsi_data_command_delay()
{
	switch(scsi_cmdbuf[0]) {
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
		// average seek time of NEC D5126A hard disk
		return attotime::from_msec(85);

	default:
		return attotime::zero;
	}
}
