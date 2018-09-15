// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "machine/nscsi_cd.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device, "scsi_cdrom", "SCSI CD-ROM")

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_full_device(mconfig, NSCSI_CDROM, tag, owner, clock), cdrom(nullptr), bytes_per_block(bytes_per_sector), lba(0), cur_sector(0), image(*this, "image")
{
}

void nscsi_cdrom_device::device_start()
{
	nscsi_full_device::device_start();
	save_item(NAME(sector_buffer));
	save_item(NAME(lba));
	save_item(NAME(cur_sector));
	save_item(NAME(bytes_per_block));
}

void nscsi_cdrom_device::device_reset()
{
	nscsi_full_device::device_reset();
	cdrom = image->get_cdrom_file();
	lba = 0;
	cur_sector = -1;
}

MACHINE_CONFIG_START(nscsi_cdrom_device::device_add_mconfig)
	MCFG_CDROM_ADD("image")
	MCFG_CDROM_INTERFACE("cdrom")
MACHINE_CONFIG_END

void nscsi_cdrom_device::set_block_size(u32 block_size)
{
	assert_always(!started(), "block size should not be set after device start");
	assert_always(bytes_per_sector % block_size == 0, "block size must be a factor of sector size");

	bytes_per_block = block_size;
};


uint8_t nscsi_cdrom_device::scsi_get_data(int id, int pos)
{
	if(id != 2)
		return nscsi_full_device::scsi_get_data(id, pos);
	const int sector = (lba * bytes_per_block + pos) / bytes_per_sector;
	const int extra_pos = (lba * bytes_per_block) % bytes_per_sector;
	if(sector != cur_sector) {
		cur_sector = sector;
		if(!cdrom_read_data(cdrom, sector, sector_buffer, CD_TRACK_MODE1)) {
			LOG("CD READ ERROR sector %d!\n", sector);
			std::fill_n(sector_buffer, sizeof(sector_buffer), 0);
		}
	}
	return sector_buffer[(pos + extra_pos) & (bytes_per_sector - 1)];
}

void nscsi_cdrom_device::return_no_cd()
{
	sense(false, 3);
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_cdrom_device::scsi_command()
{
	int blocks;

	// check for media change
	if((cdrom != image->get_cdrom_file()) && (scsi_cmdbuf[0] != SC_INQUIRY))
	{
		// clear media change condition
		cdrom = image->get_cdrom_file();
		cur_sector = -1;

		// report unit attention condition
		sense(false, 6);
		scsi_status_complete(SS_CHECK_CONDITION);
		return;
	}

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		LOG("command TEST UNIT READY\n");
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

		LOG("command READ start=%08x blocks=%04x\n", lba, blocks);

		scsi_data_in(2, blocks*bytes_per_block);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INQUIRY: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		LOG("command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
					lun, scsi_cmdbuf[1] & 1, scsi_cmdbuf[2], scsi_cmdbuf[4], scsi_cmdbuf[5]);
		if(lun) {
			bad_lun();
			return;
		}

		int page = scsi_cmdbuf[2];
		int size = scsi_cmdbuf[4];
		switch(page) {
		case 0:
			std::fill_n(scsi_cmdbuf, 36, 0);

			// vendor and product information must be padded with spaces
			std::fill_n(&scsi_cmdbuf[8], 28, 0x20);

			scsi_cmdbuf[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
			scsi_cmdbuf[1] = 0x80; // media is removable
			scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x02; // response data format = SPC-3 standard
			scsi_cmdbuf[4] = 32; // additional length
			// some Konami games freak out if this isn't "Sony", so we'll lie
			// this is the actual drive on my Nagano '98 board
			strncpy((char *)&scsi_cmdbuf[8], "Sony", 4);
			strncpy((char *)&scsi_cmdbuf[16], "CDU-76S", 7);
			strncpy((char *)&scsi_cmdbuf[32], "1.0", 3);
			if(size > 36)
				size = 36;
			scsi_data_in(SBUF_MAIN, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_START_STOP_UNIT:
		LOG("command %s UNIT%s\n", (scsi_cmdbuf[4] & 0x1) ? "START" : "STOP",
			(scsi_cmdbuf[4] & 0x2) ? (scsi_cmdbuf[4] & 0x1) ? " (LOAD)" : " (EJECT)" : "");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		if(!cdrom) {
			return_no_cd();
			break;
		}

		LOG("command READ CAPACITY\n");

		// get the last used block on the disc
		const uint32_t temp = cdrom_get_track_start(cdrom, 0xaa) * (bytes_per_sector / bytes_per_block) - 1;

		scsi_cmdbuf[0] = (temp>>24) & 0xff;
		scsi_cmdbuf[1] = (temp>>16) & 0xff;
		scsi_cmdbuf[2] = (temp>>8) & 0xff;
		scsi_cmdbuf[3] = (temp & 0xff);
		scsi_cmdbuf[4] = 0;
		scsi_cmdbuf[5] = 0;
		scsi_cmdbuf[6] = (bytes_per_block>>8)&0xff;
		scsi_cmdbuf[7] = (bytes_per_block & 0xff);

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

		LOG("command READ EXTENDED start=%08x blocks=%04x\n", lba, blocks);

		scsi_data_in(2, blocks*bytes_per_block);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_MODE_SENSE_6: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		LOG("command MODE SENSE 6 lun=%d page=%02x alloc=%02x link=%02x\n",
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

		// get the last used block on the disc
		const uint32_t temp = cdrom_get_track_start(cdrom, 0xaa) * (bytes_per_sector / bytes_per_block) - 1;
		scsi_cmdbuf[pos++] = 0x08; // Block descriptor length

		scsi_cmdbuf[pos++] = 0x00; // density code
		scsi_cmdbuf[pos++] = (temp>>16) & 0xff;
		scsi_cmdbuf[pos++] = (temp>>8) & 0xff;
		scsi_cmdbuf[pos++] = (temp & 0xff);
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = (bytes_per_block>>8)&0xff;
		scsi_cmdbuf[pos++] = (bytes_per_block & 0xff);

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
				LOG("mode sense page %02x unhandled\n", page);
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

	case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
		// TODO: support eject prevention
		LOG("command %s MEDIUM REMOVAL\n", (scsi_cmdbuf[4] & 0x1) ? "PREVENT" : "ALLOW");
		scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("unhandled command %02x\n", scsi_cmdbuf[0]);

		nscsi_full_device::scsi_command();
		break;
	}
}
