// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "machine/nscsi_s1410.h"

const device_type NSCSI_S1410 = &device_creator<nscsi_s1410_device>;

nscsi_s1410_device::nscsi_s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	nscsi_harddisk_device(mconfig, NSCSI_S1410, "S1410", tag, owner, clock, "scsi_s1410", __FILE__)
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
	case SC_REQUEST_SENSE:
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

	case SC_FORMAT_UNIT:
		if (scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			return;
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
		dynamic_buffer data(track_length);
		memset(&data[0], 0xc6, track_length);

		if(!hard_disk_write(harddisk, lba, &data[0])) {
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

		scsi_data_in(2, 3);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INIT_DRIVE_PARAMS:
		scsi_data_in(2, 8);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_SECTOR_BUFFER:
		scsi_data_in(2, 512);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_SECTOR_BUFFER:
		scsi_data_out(2, 512);
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

UINT8 nscsi_s1410_device::scsi_get_data(int id, int pos)
{
	switch(scsi_cmdbuf[0]) {
	case SC_READ_SECTOR_BUFFER:
		return block[pos];

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_s1410_device::scsi_put_data(int id, int pos, UINT8 data)
{
	switch(scsi_cmdbuf[0]) {
	case SC_FORMAT_ALT_TRACK:
		break;

	case SC_INIT_DRIVE_PARAMS:
		params[pos] = data;
		break;

	case SC_WRITE_SECTOR_BUFFER:
		block[pos] = data;
		break;

	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}
