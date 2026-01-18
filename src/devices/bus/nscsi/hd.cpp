// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "bus/nscsi/hd.h"
#include "imagedev/harddriv.h"

#include "multibyte.h"

#define LOG_COMMAND     (1U << 1)
#define LOG_DATA        (1U << 2)
#define LOG_UNSUPPORTED (1U << 3)

#define VERBOSE 3

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_HARDDISK, nscsi_harddisk_device, "scsi_harddisk", "SCSI Hard Disk")

nscsi_harddisk_device::nscsi_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_HARDDISK, tag, owner, clock)
{
}

nscsi_harddisk_device::nscsi_harddisk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_full_device(mconfig, type, tag, owner, clock), image(*this, "image"), lba(0), cur_lba(0), blocks(0), bytes_per_sector(0)
{
	m_default_model_name =
		" SEAGATE"
        "          ST225N"
		"1.00";
}

void nscsi_harddisk_device::set_default_model_name(const std::string_view &model)
{
	m_default_model_name = model;
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
	if(!image->exists()) {
		scsi_id = -1;
		bytes_per_sector = 0;
	} else {
		const auto &hdinfo = image->get_info();
		bytes_per_sector = hdinfo.sectorbytes;
		image->get_inquiry_data(m_inquiry_data);
	}
	cur_lba = -1;
}

void nscsi_harddisk_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, image).set_interface("scsi_hdd,hdd");
}

uint8_t nscsi_harddisk_device::scsi_get_data(int id, int pos)
{
	uint8_t data = 0;
	if(id != 2)
	{
		data = nscsi_full_device::scsi_get_data(id, pos);
	} else {
		int clba = lba + pos / bytes_per_sector;
		if(clba != cur_lba) {
			cur_lba = clba;
			if(!image->read(cur_lba, block)) {
				LOG("HD READ ERROR !\n");
				memset(block, 0, sizeof(block));
			}
		}
		data = block[pos % bytes_per_sector];
	}
	LOGMASKED(LOG_DATA, "nscsi_hd: scsi_get_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
	return data;
}

void nscsi_harddisk_device::scsi_put_data(int id, int pos, uint8_t data)
{
	LOGMASKED(LOG_DATA, "nscsi_hd: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
	if(id != 2) {
		nscsi_full_device::scsi_put_data(id, pos, data);
		return;
	}

	int offset = pos % bytes_per_sector;
	block[offset] = data;
	cur_lba = lba + pos / bytes_per_sector;
	if(offset == bytes_per_sector-1) {
		if(!image->write(cur_lba, block))
			LOG("HD WRITE ERROR !\n");
	}
}

void nscsi_harddisk_device::scsi_command()
{
	if(scsi_cmdbuf[0] != SC_READ_6) {
		LOGMASKED(LOG_COMMAND, "%02x %02x %02x %02x %02x %02x\n",
			scsi_cmdbuf[0], scsi_cmdbuf[1], scsi_cmdbuf[2],
			scsi_cmdbuf[3], scsi_cmdbuf[4], scsi_cmdbuf[5]);
	}

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		LOG("command TEST UNIT READY\n");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_6:
		lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
		blocks = scsi_cmdbuf[4];
		if(!blocks)
			blocks = 256;

		LOG("command READ start=%08x blocks=%04x\n", lba, blocks);

		if(image->read(lba, block)) {
			scsi_data_in(2, blocks*bytes_per_sector);
			scsi_status_complete(SS_GOOD);
		} else {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;

	case SC_WRITE_6:
		lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
		blocks = scsi_cmdbuf[4];
		if(!blocks)
			blocks = 256;

		LOG("command WRITE start=%08x blocks=%04x\n", lba, blocks);

		if(image->write(lba, block)) {
			scsi_data_out(2, blocks*bytes_per_sector);
			scsi_status_complete(SS_GOOD);
		} else {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;

	case SC_INQUIRY: {
		int lun = get_lun(scsi_cmdbuf[1] >> 5);
		LOG("command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
			lun, scsi_cmdbuf[1] & 1, scsi_cmdbuf[2], scsi_cmdbuf[4], scsi_cmdbuf[5]);

		int page = scsi_cmdbuf[2];
		int size = scsi_cmdbuf[4];
		switch(page) {
		case 0:
			std::fill_n(scsi_cmdbuf, 148, 0);

			// vendor and product information must be padded with spaces
			std::fill_n(&scsi_cmdbuf[8], 28, 0x20);

			// From Seagate SCSI Commands Reference Manual (http://www.seagate.com/staticfiles/support/disc/manuals/scsi/100293068a.pdf), page 73:
			// If the SCSI target device is not capable of supporting a peripheral device connected to this logical unit, the
			// device server shall set these fields to 7Fh (i.e., PERIPHERAL QUALIFIER field set to 011b and PERIPHERAL DEVICE
			// TYPE set to 1Fh).
			if (lun != 0)
				scsi_cmdbuf[0] = 0x7f;
			else
				scsi_cmdbuf[0] = 0x00; // device is direct-access (e.g. hard disk)
			scsi_cmdbuf[1] = 0x00; // media is not removable
			scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x01; // response data format = CCS
			scsi_cmdbuf[4] = 52;   // additional length
			if(m_inquiry_data.empty()) {
				LOG("IDNT tag not found in chd metadata, using default inquiry data\n");

				// Apple HD SC setup utility needs to see this
				memset(scsi_cmdbuf+8, ' ', 28);
				int len = m_default_model_name.size();
				if(len > 28)
					len = 28;
				memcpy(scsi_cmdbuf+8, m_default_model_name.data(), len);
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
			}
			else
				std::copy_n(m_inquiry_data.begin(), std::min(m_inquiry_data.size(), size_t(48)), &scsi_cmdbuf[8]);

			if(size > 56)
				size = 56;
			scsi_data_in(0, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

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
		scsi_cmdbuf[pos++] = 0x00; // WP, cache

		const auto &info = image->get_info();
		uint32_t dsize = info.cylinders * info.heads * info.sectors - 1;
		scsi_cmdbuf[pos++] = 0x08; // Block descriptor length
		scsi_cmdbuf[pos++] = 0x00;
		put_u24be(&scsi_cmdbuf[pos], dsize);
		pos += 3;
		scsi_cmdbuf[pos++] = 0x00;
		put_u24be(&scsi_cmdbuf[pos], info.sectorbytes);
		pos += 3;

		int pmax = page == 0x3f ? 0x3e : page;
		int pmin = page == 0x3f ? 0x00 : page;

		bool fail = false;
		for(int p=pmax; p >= pmin; p--) {
			switch(p) {
			case 0x00: // Unit attention parameters page (weird)
				scsi_cmdbuf[pos++] = 0x80; // PS, page id
				scsi_cmdbuf[pos++] = 0x02; // Page length
				scsi_cmdbuf[pos++] = 0x00; // Meh
				scsi_cmdbuf[pos++] = 0x00; // Double meh
				break;

			case 0x01: // read-write error recovery page
				scsi_cmdbuf[pos++] = 0x01; // !PS, page id
				scsi_cmdbuf[pos++] = 0x0a; // page length
				scsi_cmdbuf[pos++] = 0x26; // various bits
				scsi_cmdbuf[pos++] = 0; // read retry count
				scsi_cmdbuf[pos++] = 0; // correction span
				scsi_cmdbuf[pos++] = 0; // head offset count
				scsi_cmdbuf[pos++] = 0; // data strobe offset count
				scsi_cmdbuf[pos++] = 0; // reserved
				scsi_cmdbuf[pos++] = 0; // write retry count
				scsi_cmdbuf[pos++] = 0; // reserved
				scsi_cmdbuf[pos++] = 0; // recovery time limit (msb)
				scsi_cmdbuf[pos++] = 0; // recovery time limit (lsb)
				break;

			case 0x02: // disconnect-reconnect page
				scsi_cmdbuf[pos++] = 0x02; // !PS, page id
				scsi_cmdbuf[pos++] = 0x0e; // page length
				scsi_cmdbuf[pos++] = 0; // buffer full ratio
				scsi_cmdbuf[pos++] = 0; // buffer empty ratio
				scsi_cmdbuf[pos++] = 0; // bus inactivity limit (msb)
				scsi_cmdbuf[pos++] = 0; // bus inactivity limit (lsb)
				scsi_cmdbuf[pos++] = 0; // disconnect time limit (msb)
				scsi_cmdbuf[pos++] = 0; // disconnect time limit (lsb)
				scsi_cmdbuf[pos++] = 0; // connect time limit (msb)
				scsi_cmdbuf[pos++] = 0; // connect time limit (lsb)
				scsi_cmdbuf[pos++] = 0; // maximum burst size (msb)
				scsi_cmdbuf[pos++] = 0; // maximum burst size (lsb)
				scsi_cmdbuf[pos++] = 0; // reserved
				scsi_cmdbuf[pos++] = 0; // reserved
				scsi_cmdbuf[pos++] = 0; // reserved
				scsi_cmdbuf[pos++] = 0; // reserved
				break;

			case 0x03:  { // Format parameters page
				scsi_cmdbuf[pos++] = 0x83; // PS, page id
				scsi_cmdbuf[pos++] = 0x16; // Page length
				put_u16be(&scsi_cmdbuf[pos], info.cylinders * info.heads); // Track/zone
				pos += 2;
				scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				put_u16be(&scsi_cmdbuf[pos], info.sectors); // Sectors/track
				pos += 2;
				put_u16be(&scsi_cmdbuf[pos], info.sectorbytes); // Bytes/sector
				pos += 2;
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
				put_u24be(&scsi_cmdbuf[pos], info.cylinders); // Cylinders
				pos += 3;
				scsi_cmdbuf[pos++] = info.heads;              // Heads
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				scsi_cmdbuf[pos++] = 0x00;                    // Drive step rate
				scsi_cmdbuf[pos++] = 0x00;                    // Drive step rate
				scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				scsi_cmdbuf[pos++] = 0x00;                    // RPL
				scsi_cmdbuf[pos++] = 0x00;                    // Rotational offset
				scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				put_u16be(&scsi_cmdbuf[pos], 10000);          // Medium rotation rate
				pos += 2;
				scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				break;
			}

			case 0x08: // caching page
				scsi_cmdbuf[pos++] = 0x08; // !PS, page id
				scsi_cmdbuf[pos++] = 0x0a; // page length
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = 0;
				break;

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
				if (page != 0x3f) {
					LOG("mode sense page %02x unhandled\n", page);
					fail = true;
				}
				break;
			}
		}

		if (!fail) {
			scsi_cmdbuf[0] = pos - 1;
			if (pos > size)
				pos = size;

			scsi_data_in(0, pos);
			scsi_status_complete(SS_GOOD);
		} else {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;
	}

	case SC_START_STOP_UNIT:
		LOG("command %s UNIT\n", (scsi_cmdbuf[4] & 0x1) ? "START" : "STOP");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_RECEIVE_DIAGNOSTIC_RESULTS: {
		LOG("command RECEIVE DIAGNOSTIC RESULTS\n");
		int size = get_u16be(&scsi_cmdbuf[3]);
		int pos = 0;
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 6;
		scsi_cmdbuf[pos++] = 0; // ROM is OK
		scsi_cmdbuf[pos++] = 0; // RAM is OK
		scsi_cmdbuf[pos++] = 0; // Data buffer is OK
		scsi_cmdbuf[pos++] = 0; // Interface is OK
		scsi_cmdbuf[pos++] = 0;
		if(size > pos)
			size = pos;
		scsi_data_in(0, size);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_SEND_DIAGNOSTIC: {
		LOG("command SEND DIAGNOSTIC\n");
		int size = get_u16be(&scsi_cmdbuf[3]);
		if(scsi_cmdbuf[1] & 4) {
			// Self-test
			scsi_status_complete(SS_GOOD);
			break;
		}
		int pos = 0;
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 6;
		scsi_cmdbuf[pos++] = 0; // ROM is OK
		scsi_cmdbuf[pos++] = 0; // RAM is OK
		scsi_cmdbuf[pos++] = 0; // Data buffer is OK
		scsi_cmdbuf[pos++] = 0; // Interface is OK
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 0;
		if(size > pos)
			size = pos;
		scsi_data_in(0, size);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_SYNCHRONIZE_CACHE:
		LOG("command SYNCHRONIZE CACHE (10)\n");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_CAPACITY: {
		LOG("command READ CAPACITY\n");

		const auto &info = image->get_info();
		uint32_t size = info.cylinders * info.heads * info.sectors - 1;

		put_u32be(&scsi_cmdbuf[0], size);
		put_u32be(&scsi_cmdbuf[4], info.sectorbytes);

		scsi_data_in(0, 8);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_10:
		lba = get_u32be(&scsi_cmdbuf[2]);
		blocks = get_u16be(&scsi_cmdbuf[7]);

		LOG("command READ EXTENDED start=%08x blocks=%04x\n",lba, blocks);

		if(image->read(lba, block)) {
			scsi_data_in(2, blocks*bytes_per_sector);
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;

	case SC_WRITE_10:
		lba = get_u32be(&scsi_cmdbuf[2]);
		blocks = get_u16be(&scsi_cmdbuf[7]);

		LOG("command WRITE EXTENDED start=%08x blocks=%04x\n", lba, blocks);

		if(image->write(lba, block)) {
			scsi_data_out(2, blocks*bytes_per_sector);
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT:%s%s%s%s%s\n",
				(scsi_cmdbuf[1] & 0x80) ? " FMT-PINFO" : "",
				(scsi_cmdbuf[1] & 0x40) ? " RTO_REQ" : "",
				(scsi_cmdbuf[1] & 0x20) ? " LONG-LIST" : "",
				(scsi_cmdbuf[1] & 0x10) ? " FMTDATA" : "",
				(scsi_cmdbuf[1] & 0x08) ? " CMPLIST" : "");
		{
			const auto &info = image->get_info();
			auto block = std::make_unique<uint8_t[]>(info.sectorbytes);
			for(int cyl = 0; cyl < info.cylinders; cyl++) {
				for(int head = 0; head < info.heads; head++) {
					for(int sector = 0; sector < info.sectors; sector++) {
						image->write(cyl * head * sector, block.get());
					}
				}
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_MODE_SELECT_6:
		LOG("command MODE SELECT 6 length %d\n", scsi_cmdbuf[4]);

		// accept mode select parameter data
		if(scsi_cmdbuf[4])
			scsi_data_out(2, scsi_cmdbuf[4]);

		scsi_status_complete(SS_GOOD);
		break;

	case SC_VERIFY_10:
		LOG("command VERIFY BytChk %d\n", !!(scsi_cmdbuf[1] & 0x02));
		if (!(scsi_cmdbuf[1] & 0x02))
			scsi_status_complete(SS_GOOD);
		else
			scsi_unknown_command();
		break;

	default:
		LOGMASKED(LOG_UNSUPPORTED, "command %02x ***UNKNOWN***\n", scsi_cmdbuf[0]);
		// Parent may handle this
		nscsi_full_device::scsi_command();
		break;
	}
}
