// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "bus/nscsi/s1410.h"

#define LOG_GENERAL (1U << 0)
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
}

void nscsi_s1410_device::scsi_command()
{
	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_REZERO:
	case SC_REASSIGN_BLOCKS:
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_REQUEST_SENSE:
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT\n");
		{
			hard_disk_info *info = hard_disk_get_info(harddisk);
			auto block = std::make_unique<uint8_t[]>(info->sectorbytes);
			memset(&block[0], 0x6c, info->sectorbytes);
			lba = ((scsi_cmdbuf[1] & 0x1f)<<16) | (scsi_cmdbuf[2]<<8) | scsi_cmdbuf[3];
			for(; lba < (info->cylinders * info->heads * info->sectors); lba++) {
				hard_disk_write(harddisk, lba, block.get());
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_TRACK: {
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			return;
		}

		lba = ((scsi_cmdbuf[1] & 0x1f)<<16) | (scsi_cmdbuf[2]<<8) | scsi_cmdbuf[3];
		blocks = (bytes_per_sector == 256) ? 32 : 17;

		int track_length = blocks*bytes_per_sector;
		auto block = std::make_unique<uint8_t[]>(track_length);
		memset(&block[0], 0x6c, track_length);

		if(!hard_disk_write(harddisk, lba, &block[0])) {
			logerror("%s: HD WRITE ERROR !\n", tag());
			scsi_status_complete(SS_FORMAT_ERROR);
		} else {
			scsi_status_complete(SS_GOOD);
		}
		}
		break;

	case SC_FORMAT_ALT_TRACK:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
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
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), scsi_cmdbuf[0]);
		break;
	}
}

uint8_t nscsi_s1410_device::scsi_get_data(int id, int pos)
{
	switch(scsi_cmdbuf[0]) {
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

	switch(scsi_cmdbuf[0]) {
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
