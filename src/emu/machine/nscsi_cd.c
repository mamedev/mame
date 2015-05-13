// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "machine/nscsi_cd.h"
#include "imagedev/chd_cd.h"

const device_type NSCSI_CDROM = &device_creator<nscsi_cdrom_device>;

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	nscsi_full_device(mconfig, NSCSI_CDROM, "SCSI CDROM", tag, owner, clock, "scsi_cdrom", __FILE__)
{
}

void nscsi_cdrom_device::device_start()
{
	nscsi_full_device::device_start();
	bytes_per_sector = 2048;
	save_item(NAME(block));
	save_item(NAME(lba));
	save_item(NAME(cur_lba));
	save_item(NAME(blocks));
	save_item(NAME(bytes_per_sector));
}

void nscsi_cdrom_device::device_reset()
{
	nscsi_full_device::device_reset();
	cdrom = subdevice<cdrom_image_device>("image")->get_cdrom_file();
	lba = 0;
	blocks = 0;
	cur_lba = -1;
}

static MACHINE_CONFIG_FRAGMENT(scsi_cdrom)
	MCFG_CDROM_ADD("image")
	MCFG_CDROM_INTERFACE("cdrom")
MACHINE_CONFIG_END

machine_config_constructor nscsi_cdrom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_cdrom);
}

UINT8 nscsi_cdrom_device::scsi_get_data(int id, int pos)
{
	if(id != 2)
		return nscsi_full_device::scsi_get_data(id, pos);
	int clba = lba + pos / bytes_per_sector;
	if(clba != cur_lba) {
		cur_lba = clba;
		if(!cdrom_read_data(cdrom, cur_lba, block, CD_TRACK_MODE1)) {
			logerror("%s: CD READ ERROR !\n", tag());
			memset(block, 0, sizeof(block));
		}
	}
	return block[pos & (bytes_per_sector - 1)];
}

void nscsi_cdrom_device::return_no_cd()
{
	sense(false, 3);
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_cdrom_device::scsi_command()
{
	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		logerror("%s: command TEST UNIT READY\n", tag());
		if(cdrom)
			scsi_status_complete(SS_GOOD);
		else
			return_no_cd();
		break;

	case SC_READ_6:
		if(!cdrom) {
			return_no_cd();
			break;
		}

		lba = ((scsi_cmdbuf[1] & 0x1f)<<16) | (scsi_cmdbuf[2]<<8) | scsi_cmdbuf[3];
		blocks = scsi_cmdbuf[4];
		if(!blocks)
			blocks = 256;

		logerror("%s: command READ start=%08x blocks=%04x\n",
					tag(), lba, blocks);

		scsi_data_in(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INQUIRY: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		logerror("%s: command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
					tag(),
					lun, scsi_cmdbuf[1] & 1, scsi_cmdbuf[2], scsi_cmdbuf[4], scsi_cmdbuf[5]);
		if(lun) {
			bad_lun();
			return;
		}

		int page = scsi_cmdbuf[2];
		int size = scsi_cmdbuf[4];
		switch(page) {
		case 0:
			memset(scsi_cmdbuf, 0, 148);
			scsi_cmdbuf[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
			scsi_cmdbuf[1] = 0x80; // media is removable
			scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x02; // response data format = SPC-3 standard
			// some Konami games freak out if this isn't "Sony", so we'll lie
			// this is the actual drive on my Nagano '98 board
			strcpy((char *)&scsi_cmdbuf[8], "Sony");
			strcpy((char *)&scsi_cmdbuf[16], "CDU-76S");
			strcpy((char *)&scsi_cmdbuf[32], "1.0");
			if(size > 148)
				size = 148;
			scsi_data_in(SBUF_MAIN, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_START_STOP_UNIT:
		logerror("%s: command START STOP UNIT\n", tag());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		if(!cdrom) {
			return_no_cd();
			break;
		}

		logerror("%s: command READ CAPACITY\n", tag());

		UINT32 temp = cdrom_get_track_start(cdrom, 0xaa);
		temp--; // return the last used block on the disc

		scsi_cmdbuf[0] = (temp>>24) & 0xff;
		scsi_cmdbuf[1] = (temp>>16) & 0xff;
		scsi_cmdbuf[2] = (temp>>8) & 0xff;
		scsi_cmdbuf[3] = (temp & 0xff);
		scsi_cmdbuf[4] = 0;
		scsi_cmdbuf[5] = 0;
		scsi_cmdbuf[6] = (bytes_per_sector>>8)&0xff;
		scsi_cmdbuf[7] = (bytes_per_sector & 0xff);

		scsi_data_in(SBUF_MAIN, 8);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_10:
		if(!cdrom) {
			return_no_cd();
			break;
		}

		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		logerror("%s: command READ EXTENDED start=%08x blocks=%04x\n",
					tag(), lba, blocks);

		scsi_data_in(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_MODE_SENSE_6: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		logerror("%s: command MODE SENSE 6 lun=%d page=%02x alloc=%02x link=%02x\n",
					tag(),
					lun, scsi_cmdbuf[2] & 0x3f, scsi_cmdbuf[4], scsi_cmdbuf[5]);
		if(lun) {
			bad_lun();
			return;
		}

		int page = scsi_cmdbuf[2] & 0x3f;
		int size = scsi_cmdbuf[4];
		int pos = 1;
		scsi_cmdbuf[pos++] = 0x00; // medium type
		scsi_cmdbuf[pos++] = 0x80; // WP, cache

		UINT32 temp = cdrom_get_track_start(cdrom, 0xaa);
		temp--; // return the last used block on the disc
		scsi_cmdbuf[pos++] = 0x08; // Block descriptor length

		scsi_cmdbuf[pos++] = (temp>>24) & 0xff;
		scsi_cmdbuf[pos++] = (temp>>16) & 0xff;
		scsi_cmdbuf[pos++] = (temp>>8) & 0xff;
		scsi_cmdbuf[pos++] = (temp & 0xff);
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = (bytes_per_sector>>8)&0xff;
		scsi_cmdbuf[pos++] = (bytes_per_sector & 0xff);

		int pmax = page == 0x3f ? 0x3e : page;
		int pmin = page == 0x3f ? 0x00 : page;
		for(int page=pmax; page >= pmin; page--) {
			switch(page) {
			case 0x00: // Unit attention parameters page (weird)
				scsi_cmdbuf[pos++] = 0x80; // PS, page id
				scsi_cmdbuf[pos++] = 0x02; // Page length
				scsi_cmdbuf[pos++] = 0x00; // Meh
				scsi_cmdbuf[pos++] = 0x00; // Double meh
				break;

			case 0x02: // Disconnect/reconnect control parameters (guessed)
				scsi_cmdbuf[pos++] = 0x82; // PS, page id
				scsi_cmdbuf[pos++] = 0x0e; // Page length
				scsi_cmdbuf[pos++] = 0xe6; // Buffer full ratio, 90%
				scsi_cmdbuf[pos++] = 0x1a; // Buffer empty ratio, 10%
				scsi_cmdbuf[pos++] = 0x00; // Bus inactivity limit, 0
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00; // Disconnect time limit, 0
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00; // Connect time limit, 0
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00; // Maximum burst size, 0
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00; // EMDP, Dimm, DTDC
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				break;

			default:
				logerror("%s: mode sense page %02x unhandled\n", tag(), page);
				break;
			}
		}
		scsi_cmdbuf[0] = pos;
		if(pos > size)
			pos = size;

		scsi_data_in(0, pos);
		scsi_status_complete(SS_GOOD);
		break;
	}

	default:
		fprintf(stderr, "scsi %02x\n", scsi_cmdbuf[0]);

		nscsi_full_device::scsi_command();
		break;
	}
}
