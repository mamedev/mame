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
}

harddisk_interface nscsi_harddisk_device::hd_intf = { 0, 0 };

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
	return block[pos & (bytes_per_sector - 1)];
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
		logerror("%s: INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
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

	case SC_START_STOP_UNIT:
		logerror("%s: command START STOP UNIT\n", tag());
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		logerror("%s: command READ CAPACITY\n", tag());

		hard_disk_info *info;
		UINT32 temp;

		info = hard_disk_get_info(harddisk);

		// get # of sectors
		temp = info->cylinders * info->heads * info->sectors;
		temp--;

		scsi_cmdbuf[0] = (temp>>24) & 0xff;
		scsi_cmdbuf[1] = (temp>>16) & 0xff;
		scsi_cmdbuf[2] = (temp>>8) & 0xff;
		scsi_cmdbuf[3] = (temp & 0xff);
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

	default:
		nscsi_full_device::scsi_command();
		break;
	}
}
