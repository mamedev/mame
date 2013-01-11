#include "machine/nscsi_hd.h"
#include "imagedev/harddriv.h"

const device_type NSCSI_HARDDISK = &device_creator<nscsi_harddisk_device>;

nscsi_harddisk_device::nscsi_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	nscsi_full_device(mconfig, NSCSI_HARDDISK, "SCSI HARDDISK", tag, owner, clock)
{
}

void nscsi_harddisk_device::device_start()
{
	nscsi_full_device::device_start();
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

harddisk_interface nscsi_harddisk_device::hd_intf = { NULL, NULL, "scsi_hdd", NULL };

static MACHINE_CONFIG_FRAGMENT(scsi_harddisk)
	MCFG_HARDDISK_CONFIG_ADD("image", nscsi_harddisk_device::hd_intf)
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
			logerror("%s: HD READ ERROR !\n", tag());
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
			logerror("%s: HD WRITE ERROR !\n", tag());
	}
}

void nscsi_harddisk_device::scsi_command()
{
	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		logerror("%s: command TEST UNIT READY\n", tag());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ:
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
			scsi_cmdbuf[0] = 0x00; // device is direct-access (e.g. hard disk)
			scsi_cmdbuf[1] = 0x00; // media is not removable
			scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x02; // response data format = SPC-3 standard
			// Apple HD SC setup utility needs to see this
			strcpy((char *)&scsi_cmdbuf[8], " SEAGATE");
			strcpy((char *)&scsi_cmdbuf[16], "          ST225N");
			strcpy((char *)&scsi_cmdbuf[32], "1.0");
			if(size > 148)
				size = 148;
			scsi_data_in(0, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

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

	case SC_START_STOP_UNIT:
		logerror("%s: command START STOP UNIT\n", tag());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		logerror("%s: command READ CAPACITY\n", tag());

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

	case SC_READ_EXTENDED:
		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		logerror("%s: command READ EXTENDED start=%08x blocks=%04x\n",
					tag(), lba, blocks);

		scsi_data_in(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_EXTENDED:
		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		logerror("%s: command WRITE EXTENDED start=%08x blocks=%04x\n",
					tag(), lba, blocks);

		scsi_data_out(2, blocks*bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	default:
		nscsi_full_device::scsi_command();
		break;
	}
}
