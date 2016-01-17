// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "machine/nscsi_hd.h"
#include "imagedev/harddriv.h"

const device_type NSCSI_HARDDISK = &device_creator<nscsi_harddisk_device>;

nscsi_harddisk_device::nscsi_harddisk_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	nscsi_full_device(mconfig, NSCSI_HARDDISK, "SCSI HARDDISK", tag, owner, clock, "scsi_harddisk", __FILE__), harddisk(nullptr), lba(0), cur_lba(0), blocks(0), bytes_per_sector(0)
{
}

nscsi_harddisk_device::nscsi_harddisk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	nscsi_full_device(mconfig, type, name, tag, owner, clock, shortname, source), harddisk(nullptr), lba(0), cur_lba(0), blocks(0), bytes_per_sector(0)
{
}

void nscsi_harddisk_device::device_start()
{
	nscsi_full_device::device_start();
	save_item(NAME(block));
	save_item(NAME(lba));
	save_item(NAME(cur_lba));
	save_item(NAME(blocks));
	save_item(NAME(bytes_per_sector));
}

void nscsi_harddisk_device::device_reset()
{
	nscsi_full_device::device_reset();
	harddisk_image_device *hd = subdevice<harddisk_image_device>("image");
	harddisk = hd->get_hard_disk_file();
	if(!harddisk) {
		scsi_id = -1;
		bytes_per_sector = 0;
	} else {
		const hard_disk_info *hdinfo = hard_disk_get_info(harddisk);
		bytes_per_sector = hdinfo->sectorbytes;
	}
	cur_lba = -1;
}

static MACHINE_CONFIG_FRAGMENT(scsi_harddisk)
	MCFG_HARDDISK_ADD("image")
	MCFG_HARDDISK_INTERFACE("scsi_hdd")
MACHINE_CONFIG_END

machine_config_constructor nscsi_harddisk_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_harddisk);
}

UINT8 nscsi_harddisk_device::scsi_get_data(int id, int pos)
{
	if(id != 2)
		return nscsi_full_device::scsi_get_data(id, pos);
	int clba = lba + pos / bytes_per_sector;
	if(clba != cur_lba) {
		cur_lba = clba;
		if(!hard_disk_read(harddisk, cur_lba, block)) {
			logerror("%s: HD READ ERROR !\n", tag().c_str());
			memset(block, 0, sizeof(block));
		}
	}
	return block[pos % bytes_per_sector];
}

void nscsi_harddisk_device::scsi_put_data(int id, int pos, UINT8 data)
{
	if(id != 2) {
		nscsi_full_device::scsi_put_data(id, pos, data);
		return;
	}

	int offset = pos % bytes_per_sector;
	block[offset] = data;
	int clba = lba + pos / bytes_per_sector;
	if(offset == bytes_per_sector-1) {
		if(!hard_disk_write(harddisk, clba, block))
			logerror("%s: HD WRITE ERROR !\n", tag().c_str());
	}
}

void nscsi_harddisk_device::scsi_command()
{
	#if 0
	if (scsi_cmdbuf[0] != SC_READ)
	{
		logerror("%s: ", tag());
		for (int i = 0; i < 6; i++)
		{
			logerror("%02x ", scsi_cmdbuf[i]);
		}
		logerror("\n");
	}
	#endif

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		logerror("%s: command TEST UNIT READY\n", tag().c_str());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_6:
		lba = ((scsi_cmdbuf[1] & 0x1f)<<16) | (scsi_cmdbuf[2]<<8) | scsi_cmdbuf[3];
		blocks = scsi_cmdbuf[4];
		if(!blocks)
			blocks = 256;

		logerror("%s: command READ start=%08x blocks=%04x\n",
					tag().c_str(), lba, blocks);

		scsi_data_in(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_6:
		lba = ((scsi_cmdbuf[1] & 0x1f)<<16) | (scsi_cmdbuf[2]<<8) | scsi_cmdbuf[3];
		blocks = scsi_cmdbuf[4];
		if(!blocks)
			blocks = 256;

		logerror("%s: command WRITE start=%08x blocks=%04x\n",
					tag().c_str(), lba, blocks);

		scsi_data_out(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INQUIRY: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		logerror("%s: command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
					tag().c_str(),
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
			scsi_cmdbuf[0] = 0x00; // device is direct-access (e.g. hard disk)
			scsi_cmdbuf[1] = 0x00; // media is not removable
			scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x01; // response data format = CCS
			// Apple HD SC setup utility needs to see this
			strcpy((char *)&scsi_cmdbuf[8], " SEAGATE");
			strcpy((char *)&scsi_cmdbuf[15], "          ST225N");
			strcpy((char *)&scsi_cmdbuf[31], "1.00");
			scsi_cmdbuf[36] = 0x00; // # of extents high
			scsi_cmdbuf[37] = 0x08; // # of extents low
			scsi_cmdbuf[38] = 0x00; // group 0 commands 0-1f
			scsi_cmdbuf[39] = 0x99; // commands 0,3,4,7
			scsi_cmdbuf[40] = 0xa0; // commands 8, a
			scsi_cmdbuf[41] = 0x27; // commands 12,15,16,17
			scsi_cmdbuf[42] = 0x34; // commands 1a,1b,1d
			scsi_cmdbuf[43] = 0x01; // group 1 commands 20-3f
			scsi_cmdbuf[44] = 0x04;
			scsi_cmdbuf[45] = 0xa0;
			scsi_cmdbuf[46] = 0x01;
			scsi_cmdbuf[47] = 0x18;
			scsi_cmdbuf[48] = 0x07; // group 7 commands e0-ff
			scsi_cmdbuf[49] = 0x00;
			scsi_cmdbuf[50] = 0xa0; // commands 8, a
			scsi_cmdbuf[51] = 0x00;
			scsi_cmdbuf[52] = 0x00;
			scsi_cmdbuf[53] = 0xff; // end of list
			if(size > 54)
				size = 54;
			scsi_data_in(0, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_MODE_SENSE_6: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		logerror("%s: command MODE SENSE 6 lun=%d page=%02x alloc=%02x link=%02x\n",
					tag().c_str(),
					lun, scsi_cmdbuf[2] & 0x3f, scsi_cmdbuf[4], scsi_cmdbuf[5]);
		if(lun) {
			bad_lun();
			return;
		}

		int page = scsi_cmdbuf[2] & 0x3f;
		int size = scsi_cmdbuf[4];
		int pos = 1;
		scsi_cmdbuf[pos++] = 0x00; // medium type
		scsi_cmdbuf[pos++] = 0x00; // WP, cache

		hard_disk_info *info = hard_disk_get_info(harddisk);
		UINT32 dsize = info->cylinders * info->heads * info->sectors - 1;
		scsi_cmdbuf[pos++] = 0x08; // Block descriptor length
		scsi_cmdbuf[pos++] = 0x00;
		scsi_cmdbuf[pos++] = (dsize>>16) & 0xff;
		scsi_cmdbuf[pos++] = (dsize>>8) & 0xff;
		scsi_cmdbuf[pos++] = (dsize & 0xff);
		scsi_cmdbuf[pos++] = 0x00;
		scsi_cmdbuf[pos++] = (info->sectorbytes>>16)&0xff;
		scsi_cmdbuf[pos++] = (info->sectorbytes>>8)&0xff;
		scsi_cmdbuf[pos++] = (info->sectorbytes & 0xff);

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

			case 0x03:  { // Format parameters page
				scsi_cmdbuf[pos++] = 0x83; // PS, page id
				scsi_cmdbuf[pos++] = 0x16; // Page length
				scsi_cmdbuf[pos++] = (info->cylinders * info->heads) >> 8; // Track/zone
				scsi_cmdbuf[pos++] = info->cylinders * info->heads;        // Track/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				scsi_cmdbuf[pos++] = info->sectors >> 8; // Sectors/track
				scsi_cmdbuf[pos++] = info->sectors;      // Sectors/track
				scsi_cmdbuf[pos++] = info->sectorbytes >> 8; // Bytes/sector
				scsi_cmdbuf[pos++] = info->sectorbytes;      // Bytes/sector
				scsi_cmdbuf[pos++] = 0x00; // Interleave
				scsi_cmdbuf[pos++] = 0x00; // Interleave
				scsi_cmdbuf[pos++] = 0x00; // Track skew
				scsi_cmdbuf[pos++] = 0x00; // Track skew
				scsi_cmdbuf[pos++] = 0x00; // Cylinder skew
				scsi_cmdbuf[pos++] = 0x00; // Cylinder skew
				scsi_cmdbuf[pos++] = 0x00; // Sectoring type
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				break;
			}

			case 0x04: { // Rigid drive geometry page
				scsi_cmdbuf[pos++] = 0x84; // PS, page id
				scsi_cmdbuf[pos++] = 0x16; // Page length
				scsi_cmdbuf[pos++] = info->cylinders >> 16; // Cylinders
				scsi_cmdbuf[pos++] = info->cylinders >> 8;  // Cylinders
				scsi_cmdbuf[pos++] = info->cylinders;       // Cylinders
				scsi_cmdbuf[pos++] = info->heads;           // Heads
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                  // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                  // Drive step rate
				scsi_cmdbuf[pos++] = 0x00;                  // Drive step rate
				scsi_cmdbuf[pos++] = 0x00;                  // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                  // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                  // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                  // RPL
				scsi_cmdbuf[pos++] = 0x00;                  // Rotational offset
				scsi_cmdbuf[pos++] = 0x00;                  // Reserved
				scsi_cmdbuf[pos++] = UINT8(10000 >> 8);     // Medium rotation rate
				scsi_cmdbuf[pos++] = UINT8(10000);          // Medium rotation rate
				scsi_cmdbuf[pos++] = 0x00;                  // Reserved
				scsi_cmdbuf[pos++] = 0x00;                  // Reserved
				break;
			}

			case 0x30: { // Apple firmware ID page
				scsi_cmdbuf[pos++] = 0xb0; // cPS, page id
				scsi_cmdbuf[pos++] = 0x16; // Page length
				scsi_cmdbuf[pos++] = 'A';
				scsi_cmdbuf[pos++] = 'P';
				scsi_cmdbuf[pos++] = 'P';
				scsi_cmdbuf[pos++] = 'L';
				scsi_cmdbuf[pos++] = 'E';
				scsi_cmdbuf[pos++] = ' ';
				scsi_cmdbuf[pos++] = 'C';
				scsi_cmdbuf[pos++] = 'O';
				scsi_cmdbuf[pos++] = 'M';
				scsi_cmdbuf[pos++] = 'P';
				scsi_cmdbuf[pos++] = 'U';
				scsi_cmdbuf[pos++] = 'T';
				scsi_cmdbuf[pos++] = 'E';
				scsi_cmdbuf[pos++] = 'R';
				scsi_cmdbuf[pos++] = ',';
				scsi_cmdbuf[pos++] = ' ';
				scsi_cmdbuf[pos++] = 'I';
				scsi_cmdbuf[pos++] = 'N';
				scsi_cmdbuf[pos++] = 'C';
				scsi_cmdbuf[pos++] = ' ';
				scsi_cmdbuf[pos++] = ' ';
				scsi_cmdbuf[pos++] = ' ';
				break;
			}

			default:
				logerror("%s: mode sense page %02x unhandled\n", tag().c_str(), page);
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

	case SC_START_STOP_UNIT:
		logerror("%s: command START STOP UNIT\n", tag().c_str());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		logerror("%s: command READ CAPACITY\n", tag().c_str());

		hard_disk_info *info = hard_disk_get_info(harddisk);
		UINT32 size = info->cylinders * info->heads * info->sectors - 1;

		scsi_cmdbuf[0] = (size>>24) & 0xff;
		scsi_cmdbuf[1] = (size>>16) & 0xff;
		scsi_cmdbuf[2] = (size>>8) & 0xff;
		scsi_cmdbuf[3] = (size & 0xff);
		scsi_cmdbuf[4] = (info->sectorbytes>>24)&0xff;
		scsi_cmdbuf[5] = (info->sectorbytes>>16)&0xff;
		scsi_cmdbuf[6] = (info->sectorbytes>>8)&0xff;
		scsi_cmdbuf[7] = (info->sectorbytes & 0xff);

		scsi_data_in(0, 8);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_10:
		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		logerror("%s: command READ EXTENDED start=%08x blocks=%04x\n",
					tag().c_str(), lba, blocks);

		scsi_data_in(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_10:
		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		logerror("%s: command WRITE EXTENDED start=%08x blocks=%04x\n",
					tag().c_str(), lba, blocks);

		scsi_data_out(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("%s: command %02x ***UNKNOWN***\n", tag().c_str(), scsi_cmdbuf[0]);
		nscsi_full_device::scsi_command();
		break;
	}
}
