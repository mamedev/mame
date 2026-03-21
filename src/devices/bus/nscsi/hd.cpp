// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "bus/nscsi/hd.h"
#include "imagedev/harddriv.h"

#include "multibyte.h"

#define LOG_COMMAND     (1U << 1)
#define LOG_DATA        (1U << 2)
#define LOG_UNSUPPORTED (1U << 3)

//#define VERBOSE 3

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
		m_scsi_id = -1;
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
	if(m_scsi_cmdbuf[0] != SC_READ_6) {
		LOGMASKED(LOG_COMMAND, "%02x %02x %02x %02x %02x %02x\n",
			m_scsi_cmdbuf[0], m_scsi_cmdbuf[1], m_scsi_cmdbuf[2],
			m_scsi_cmdbuf[3], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);
	}

	switch(m_scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		LOG("command TEST UNIT READY\n");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_6:
		lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
		blocks = m_scsi_cmdbuf[4];
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
		lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
		blocks = m_scsi_cmdbuf[4];
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
		int lun = get_lun(m_scsi_cmdbuf[1] >> 5);
		LOG("command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
			lun, m_scsi_cmdbuf[1] & 1, m_scsi_cmdbuf[2], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);

		int page = m_scsi_cmdbuf[2];
		int size = m_scsi_cmdbuf[4];
		switch(page) {
		case 0:
			std::fill_n(m_scsi_cmdbuf, 148, 0);

			// vendor and product information must be padded with spaces
			std::fill_n(&m_scsi_cmdbuf[8], 28, 0x20);

			// From Seagate SCSI Commands Reference Manual (http://www.seagate.com/staticfiles/support/disc/manuals/scsi/100293068a.pdf), page 73:
			// If the SCSI target device is not capable of supporting a peripheral device connected to this logical unit, the
			// device server shall set these fields to 7Fh (i.e., PERIPHERAL QUALIFIER field set to 011b and PERIPHERAL DEVICE
			// TYPE set to 1Fh).
			if (lun != 0)
				m_scsi_cmdbuf[0] = 0x7f;
			else
				m_scsi_cmdbuf[0] = 0x00; // device is direct-access (e.g. hard disk)
			m_scsi_cmdbuf[1] = 0x00; // media is not removable
			m_scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			m_scsi_cmdbuf[3] = 0x01; // response data format = CCS
			m_scsi_cmdbuf[4] = 52;   // additional length
			if(m_inquiry_data.empty()) {
				LOG("IDNT tag not found in chd metadata, using default inquiry data\n");

				// Apple HD SC setup utility needs to see this
				memset(m_scsi_cmdbuf+8, ' ', 28);
				int len = m_default_model_name.size();
				if(len > 28)
					len = 28;
				memcpy(m_scsi_cmdbuf+8, m_default_model_name.data(), len);
				m_scsi_cmdbuf[36] = 0x00; // # of extents high
				m_scsi_cmdbuf[37] = 0x08; // # of extents low
				m_scsi_cmdbuf[38] = 0x00; // group 0 commands 0-1f
				m_scsi_cmdbuf[39] = 0x99; // commands 0,3,4,7
				m_scsi_cmdbuf[40] = 0xa0; // commands 8, a
				m_scsi_cmdbuf[41] = 0x27; // commands 12,15,16,17
				m_scsi_cmdbuf[42] = 0x34; // commands 1a,1b,1d
				m_scsi_cmdbuf[43] = 0x01; // group 1 commands 20-3f
				m_scsi_cmdbuf[44] = 0x04;
				m_scsi_cmdbuf[45] = 0xa0;
				m_scsi_cmdbuf[46] = 0x01;
				m_scsi_cmdbuf[47] = 0x18;
				m_scsi_cmdbuf[48] = 0x07; // group 7 commands e0-ff
				m_scsi_cmdbuf[49] = 0x00;
				m_scsi_cmdbuf[50] = 0xa0; // commands 8, a
				m_scsi_cmdbuf[51] = 0x00;
				m_scsi_cmdbuf[52] = 0x00;
				m_scsi_cmdbuf[53] = 0xff; // end of list
			}
			else
				std::copy_n(m_inquiry_data.begin(), std::min(m_inquiry_data.size(), size_t(48)), &m_scsi_cmdbuf[8]);

			if(size > 56)
				size = 56;
			scsi_data_in(0, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_MODE_SENSE_6: {
		int lun = get_lun(m_scsi_cmdbuf[1] >> 5);
		LOG("command MODE SENSE 6 lun=%d page=%02x alloc=%02x link=%02x\n",
			lun, m_scsi_cmdbuf[2] & 0x3f, m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);
		if(lun) {
			bad_lun();
			return;
		}

		int page = m_scsi_cmdbuf[2] & 0x3f;
		int size = m_scsi_cmdbuf[4];
		int pos = 1;
		m_scsi_cmdbuf[pos++] = 0x00; // medium type
		m_scsi_cmdbuf[pos++] = 0x00; // WP, cache

		const auto &info = image->get_info();
		uint32_t dsize = info.cylinders * info.heads * info.sectors - 1;
		m_scsi_cmdbuf[pos++] = 0x08; // Block descriptor length
		m_scsi_cmdbuf[pos++] = 0x00;
		put_u24be(&m_scsi_cmdbuf[pos], dsize);
		pos += 3;
		m_scsi_cmdbuf[pos++] = 0x00;
		put_u24be(&m_scsi_cmdbuf[pos], info.sectorbytes);
		pos += 3;

		int pmax = page == 0x3f ? 0x3e : page;
		int pmin = page == 0x3f ? 0x00 : page;

		bool fail = false;
		for(int p=pmax; p >= pmin; p--) {
			switch(p) {
			case 0x00: // Unit attention parameters page (weird)
				m_scsi_cmdbuf[pos++] = 0x80; // PS, page id
				m_scsi_cmdbuf[pos++] = 0x02; // Page length
				m_scsi_cmdbuf[pos++] = 0x00; // Meh
				m_scsi_cmdbuf[pos++] = 0x00; // Double meh
				break;

			case 0x01: // read-write error recovery page
				m_scsi_cmdbuf[pos++] = 0x01; // !PS, page id
				m_scsi_cmdbuf[pos++] = 0x0a; // page length
				m_scsi_cmdbuf[pos++] = 0x26; // various bits
				m_scsi_cmdbuf[pos++] = 0; // read retry count
				m_scsi_cmdbuf[pos++] = 0; // correction span
				m_scsi_cmdbuf[pos++] = 0; // head offset count
				m_scsi_cmdbuf[pos++] = 0; // data strobe offset count
				m_scsi_cmdbuf[pos++] = 0; // reserved
				m_scsi_cmdbuf[pos++] = 0; // write retry count
				m_scsi_cmdbuf[pos++] = 0; // reserved
				m_scsi_cmdbuf[pos++] = 0; // recovery time limit (msb)
				m_scsi_cmdbuf[pos++] = 0; // recovery time limit (lsb)
				break;

			case 0x02: // disconnect-reconnect page
				m_scsi_cmdbuf[pos++] = 0x02; // !PS, page id
				m_scsi_cmdbuf[pos++] = 0x0e; // page length
				m_scsi_cmdbuf[pos++] = 0; // buffer full ratio
				m_scsi_cmdbuf[pos++] = 0; // buffer empty ratio
				m_scsi_cmdbuf[pos++] = 0; // bus inactivity limit (msb)
				m_scsi_cmdbuf[pos++] = 0; // bus inactivity limit (lsb)
				m_scsi_cmdbuf[pos++] = 0; // disconnect time limit (msb)
				m_scsi_cmdbuf[pos++] = 0; // disconnect time limit (lsb)
				m_scsi_cmdbuf[pos++] = 0; // connect time limit (msb)
				m_scsi_cmdbuf[pos++] = 0; // connect time limit (lsb)
				m_scsi_cmdbuf[pos++] = 0; // maximum burst size (msb)
				m_scsi_cmdbuf[pos++] = 0; // maximum burst size (lsb)
				m_scsi_cmdbuf[pos++] = 0; // reserved
				m_scsi_cmdbuf[pos++] = 0; // reserved
				m_scsi_cmdbuf[pos++] = 0; // reserved
				m_scsi_cmdbuf[pos++] = 0; // reserved
				break;

			case 0x03:  { // Format parameters page
				m_scsi_cmdbuf[pos++] = 0x83; // PS, page id
				m_scsi_cmdbuf[pos++] = 0x16; // Page length
				put_u16be(&m_scsi_cmdbuf[pos], info.cylinders * info.heads); // Track/zone
				pos += 2;
				m_scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				m_scsi_cmdbuf[pos++] = 0x00; // Alt sect/zone
				m_scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				m_scsi_cmdbuf[pos++] = 0x00; // Alt track/zone
				m_scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				m_scsi_cmdbuf[pos++] = 0x00; // Alt track/volume
				put_u16be(&m_scsi_cmdbuf[pos], info.sectors); // Sectors/track
				pos += 2;
				put_u16be(&m_scsi_cmdbuf[pos], info.sectorbytes); // Bytes/sector
				pos += 2;
				m_scsi_cmdbuf[pos++] = 0x00; // Interleave
				m_scsi_cmdbuf[pos++] = 0x00; // Interleave
				m_scsi_cmdbuf[pos++] = 0x00; // Track skew
				m_scsi_cmdbuf[pos++] = 0x00; // Track skew
				m_scsi_cmdbuf[pos++] = 0x00; // Cylinder skew
				m_scsi_cmdbuf[pos++] = 0x00; // Cylinder skew
				m_scsi_cmdbuf[pos++] = 0x00; // Sectoring type
				m_scsi_cmdbuf[pos++] = 0x00; // Reserved
				m_scsi_cmdbuf[pos++] = 0x00; // Reserved
				m_scsi_cmdbuf[pos++] = 0x00; // Reserved
				break;
			}

			case 0x04: { // Rigid drive geometry page
				m_scsi_cmdbuf[pos++] = 0x84; // PS, page id
				m_scsi_cmdbuf[pos++] = 0x16; // Page length
				put_u24be(&m_scsi_cmdbuf[pos], info.cylinders); // Cylinders
				pos += 3;
				m_scsi_cmdbuf[pos++] = info.heads;              // Heads
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - write precomp
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				m_scsi_cmdbuf[pos++] = 0x00;                    // Starting cylinder - reduced write current
				m_scsi_cmdbuf[pos++] = 0x00;                    // Drive step rate
				m_scsi_cmdbuf[pos++] = 0x00;                    // Drive step rate
				m_scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				m_scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				m_scsi_cmdbuf[pos++] = 0x00;                    // Landing zone cylinder
				m_scsi_cmdbuf[pos++] = 0x00;                    // RPL
				m_scsi_cmdbuf[pos++] = 0x00;                    // Rotational offset
				m_scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				put_u16be(&m_scsi_cmdbuf[pos], 10000);          // Medium rotation rate
				pos += 2;
				m_scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				m_scsi_cmdbuf[pos++] = 0x00;                    // Reserved
				break;
			}

			case 0x08: // caching page
				m_scsi_cmdbuf[pos++] = 0x08; // !PS, page id
				m_scsi_cmdbuf[pos++] = 0x0a; // page length
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				m_scsi_cmdbuf[pos++] = 0;
				break;

			case 0x30: { // Apple firmware ID page
				m_scsi_cmdbuf[pos++] = 0xb0; // cPS, page id
				m_scsi_cmdbuf[pos++] = 0x16; // Page length
				m_scsi_cmdbuf[pos++] = 'A';
				m_scsi_cmdbuf[pos++] = 'P';
				m_scsi_cmdbuf[pos++] = 'P';
				m_scsi_cmdbuf[pos++] = 'L';
				m_scsi_cmdbuf[pos++] = 'E';
				m_scsi_cmdbuf[pos++] = ' ';
				m_scsi_cmdbuf[pos++] = 'C';
				m_scsi_cmdbuf[pos++] = 'O';
				m_scsi_cmdbuf[pos++] = 'M';
				m_scsi_cmdbuf[pos++] = 'P';
				m_scsi_cmdbuf[pos++] = 'U';
				m_scsi_cmdbuf[pos++] = 'T';
				m_scsi_cmdbuf[pos++] = 'E';
				m_scsi_cmdbuf[pos++] = 'R';
				m_scsi_cmdbuf[pos++] = ',';
				m_scsi_cmdbuf[pos++] = ' ';
				m_scsi_cmdbuf[pos++] = 'I';
				m_scsi_cmdbuf[pos++] = 'N';
				m_scsi_cmdbuf[pos++] = 'C';
				m_scsi_cmdbuf[pos++] = ' ';
				m_scsi_cmdbuf[pos++] = ' ';
				m_scsi_cmdbuf[pos++] = ' ';
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
			m_scsi_cmdbuf[0] = pos - 1;
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
		LOG("command %s UNIT\n", (m_scsi_cmdbuf[4] & 0x1) ? "START" : "STOP");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_RECEIVE_DIAGNOSTIC_RESULTS: {
		LOG("command RECEIVE DIAGNOSTIC RESULTS\n");
		int size = get_u16be(&m_scsi_cmdbuf[3]);
		int pos = 0;
		m_scsi_cmdbuf[pos++] = 0;
		m_scsi_cmdbuf[pos++] = 6;
		m_scsi_cmdbuf[pos++] = 0; // ROM is OK
		m_scsi_cmdbuf[pos++] = 0; // RAM is OK
		m_scsi_cmdbuf[pos++] = 0; // Data buffer is OK
		m_scsi_cmdbuf[pos++] = 0; // Interface is OK
		m_scsi_cmdbuf[pos++] = 0;
		if(size > pos)
			size = pos;
		scsi_data_in(0, size);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_SEND_DIAGNOSTIC: {
		LOG("command SEND DIAGNOSTIC\n");
		int size = get_u16be(&m_scsi_cmdbuf[3]);
		if(m_scsi_cmdbuf[1] & 4) {
			// Self-test
			scsi_status_complete(SS_GOOD);
			break;
		}
		int pos = 0;
		m_scsi_cmdbuf[pos++] = 0;
		m_scsi_cmdbuf[pos++] = 6;
		m_scsi_cmdbuf[pos++] = 0; // ROM is OK
		m_scsi_cmdbuf[pos++] = 0; // RAM is OK
		m_scsi_cmdbuf[pos++] = 0; // Data buffer is OK
		m_scsi_cmdbuf[pos++] = 0; // Interface is OK
		m_scsi_cmdbuf[pos++] = 0;
		m_scsi_cmdbuf[pos++] = 0;
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

		put_u32be(&m_scsi_cmdbuf[0], size);
		put_u32be(&m_scsi_cmdbuf[4], info.sectorbytes);

		scsi_data_in(0, 8);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_10:
		lba = get_u32be(&m_scsi_cmdbuf[2]);
		blocks = get_u16be(&m_scsi_cmdbuf[7]);

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
		lba = get_u32be(&m_scsi_cmdbuf[2]);
		blocks = get_u16be(&m_scsi_cmdbuf[7]);

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
				(m_scsi_cmdbuf[1] & 0x80) ? " FMT-PINFO" : "",
				(m_scsi_cmdbuf[1] & 0x40) ? " RTO_REQ" : "",
				(m_scsi_cmdbuf[1] & 0x20) ? " LONG-LIST" : "",
				(m_scsi_cmdbuf[1] & 0x10) ? " FMTDATA" : "",
				(m_scsi_cmdbuf[1] & 0x08) ? " CMPLIST" : "");
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
		LOG("command MODE SELECT 6 length %d\n", m_scsi_cmdbuf[4]);

		// accept mode select parameter data
		if(m_scsi_cmdbuf[4])
			scsi_data_out(2, m_scsi_cmdbuf[4]);

		scsi_status_complete(SS_GOOD);
		break;

	case SC_VERIFY_10:
		LOG("command VERIFY BytChk %d\n", !!(m_scsi_cmdbuf[1] & 0x02));
		if (!(m_scsi_cmdbuf[1] & 0x02))
			scsi_status_complete(SS_GOOD);
		else
			scsi_unknown_command();
		break;

	default:
		LOGMASKED(LOG_UNSUPPORTED, "command %02x ***UNKNOWN***\n", m_scsi_cmdbuf[0]);
		// Parent may handle this
		nscsi_full_device::scsi_command();
		break;
	}
}
