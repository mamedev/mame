// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "emuopts.h"

#include "bus/nscsi/cd.h"

#include "coreutil.h"
#include "multibyte.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device, "scsi_cdrom", "SCSI CD-ROM")
DEFINE_DEVICE_TYPE(NSCSI_CDROM_SGI, nscsi_cdrom_sgi_device, "scsi_cdrom_sgi", "SCSI CD-ROM SGI")
DEFINE_DEVICE_TYPE(NSCSI_CDROM_NEWS, nscsi_cdrom_news_device, "scsi_cdrom_news", "SCSI CD-ROM NEWS")
DEFINE_DEVICE_TYPE(NSCSI_RRD45, nscsi_dec_rrd45_device, "nrrd45", "RRD45 CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM3301, nscsi_toshiba_xm3301_device, "nxm3301", "XM-3301TA CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5301SUN, nscsi_toshiba_xm5301_sun_device, "nxm5301sun", "XM-5301B Sun 4x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5401, nscsi_toshiba_xm5401_device, "nxm5401", "XM-5401B 4x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5401SUN, nscsi_toshiba_xm5401_sun_device, "nxm5401sun", "XM-5401B Sun 4x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5701, nscsi_toshiba_xm5701_device, "nxm5701", "XM-5701B 12x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5701SUN, nscsi_toshiba_xm5701_sun_device, "nxm5701sun", "XM-5701B Sun 12x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_CDROM_APPLE, nscsi_cdrom_apple_device, "scsi_cdrom_apple", "Apple SCSI CD-ROM")

// ZuluSCSI/BlueSCSI toolbox commands.  All are 10 bytes as of protocol version 0.
static constexpr uint8_t TOOLBOX_LIST_FILES     = 0xd0;
static constexpr uint8_t TOOLBOX_GET_FILE       = 0xd1;
static constexpr uint8_t TOOLBOX_COUNT_FILES    = 0xd2;
static constexpr uint8_t TOOLBOX_SEND_FILE_PREP = 0xd3;
static constexpr uint8_t TOOLBOX_SEND_FILE_10   = 0xd4;
static constexpr uint8_t TOOLBOX_SEND_FILE_END  = 0xd5;
static constexpr uint8_t TOOLBOX_TOGGLE_DEBUG   = 0xd6;
static constexpr uint8_t TOOLBOX_LIST_CDS       = 0xd7;
static constexpr uint8_t TOOLBOX_SET_NEXT_CD    = 0xd8;
static constexpr uint8_t TOOLBOX_LIST_DEVICES   = 0xd9;
static constexpr uint8_t TOOLBOX_COUNT_CDS      = 0xda;

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
{
}

nscsi_cdrom_sgi_device::nscsi_cdrom_sgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_SGI, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
{
}

nscsi_cdrom_news_device::nscsi_cdrom_news_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_NEWS, tag, owner, "Sony", "CD-ROM CDU-541", "1.0A", 0x00, 0x05)
{
}

nscsi_dec_rrd45_device::nscsi_dec_rrd45_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_RRD45, tag, owner, "DEC     ", "RRD45   (C) DEC ", "0436", 0x98, 0x02)
{
}

nscsi_toshiba_xm3301_device::nscsi_toshiba_xm3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM3301, tag, owner, "TOSHIBA ", "CD-ROM XM-3301TA", "0272", 0x88, 0x02)
{
}

nscsi_toshiba_xm5301_sun_device::nscsi_toshiba_xm5301_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM5301SUN, tag, owner, "TOSHIBA ", "XM-5301TASUN4XCD", "2915", 0x98, 0x02)
{
}

nscsi_toshiba_xm5401_device::nscsi_toshiba_xm5401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM5401, tag, owner, "TOSHIBA ", "CD-ROM XM-5401TA", "3605", 0x98, 0x02)
{
}

nscsi_toshiba_xm5401_sun_device::nscsi_toshiba_xm5401_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM5401SUN, tag, owner, "TOSHIBA ", "XM-5401TASUN4XCD", "1036", 0x98, 0x02)
{
}

nscsi_toshiba_xm5701_device::nscsi_toshiba_xm5701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM5701, tag, owner, "TOSHIBA ", "CD-ROM XM-5701TA", "3136", 0x98, 0x02)
{
}

nscsi_toshiba_xm5701_sun_device::nscsi_toshiba_xm5701_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_XM5701SUN, tag, owner, "TOSHIBA ", "XM5701TASUN12XCD", "0997", 0x98, 0x02)
{
}

// drive identifies as an original Apple CDSC (Sony CDU-8001 with custom firmware)
nscsi_cdrom_apple_device::nscsi_cdrom_apple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_APPLE, tag, owner, "SONY    ", "CD-ROM CDU-8002 ", "1.8g", 0x00, 0x05),
	m_stopped(true),
	m_stop_position(0)
{
}

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_full_device(mconfig, type, tag, owner, clock)
	, image(*this, "image")
	, cdda(*this, "cdda")
	, m_removal_prevented(false)
	, bytes_per_block(bytes_per_sector)
	, lba(0)
	, cur_sector(0)
{
}

void nscsi_cdrom_device::device_start()
{
	nscsi_full_device::device_start();

	m_xfer_buffer.resize(8192);

	save_item(NAME(sector_buffer));
	save_item(NAME(lba));
	save_item(NAME(cur_sector));
	save_item(NAME(bytes_per_block));
	save_item(NAME(mode_data_size));
	save_item(NAME(m_xfer_position));
	save_item(NAME(m_write_length));
	save_item(NAME(m_write_offset));
	save_item(NAME(m_write_is_setup));
}

void nscsi_cdrom_device::device_reset()
{
	nscsi_full_device::device_reset();
	sequence_counter = image->sequence_counter();
	lba = 0;
	cur_sector = -1;
	mode_data_size = 12;
}

void nscsi_cdrom_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, image).set_interface("cdrom");

	CDDA(config, cdda).set_cdrom_tag(image);
}

int nscsi_cdrom_device::to_msf(int frame)
{
	int m = frame / (75 * 60);
	int s = (frame / 75) % 60;
	int f = frame % 75;

	return (m << 16) | (s << 8) | f;
}

void nscsi_cdrom_device::set_block_size(u32 block_size)
{
	if (bytes_per_sector % block_size)
	{
		logerror("nscsi_cdrom_device(%s): block size %d must be a factor of sector size %d", tag(), block_size, bytes_per_sector);
		return;
	}

	bytes_per_block = block_size;
};

bool nscsi_cdrom_device::scsi_command_done(uint8_t command, uint8_t length)
{
	switch (command & 0xd0)
	{
	case 0xd0:  // ZuluSCSI/BlueSCSI toolbox commands
		return length == 10;

	default:
		return nscsi_full_device::scsi_command_done(command, length);
	}
}

uint8_t nscsi_cdrom_device::scsi_get_data(int id, int pos)
{
	if (id == 4)
		return m_xfer_buffer[m_xfer_position++];

	if(id != 2)
		return nscsi_full_device::scsi_get_data(id, pos);

	const int sector = (lba * bytes_per_block + pos) / bytes_per_sector;
	const int extra_pos = (lba * bytes_per_block) % bytes_per_sector;
	if(sector != cur_sector) {
		cur_sector = sector;
		if(!image->read_data(sector, sector_buffer, cdrom_file::CD_TRACK_MODE1)) {
			LOG("CD READ ERROR sector %d!\n", sector);
			std::fill_n(sector_buffer, sizeof(sector_buffer), 0);
		}
	}
	return sector_buffer[(pos + extra_pos) & (bytes_per_sector - 1)];
}

void nscsi_cdrom_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if (id == 4) {
		m_xfer_buffer[m_xfer_position] = data;
		m_xfer_position++;
		if (m_write_is_setup) {
			if (m_xfer_position == 33) {
				osd_file::ptr file;
				uint64_t filesize;
				m_write_path.clear();
				m_write_path = machine().options().share_directory();
				m_write_path.append(PATH_SEPARATOR);
				m_write_path.append((char *)&m_xfer_buffer[0]);
				if (osd_file::open(m_write_path, OPEN_FLAG_CREATE | OPEN_FLAG_WRITE, file, filesize)) {
					LOG("Open for write/creation failed for [%s]\n", m_write_path.c_str());
					scsi_status_complete(SS_CHECK_CONDITION);
					sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
					return;
				}
			}
		}
		else
		{
			if (m_xfer_position == m_write_length) {
				uint32_t actualWritten;
				osd_file::ptr file;
				uint64_t filesize;

				LOG("flushing [%s] to disk at %08x\n", m_write_path.c_str(), m_write_offset * 512);
				osd_file::open(m_write_path, OPEN_FLAG_WRITE, file, filesize);
				file->write(&m_xfer_buffer[0], m_write_offset * 512, m_write_length, actualWritten);

				if (actualWritten != m_write_length) {
					scsi_status_complete(SS_CHECK_CONDITION);
					sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
				}
			}
		}
		return;
	}

	if(id != 2) {
		nscsi_full_device::scsi_put_data(id, pos, data);
		return;
	}

	// process mode parameter header and one block descriptor
	if(pos < mode_data_size) {
		mode_data[pos] = data;

		// is this the last byte of the mode parameter block descriptor?
		if(pos == mode_data_size - 1) {
			// is there exactly one block descriptor?
			if(mode_data[3] == 8)
				set_block_size(get_u24be(&mode_data[9]));

			// Size of block descriptor should change based on if it's mode select (6) or (10)
			// TODO: Find a usage of mode select (10) for testing
			int pagedata_offset = 4 + mode_data[3];
			switch(mode_data[pagedata_offset] & 0x3f) {
				case 0x0e: // CD Audio Control Mode Page
					cdda_sotc = BIT(mode_data[pagedata_offset+2], 1);
					LOG("Ch 0 route: %x vol: %x\n", mode_data[pagedata_offset+8], mode_data[pagedata_offset+9]);
					LOG("Ch 1 route: %x vol: %x\n", mode_data[pagedata_offset+10], mode_data[pagedata_offset+11]);
					LOG("Ch 2 route: %x vol: %x\n", mode_data[pagedata_offset+12], mode_data[pagedata_offset+13]);
					LOG("Ch 3 route: %x vol: %x\n", mode_data[pagedata_offset+14], mode_data[pagedata_offset+15]);
					cdda->set_output_gain(0, mode_data[pagedata_offset+9] / 255.0f);
					cdda->set_output_gain(1, mode_data[pagedata_offset+11] / 255.0f);
					break;
			}
		}
	}
}

void nscsi_cdrom_device::update_directory()
{
	std::string tmpPath(machine().options().share_directory());
	osd::directory::ptr directory = osd::directory::open(tmpPath);

	m_directory.clear();

	if (directory != nullptr)
	{
		const osd::directory::entry *ourEntry;

		while ((ourEntry = directory->read()) != nullptr)
		{
			m_directory.push_back(*ourEntry);

			// API version 0 has a hard cap of 100 files
			if (m_directory.size() >= 99)
			{
				break;
			}
		}

		directory.reset();
	}
}

void nscsi_cdrom_device::return_no_cd()
{
	sense(false, SK_NOT_READY, SK_ASC_MEDIUM_NOT_PRESENT);
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_cdrom_device::scsi_command()
{
	int blocks;

	// check for media change
	if( sequence_counter != image->sequence_counter() && (scsi_cmdbuf[0] != SC_INQUIRY))
	{
		// clear media change condition
		cur_sector = -1;
		sequence_counter = image->sequence_counter();

		// report unit attention condition
		sense(false, SK_UNIT_ATTENTION);
		scsi_status_complete(SS_CHECK_CONDITION);
		return;
	}

	switch(scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		LOG("command TEST UNIT READY\n");
		if(image->exists())
			scsi_status_complete(SS_GOOD);
		else
			return_no_cd();
		break;

	case SC_READ_6:
		if(!image->exists()) {
			return_no_cd();
			break;
		}

		lba = get_u24be(&scsi_cmdbuf[1]) & 0x1fffff;
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

		/*
		 * 7.5.3 Selection of an invalid logical unit
		 *
		 * The logical unit may not be valid because:
		 * a) the target does not support the logical unit (e.g. some targets
		 *    support only one peripheral device). In response to an INQUIRY
		 *    command, the target shall return the INQUIRY data with the
		 *    peripheral qualifier set to the value required in 8.2.5.1.
		 *
		 * If the logic from the specification above is applied, Sun SCSI probe
		 * code gets confused and reports multiple valid logical units are
		 * attached; proper behaviour is produced when check condition status
		 * is returned with sense data ILLEGAL REQUEST and LOGICAL UNIT NOT
		 * SUPPORTED.
		 */
		int page = scsi_cmdbuf[2];
		int size = scsi_cmdbuf[4];
		switch(page) {
		case 0:
			std::fill_n(scsi_cmdbuf, size, 0);

			if (lun != 0)
				scsi_cmdbuf[0] = 0x7f;
			else
				scsi_cmdbuf[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
			scsi_cmdbuf[1] = 0x80; // media is removable
			scsi_cmdbuf[2] = compliance; // device complies with SPC-3 standard
			scsi_cmdbuf[3] = 0x02; // response data format = SPC-3 standard
			scsi_cmdbuf[4] = 0x20; // additional length
			scsi_cmdbuf[7] = inquiry_data;
			strncpy((char *)&scsi_cmdbuf[8], manufacturer, 8);
			strncpy((char *)&scsi_cmdbuf[16], product, 16);
			strncpy((char *)&scsi_cmdbuf[32], revision, 4);

			// vendor and product information must be padded with spaces
			for(int i = 8; i < 36; i++)
				if(scsi_cmdbuf[i] == 0)
					scsi_cmdbuf[i] = 0x20;

			scsi_data_in(SBUF_MAIN, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_MODE_SELECT_6:
		LOG("command MODE SELECT 6 length %d\n", scsi_cmdbuf[4]);

		// accept mode select parameter data
		mode_data_size = scsi_cmdbuf[4];
		if(mode_data_size)
			scsi_data_out(2, mode_data_size);

		scsi_status_complete(SS_GOOD);
		break;

	case SC_START_STOP_UNIT:
		LOG("command %s UNIT%s\n", (scsi_cmdbuf[4] & 0x1) ? "START" : "STOP",
			(scsi_cmdbuf[4] & 0x2) ? (scsi_cmdbuf[4] & 0x1) ? " (LOAD)" : " (EJECT)" : "");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_RECEIVE_DIAGNOSTIC_RESULTS: {
		LOG("command RECEIVE DIAGNOSTIC RESULTS");
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
		LOG("command SEND DIAGNOSTIC");
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

	case SC_READ_CAPACITY: {
		LOG("command READ CAPACITY\n");

		if(!image->exists()) {
			return_no_cd();
			break;
		}

		// get the last used block on the disc
		const u32 temp = image->get_track_start(0xaa) * (bytes_per_sector / bytes_per_block) - 1;

		put_u32be(&scsi_cmdbuf[0], temp);
		scsi_cmdbuf[4] = 0;
		scsi_cmdbuf[5] = 0;
		put_u16be(&scsi_cmdbuf[6], bytes_per_block);

		scsi_data_in(SBUF_MAIN, 8);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_10:
		lba = get_u32be(&scsi_cmdbuf[2]);
		blocks = get_u16be(&scsi_cmdbuf[7]);

		LOG("command READ EXTENDED start=%08x blocks=%04x\n", lba, blocks);
		if(!image->exists()) {
			return_no_cd();
			break;
		}

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

		const int page = scsi_cmdbuf[2] & 0x3f;
		const int size = scsi_cmdbuf[4];
		const bool noBlockDesc = BIT(scsi_cmdbuf[1], 3);
		int pos = 1;
		scsi_cmdbuf[pos++] = 0x00; // medium type
		scsi_cmdbuf[pos++] = 0x80; // WP, cache

		// get the last used block on the disc
		const u32 temp = image->exists() ? image->get_track_start(0xaa) * (bytes_per_sector / bytes_per_block) - 1 : 0;
		scsi_cmdbuf[pos++] = noBlockDesc ? 0x00 : 0x08; // 0 or 8 bytes of block descriptor

		if (!noBlockDesc)
		{
			scsi_cmdbuf[pos++] = 0x00; // density code
			put_u24be(&scsi_cmdbuf[pos], temp);
			pos += 3;
			scsi_cmdbuf[pos++] = 0;
			scsi_cmdbuf[pos++] = 0;
			put_u16be(&scsi_cmdbuf[pos], bytes_per_block);
			pos += 2;
		}

		bool fail = false;
		int pmax = page == 0x3f ? 0x3e : page;
		int pmin = page == 0x3f ? 0x00 : page;
		for(int p=pmax; p >= pmin; p--) {
			switch(p) {
			case 0x00: // Vendor specific (does not require page format)
				scsi_cmdbuf[pos++] = 0x80; // PS, page id
				scsi_cmdbuf[pos++] = 0x02; // Page length
				scsi_cmdbuf[pos++] = 0x00; // Meh
				scsi_cmdbuf[pos++] = 0x00; // Double meh
				break;

			case 0x01: // Read-write error recovery
				scsi_cmdbuf[pos++] = 0x01; // PS, page id
				scsi_cmdbuf[pos++] = 0x0a; // Page length
				scsi_cmdbuf[pos++] = 0x01;
				scsi_cmdbuf[pos++] = 0x01;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
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

			case 0x0e: // CD Audio control page
				scsi_cmdbuf[pos++] = 0x8e; // PS, page id
				scsi_cmdbuf[pos++] = 0x0e; // Page length
				scsi_cmdbuf[pos++] = (1 << 2) | (cdda_sotc << 1); // IMMED = 1
				scsi_cmdbuf[pos++] = 0x00; // Reserved
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x00;
				scsi_cmdbuf[pos++] = 0x01; // Ch 1 connected
				scsi_cmdbuf[pos++] = 0xff; // Ch 1 max volume
				scsi_cmdbuf[pos++] = 0x02; // Ch 2 connected
				scsi_cmdbuf[pos++] = 0xff; // Ch 2 max volume
				scsi_cmdbuf[pos++] = 0x04; // Ch 3 connected
				scsi_cmdbuf[pos++] = 0xff; // Ch 3 max volume
				scsi_cmdbuf[pos++] = 0x08; // Ch 4 connected
				scsi_cmdbuf[pos++] = 0xff; // Ch 4 max volume
				break;

			case 0x30: // magic Apple page
				{
					static const u8 apple_magic[0x17] =
						{
							0x00, 0x41, 0x50, 0x50, 0x4C, 0x45, 0x20, 0x43, 0x4F, 0x4D, 0x50, 0x55,
							0x54, 0x45, 0x52, 0x2C, 0x20, 0x49, 0x4E, 0x43, 0x20, 0x20, 0x20
						};

					LOG("Apple special MODE SENSE page\n");
					scsi_cmdbuf[pos++] = 0x30; // PS, page id
					memcpy(&scsi_cmdbuf[pos], apple_magic, 0x17);
					pos += 0x17;
				}
				break;

			case 0x31: // magic BlueSCSI page; official clients require this to recognize a toolbox-capable device
				{
					// Yes this is lame for a GPLv3 project.
					static const u8 bluescsi_magic[] =
					{
						"BlueSCSI is the BEST STOLEN FROM BLUESCSI\0"
					};

					scsi_cmdbuf[pos++] = 0x31; // page ID
					scsi_cmdbuf[pos++] = 42;   // page length
					memcpy(&scsi_cmdbuf[pos], bluescsi_magic, 42);
					pos += 42;
				}
				break;

				default:
					if (page != 0x3f)
					{
						LOG("mode sense page %02x unhandled\n", p);
						fail = true;
					}
					break;
			}
		}
		scsi_cmdbuf[0] = pos - 1;   // return -1 for mode sense 6

		if (!fail) {
			if (size < pos)
				scsi_data_in(0, size);
			else
				scsi_data_in(0, pos);

			scsi_status_complete(SS_GOOD);
		} else {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;
	}

	case SC_READ_DISC_INFORMATION:
		LOG("command READ DISC INFORMATION\n");
		if(!image->exists()) {
			return_no_cd();
			break;
		}

		std::fill_n(scsi_cmdbuf, 34, 0);
		scsi_cmdbuf[1] = 32;
		scsi_cmdbuf[2] = 2; // disc is complete
		scsi_cmdbuf[3] = 1; // first track
		scsi_cmdbuf[4] = 1; // number of sessions (TODO: session support for CHDv6)
		scsi_cmdbuf[5] = 1; // first track in last session
		scsi_cmdbuf[6] = image->get_last_track();   // last track in last session
		scsi_cmdbuf[8] = 0; // CD-ROM, not XA

		// lead in start time in MSF
		{
			u32 tstart = image->get_track_start(0);
			tstart = to_msf(tstart + 150);

			put_u32be(&scsi_cmdbuf[16], tstart);

			// lead-out start time in MSF
			tstart = image->get_track_start(0xaa);
			tstart = to_msf(tstart + 150);

			put_u32be(&scsi_cmdbuf[20], tstart);
		}

		scsi_data_in(0, 34);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
		// TODO: support eject prevention
		LOG("command %s MEDIUM REMOVAL\n", (scsi_cmdbuf[4] & 0x1) ? "PREVENT" : "ALLOW");
		m_removal_prevented = BIT(scsi_cmdbuf[4], 0);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_SUB_CHANNEL: {
		const bool msf = BIT(scsi_cmdbuf[1], 1) != 0;
		const bool subq = BIT(scsi_cmdbuf[2], 6);
		const int param = scsi_cmdbuf[3];
		const int track = scsi_cmdbuf[6];
		const int alloc_length = get_u16be(&scsi_cmdbuf[7]);

		LOG("command READ SUB CHANNEL Param = %d, Track = %d, MSF = %d, SUBQ = %d\n", param, track, msf, subq);

		if(!image->exists()) {
			return_no_cd();
			break;
		}

		std::fill_n(scsi_cmdbuf, alloc_length, 0);

		scsi_cmdbuf[0] = 0x00; // Reserved

		int audio_active = cdda->audio_active();
		if(audio_active) {
			if(cdda->audio_paused())
				scsi_cmdbuf[1] = 0x12; // Audio play operation paused
			else
				scsi_cmdbuf[1] = 0x11; // Audio play operation in progress
		} else {
			if(cdda->audio_ended())
				scsi_cmdbuf[1] = 0x13; // Audio play operation successfully completed
			else
				scsi_cmdbuf[1] = 0x15; // No current audio status to return
		}

		if(subq) {
			switch(param) {
				case 1: { //  CD current position
					int m_last_lba = audio_active ? cdda->get_audio_lba() : 0;

					scsi_cmdbuf[2] = 0; // Sub-channel Data Length
					scsi_cmdbuf[3] = 12;

					scsi_cmdbuf[4] = 0x01; // Sub-channel Data Format Code
					scsi_cmdbuf[5] = (audio_active ? 0 : 4) | 0x10; // Q Sub-channel encodes current position data
					scsi_cmdbuf[6] = image->get_track(m_last_lba) + 1; // Track Number
					scsi_cmdbuf[7] = 0x00; // Index Number

					uint32_t frame = m_last_lba;
					if(msf)
						frame = to_msf(frame);

					put_u32be(&scsi_cmdbuf[8], frame); // Absolute CD Address

					frame = m_last_lba - image->get_track_start(scsi_cmdbuf[6] - 1);
					if(msf)
						frame = to_msf(frame);

					put_u32be(&scsi_cmdbuf[12], frame); // Track Relative CD Address
					break;
				}

				case 0x02: // Media Catalog number (UPC/bar code)
				case 0x03: // Track International standard recording code (ISRC)
				default:
					LOG("Unknown subchannel type %d requested\n", scsi_cmdbuf[3]);
					break;
			}
		}

		scsi_data_in(0, alloc_length);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_READ_TOC_PMA_ATIP: {
		/*
		    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
		    That makes sense in terms of how real-world CDs are referred to, but
		    our internal routines for tracks use "0" as track 1.  That probably
		    should be fixed...
		*/
		static const char *const format_names[16] = {
			"TOC",
			"Session info",
			"Full TOC",
			"PMA",
			"ATIP",
			"Reserved 5",
			"Reserved 6",
			"Reserved 7",
			"Reserved 8",
			"Reserved 9",
			"Reserved 10",
			"Reserved 11",
			"Reserved 12",
			"Reserved 13",
			"Reserved 14",
			"Reserved 15"
		};

		bool msf = (scsi_cmdbuf[1] & 0x2) != 0;
		u16 size = get_u16be(&scsi_cmdbuf[7]);
		u8 format = scsi_cmdbuf[2] & 15;

		/// SFF8020 legacy format field (see T10/1836-D Revision 2g page 643)
		if(!format)
			format = (scsi_cmdbuf[9] >> 6) & 3;

		LOG("command READ TOC PMA ATIP, format %s msf=%d size=%d\n", format_names[format], msf, size);

		if(!image->exists()) {
			return_no_cd();
			break;
		}

		int pos = 0;
		switch (format) {
		case 0: {
			int start_track = scsi_cmdbuf[6];
			int end_track = image->get_last_track();

			int tracks;
			if(start_track == 0)
				tracks = end_track + 1;
			else if(start_track <= end_track)
				tracks = (end_track - start_track) + 2;
			else if(start_track <= 0xaa)
				tracks = 1;
			else
				tracks = 0;

			int len = 2 + (tracks * 8);

			// the returned TOC DATA LENGTH must be the full amount,
			// regardless of how much we're able to pass back due to size
			put_u16be(&scsi_cmdbuf[pos], len);
			pos += 2;
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = image->get_last_track();

			if (start_track == 0)
				start_track = 1;

			for(int i = 0; i < tracks; i++) {
				int track = start_track + i;
				int cdrom_track = track - 1;
				if(i == tracks-1) {
					track = 0xaa;
					cdrom_track = 0xaa;
				}

				scsi_cmdbuf[pos++] = 0;
				scsi_cmdbuf[pos++] = image->get_adr_control(cdrom_track);
				scsi_cmdbuf[pos++] = track;
				scsi_cmdbuf[pos++] = 0;

				u32 tstart = image->get_track_start(cdrom_track);

				if(msf)
					tstart = to_msf(tstart+150);

				put_u32be(&scsi_cmdbuf[pos], tstart);
				pos += 4;
			}
			break;
		}

		case 1: {
			int len = 2 + (8 * 1);

			put_u16be(&scsi_cmdbuf[pos], len);
			pos += 2;
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = 1;

			scsi_cmdbuf[pos++] = 0;
			scsi_cmdbuf[pos++] = image->get_adr_control(0);
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = 0;

			u32 tstart = image->get_track_start(0);

			if (msf)
				tstart = to_msf(tstart+150);

			put_u32be(&scsi_cmdbuf[pos], tstart);
			pos += 4;
			break;
		}

		default:
			LOG("Unhandled format %d\n", format_names[format]);
			break;
		}

		if(pos) {
			if(pos > size)
				pos = size;

			scsi_data_in(0, pos);
			scsi_status_complete(SS_GOOD);
		} else {
			// report unit attention condition
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST);
			break;
		}
		break;
	}

	case SC_PLAY_AUDIO_TRACK_INDEX: {
		LOG("command PLAY AUDIO TRACK INDEX tr %d/idx %d to tr %d/idx %d\n", scsi_cmdbuf[4], scsi_cmdbuf[5], scsi_cmdbuf[7], scsi_cmdbuf[8]);

		if(scsi_cmdbuf[4] > scsi_cmdbuf[7]) {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, 0x00, 0x14);
			break;
		}

		// TODO: Index isn't accounted for at all
		const uint32_t start_track = scsi_cmdbuf[4];
		const uint32_t end_track = cdda_sotc ? start_track : std::min(image->get_last_track(), (int)scsi_cmdbuf[7]);
		const uint32_t lba = image->get_track_start(start_track - 1);
		const uint32_t blocks = image->get_track_start(end_track) - lba;
		const uint32_t track = image->get_track(lba);

		if(image->get_track_type(track) == cdrom_file::CD_TRACK_AUDIO) {
			LOG("Playing %d blocks from track %d (lba %d)\n", blocks, start_track, lba);

			cdda->start_audio(lba, blocks);
			scsi_status_complete(SS_GOOD);
			sense(false, SK_NO_SENSE, 0x00, 0x11);
		} else {
			LOG("Track %d is not audio\n", start_track);

			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, 0x64, 0x00);
		}
		break;
	}

	case SC_PAUSE_RESUME:
		if (image->exists())
			cdda->pause_audio(BIT(scsi_cmdbuf[8], 0) ^ 1);

		scsi_status_complete(SS_GOOD);
		break;

	case TOOLBOX_COUNT_FILES:
		LOG("TOOLBOX_COUNT_FILES\n");

		update_directory();

		scsi_cmdbuf[0] = m_directory.size();
		scsi_data_in(0, 1);
		scsi_status_complete(SS_GOOD);
		break;

	case TOOLBOX_LIST_FILES:
	{
		LOG("TOOLBOX_LIST_FILES\n");

		// it's assumed clients will always do COUNT_FILES before LIST_FILES, because otherwise
		// they'll have no idea how much data is coming.
		int pos = 0;
		int index = 0;

		if (m_directory.size() > 0)
		{
			while (index < m_directory.size())
			{
				scsi_cmdbuf[pos] = index;
				scsi_cmdbuf[pos + 1] = m_directory[index].type != osd::directory::entry::entry_type::DIR;
				// There's a guaranteed null terminator one byte after the name field
				strncpy(reinterpret_cast<char *>(&scsi_cmdbuf[pos + 2]), m_directory[index].name, 31);
				scsi_cmdbuf[pos + 36] = (m_directory[index].size >> 24) & 0xff;
				scsi_cmdbuf[pos + 37] = (m_directory[index].size >> 16) & 0xff;
				scsi_cmdbuf[pos + 38] = (m_directory[index].size >> 8) & 0xff;
				scsi_cmdbuf[pos + 39] = m_directory[index].size & 0xff;

				LOG("%02d: %s %08x\n", index, m_directory[index].name, (uint32_t)m_directory[index].size);

				index++;
				pos += 40;
			}
		}

		scsi_data_in(0, pos);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case TOOLBOX_GET_FILE:
	{
		update_directory();

		if (scsi_cmdbuf[1] >= m_directory.size())
		{
			LOG("TOOLBOX_GET_FILE: out of range file %d, %d available\n", scsi_cmdbuf[1], (int)m_directory.size());
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
			return;
		}

		uint32_t offset = scsi_cmdbuf[2] << 24 | scsi_cmdbuf[3] << 16 | scsi_cmdbuf[4] << 8 | scsi_cmdbuf[5];
		LOG("TOOLBOX_GET_FILE: file # %d (%s), offset %08x\n", scsi_cmdbuf[1], m_directory[scsi_cmdbuf[1]].name, offset);

		std::string tmpPath(machine().options().share_directory());
		tmpPath.append(PATH_SEPARATOR);
		tmpPath.append(m_directory[scsi_cmdbuf[1]].name);

		osd_file::ptr file;
		u64 size;
		if (osd_file::open(tmpPath, OPEN_FLAG_READ, file, size))
		{
			LOG("Open failed for [%s]\n", tmpPath.c_str());
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
			return;
		}
		u32 actualRead;
		file->read(&m_xfer_buffer[0], offset * 4096, 4096, actualRead);

		if (actualRead == 0)
		{
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
			return;
		}

		m_xfer_position = 0;
		scsi_data_in(4, actualRead);
		scsi_status_complete(SS_GOOD);
		break;
	}

	case TOOLBOX_SEND_FILE_PREP:
		LOG("TOOLBOX_SEND_FILE_PREP\n");
		m_xfer_position = 0;
		m_write_is_setup = true;
		scsi_data_out(4, 33);
		scsi_status_complete(SS_GOOD);
		break;

	case TOOLBOX_SEND_FILE_10:
		m_xfer_position = 0;
		m_write_is_setup = false;
		m_write_length = scsi_cmdbuf[1] << 8 | scsi_cmdbuf[2];
		m_write_offset = scsi_cmdbuf[3] << 16 | scsi_cmdbuf[4] << 8 | scsi_cmdbuf[5];
		LOG("TOOLBOX_SEND_FILE_10 offset %08x len %04x\n", m_write_offset, m_write_length);
		scsi_data_out(4, m_write_length);
		scsi_status_complete(SS_GOOD);
		break;

	case TOOLBOX_SEND_FILE_END:
		LOG("TOOLBOX_SEND_FILE_END\n");
		scsi_status_complete(SS_GOOD);
		break;

	case TOOLBOX_COUNT_CDS:
		LOG("TOOLBOX_COUNT_CDS\n");
		scsi_cmdbuf[0] = 0;
		scsi_data_in(0, 1);
		scsi_status_complete(SS_GOOD);
		break;

	default:
		nscsi_full_device::scsi_command();
		break;
	}
}

enum sgi_scsi_command_e : uint8_t {
	/*
	 * The SGI supplied CD-ROM drives (and possibly those from some other vendors)
	 * identify themselves as hard disk drives at poweron, and after SCSI bus resets,
	 * until issued a vendor specific command (0xc9).  This is done because older
	 * systems would otherwise be unable to boot and load miniroots from CD, due to
	 * their design (they attempted to protect the user from booting from
	 * "ridiculous" devices, long before CD-ROM drives existed).  The SGI drives are
	 * sent a command to "revert" to CD-ROM inquiry information during boot if on
	 * a SCSI bus handled by the PROM, but not all possible buses are handled by all
	 * PROMs; additionally, a SCSI bus reset causes the CD-ROM drives to revert to
	 * the poweron default, and this could happen before the hardware inventory code
	 * in the kernel runs, if there are SCSI problems.
	 */
	SGI_HD2CDROM = 0xc9,
	/*
	 * IRIX 5.3 sends this command when it wants to eject the drive
	 */
	SGI_EJECT = 0xc4,
};

void nscsi_cdrom_sgi_device::scsi_command()
{
	switch (scsi_cmdbuf[0]) {
	case SGI_HD2CDROM:
		LOG("command SGI_HD2CDROM\n");
		// No need to do anything (yet). Just acknowledge the command.
		scsi_status_complete(SS_GOOD);
		break;

	case SGI_EJECT:
		LOG("command SGI_EJECT\n");
		// No need to do anything (yet). Just acknowledge the command.
		scsi_status_complete(SS_GOOD);
		break;

	default:
		nscsi_cdrom_device::scsi_command();
		break;
	}
}

bool nscsi_cdrom_sgi_device::scsi_command_done(uint8_t command, uint8_t length)
{
	switch (command) {
	case SGI_HD2CDROM:
		return length == 10;

	case SGI_EJECT:
		return length == 10;

	default:
		return nscsi_cdrom_device::scsi_command_done(command, length);
	}
}

// Apple CDSC commands, from "Apple CD-ROM SCSI Command Set", version 1.4, February 15, 1988.
// These are related to the Sony CDU-541 commands but customized by Apple.
enum apple_scsi_command_e : uint8_t {
	APPLE_EJECT            = 0xc0,      // EJECT DISC
	APPLE_READ_TOC         = 0xc1,      // READ TOC
	APPLE_READ_Q_SUBCODE   = 0xc2,      // READ Q SUBCODE
	APPLE_READ_HEADER      = 0xc3,      // READ HEADER
	APPLE_AUDIO_TRK_SEARCH = 0xc8,      // AUDIO TRACK SEARCH
	APPLE_AUDIO_PLAY       = 0xc9,      // AUDIO PLAY
	APPLE_AUDIO_PAUSE      = 0xca,      // AUDIO PAUSE
	APPLE_AUDIO_STOP       = 0xcb,      // AUDIO STOP
	APPLE_AUDIO_STATUS     = 0xcc,      // AUDIO STATUS
	APPLE_AUDIO_SCAN       = 0xcd,      // AUDIO SCAN
	APPLE_AUDIO_CONTROL    = 0xce
};

void nscsi_cdrom_apple_device::device_start()
{
	nscsi_cdrom_device::device_start();

	save_item(NAME(m_stopped));
	save_item(NAME(m_stop_position));
}

/*
   The Apple II SCSI Card firmware demands that ASC on a failing TEST_UNIT_READY be either 0x28 or 0xB0.
   0x28 is MEDIA_CHANGED, 0xB0 is vendor-specific.  If the drive returns the normal 0x3A for disc-not-present,
   the firmware assumes the drive is broken and retries the TEST_UNIT_READY for 60 seconds before giving up
   and booting the machine.

   MacOS will see the normal 0x3A disc-not-present and simply disbelieve it and hammer on the drive while
   asking the user to format it because it's unreadable.  0xB0 makes it behave as expected.
*/

void nscsi_cdrom_apple_device::return_no_cd()
{
	sense(false, SK_NOT_READY, 0xb0);
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_cdrom_apple_device::scsi_command()
{
	#if 1
	if (scsi_cmdbuf[0] != 8 && scsi_cmdbuf[0] != 0x28 && scsi_cmdbuf[0] != 0 && scsi_cmdbuf[0] != 0x03)
	{
		LOG("CD command: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   scsi_cmdbuf[0], scsi_cmdbuf[1], scsi_cmdbuf[2],
			   scsi_cmdbuf[3], scsi_cmdbuf[4], scsi_cmdbuf[5],
			   scsi_cmdbuf[6], scsi_cmdbuf[7], scsi_cmdbuf[8],
			   scsi_cmdbuf[9]);
	}
	#endif
	switch (scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		//LOG("command TEST UNIT READY (AppleCD)\n");
		if(image->exists())
		{
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			return_no_cd();
		}
		break;

	case SC_INQUIRY:
		{
			int lun = get_lun(scsi_cmdbuf[1] >> 5);
			LOG("command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
				lun, scsi_cmdbuf[1] & 1, scsi_cmdbuf[2], scsi_cmdbuf[4], scsi_cmdbuf[5]);

			int page = scsi_cmdbuf[2];
			int size = scsi_cmdbuf[4];
			switch (page)
			{
			case 0:
				std::fill_n(scsi_cmdbuf, size, 0);

				// data taken from the ROM of an AppleCD 150
				scsi_cmdbuf[0] = 0x05;   // device is present, device is CD/DVD (MMC-3)
				scsi_cmdbuf[1] = 0x80;       // media is removable
				scsi_cmdbuf[2] = 0x01;
				scsi_cmdbuf[3] = 0x01;
				scsi_cmdbuf[4] = 0x31;       // additional length
				memcpy((char *)&scsi_cmdbuf[8],  "SONY    ", 8);
				memcpy((char *)&scsi_cmdbuf[16], "CD-ROM CDU-8002 ", 16);
				memcpy((char *)&scsi_cmdbuf[32], "1.8g", 4);
				scsi_cmdbuf[39] = 0xd0;
				scsi_cmdbuf[40] = 0x90;
				scsi_cmdbuf[41] = 0x27;
				scsi_cmdbuf[42] = 0x3e;
				scsi_cmdbuf[43] = 0x01;
				scsi_cmdbuf[44] = 0x04;
				scsi_cmdbuf[45] = 0x91;
				scsi_cmdbuf[47] = 0x18;
				scsi_cmdbuf[48] = 0x06;
				scsi_cmdbuf[49] = 0xf0;
				scsi_cmdbuf[50] = 0xfe;

				scsi_data_in(SBUF_MAIN, size);
				break;
			}
			scsi_status_complete(SS_GOOD);
		}
		break;

	case APPLE_READ_TOC:
		{
			if (!image->exists())
			{
				return_no_cd();
				break;
			}

			u16 size = get_u16be(&scsi_cmdbuf[7]);
			LOG("command READ TOC (AppleCD), size=%d, track=%d, type1=%02x type2=%02x\n", size, scsi_cmdbuf[2], scsi_cmdbuf[9], scsi_cmdbuf[5]);

			// cmdbuf 9 bits 6 & 7:
			// 00 = first and last track # in BCD
			// 01 = starting address of the lead-out in M/S/F
			// 10 = starting address of a range of tracks from a starting track
			// 11 = reserved
			const uint8_t operation = scsi_cmdbuf[9] & 0xc0;
			if (operation == 0x80)
			{
				// get TOC info for 1 or more tracks from a specified starting track
				assert(scsi_cmdbuf[5] > 0);

				const int start_track = bcd_2_dec(scsi_cmdbuf[5]) - 1;
				const int num_trks = size / 4;
				int pos = 0;

				for (int trk = 0; trk < num_trks; trk++)
				{
					// Coverity: The emulated program could conceivably request a buffer size for
					// more tracks than exist, but we must still return the requested amount of data.
					if ((trk + start_track) < image->get_last_track())
					{
						const uint32_t start_lba = image->get_track_start(trk + start_track);
						const uint32_t start_frame = to_msf(start_lba);

						scsi_cmdbuf[pos++] = image->get_adr_control(trk + start_track);
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 16, 8)); // minutes
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 8, 8));  // seconds
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 0, 8));  // frames
					}
					else    // keep returning the last track if necessary
					{
						const uint32_t start_lba = image->get_track_start(image->get_last_track());
						const uint32_t start_frame = to_msf(start_lba);

						scsi_cmdbuf[pos++] = image->get_adr_control(image->get_last_track());
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 16, 8)); // minutes
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 8, 8));  // seconds
						scsi_cmdbuf[pos++] = dec_2_bcd(BIT(start_frame, 0, 8));  // frames
					}
				}

				scsi_data_in(SBUF_MAIN, pos);
				scsi_status_complete(SS_GOOD);
			}
			else if (operation == 0x40)    // get MSF of the start of the leadout
			{
				const int track = image->get_last_track();
				const uint32_t frame = to_msf(image->get_track_start(track));
				scsi_cmdbuf[0] = dec_2_bcd(BIT(frame, 16, 8)); // minutes
				scsi_cmdbuf[1] = dec_2_bcd(BIT(frame, 8, 8));  // seconds
				scsi_cmdbuf[2] = dec_2_bcd(BIT(frame, 0, 8));  // frames
				scsi_cmdbuf[3] = 0;

				scsi_data_in(SBUF_MAIN, 4);
				scsi_status_complete(SS_GOOD);
			}
			else if (operation == 0x00) // get start and end track numbers
			{
				scsi_cmdbuf[0] = 1;
				scsi_cmdbuf[1] = dec_2_bcd(image->get_last_track());
				scsi_cmdbuf[2] = 0;
				scsi_cmdbuf[3] = 0;

				scsi_data_in(SBUF_MAIN, 4);
				scsi_status_complete(SS_GOOD);
			}
		}
		break;

	case APPLE_AUDIO_STATUS:
		LOG("command APPLE AUDIO STATUS, type %02x, want %d bytes\n", scsi_cmdbuf[3], scsi_cmdbuf[8]);

		if (!image->exists())
		{
			return_no_cd();
			break;
		}

		// Audio status codes, adapted from CDRemote.equ in GS/OS 6.0.1 (matches Sony CDU-541 as well)
		enum cd_status_t: u8
		{
			PLAYING = 0,        // Audio is playing
			PAUSED,             // Audio is paused
			PLAYING_MUTED,      // Playing, but muted
			REACHED_END,        // Playback reached the end
			ERROR,              // an error occured
			VOID                // really "idle"
		};

		{
			const int type = scsi_cmdbuf[3];

			scsi_cmdbuf[2] = image->get_adr_control(image->get_track(cdda->get_audio_lba()));

			const uint32_t frame = to_msf(cdda->get_audio_lba());
			scsi_cmdbuf[3] = dec_2_bcd(BIT(frame, 16, 8));  // minute
			scsi_cmdbuf[4] = dec_2_bcd(BIT(frame, 8, 8));   // second
			scsi_cmdbuf[5] = dec_2_bcd(BIT(frame, 0, 8));   // frame

			if (type == 0)
			{
				scsi_cmdbuf[1] = 0;
				if (m_stopped)
				{
					scsi_cmdbuf[0] = VOID;
				}
				else
				{
					if (cdda->audio_ended())
					{
						scsi_cmdbuf[0] = REACHED_END;
					}
					else
					{
						if (cdda->audio_active())
						{
							if (cdda->audio_paused())
							{
								scsi_cmdbuf[0] = PAUSED;
							}
							else
							{
								scsi_cmdbuf[0] = PLAYING;
							}
						}
						else
						{
							scsi_cmdbuf[0] = VOID;
						}
					}
				}
			}
			else if (type == 1) // "volume status"
			{
				scsi_cmdbuf[0] = (u8)(cdda->output_gain(0) * 255.0f);
				scsi_cmdbuf[1] = (u8)(cdda->output_gain(1) * 255.0f);

				// Other 4 bytes unknown in this mode, going with the same as type 0 for now
			}
		}
		scsi_data_in(SBUF_MAIN, 6);
		scsi_status_complete(SS_GOOD);
		break;

	case APPLE_READ_Q_SUBCODE:
		{
			LOG("command READ SUB CHANNEL\n");

			scsi_cmdbuf[0] = 0; // Control nibble
			scsi_cmdbuf[1] = 0; // Track
			scsi_cmdbuf[2] = 0; // Index
			scsi_cmdbuf[3] = 0; // Relative Minute
			scsi_cmdbuf[4] = 0; // Relative Second
			scsi_cmdbuf[5] = 0; // Relative Frame
			scsi_cmdbuf[6] = 0; // AMinute
			scsi_cmdbuf[7] = 0; // ASecond
			scsi_cmdbuf[8] = 0; // AFrame

			const int track = image->get_track(cdda->get_audio_lba());
			scsi_cmdbuf[1] = dec_2_bcd(track+1);
			scsi_cmdbuf[2] = 1; // Index

			const uint32_t track_start = image->get_track_start(track);
			uint32_t cur_lba = cdda->get_audio_lba();

			if (cur_lba < track_start)
			{
				cur_lba = track_start;
			}

			const uint32_t relframe = to_msf(cur_lba - track_start);
			scsi_cmdbuf[3] = dec_2_bcd(BIT(relframe, 16, 8)); // minute
			scsi_cmdbuf[4] = dec_2_bcd(BIT(relframe, 8, 8));    // second
			scsi_cmdbuf[5] = dec_2_bcd(BIT(relframe, 0, 8));    // frame

			const uint32_t frame = to_msf(cur_lba);
			scsi_cmdbuf[6] = dec_2_bcd(BIT(frame, 16, 8)); // minute
			scsi_cmdbuf[7] = dec_2_bcd(BIT(frame, 8, 8));   // second
			scsi_cmdbuf[8] = dec_2_bcd(BIT(frame, 0, 8));   // frame

			scsi_data_in(SBUF_MAIN, 9);
			scsi_status_complete(SS_GOOD);
		}
		break;

	// This command does not change the play or pause status.
	case APPLE_AUDIO_TRK_SEARCH:
		{
			const uint8_t val = scsi_cmdbuf[5];
			const uint8_t start_track = (((val & 0xf0) >> 4) * 10) + (val & 0x0f);

			cdda->cancel_scan();
			if (cdda->audio_paused())
			{
				cdda->pause_audio(0);
			}

			uint32_t start_lba = 0;

			// cmdbuf[9] bits 6 & 7:
			// 00 = search address is LBA
			// 01 = search address is M/S/F
			// 10 = search address is track number
			if (scsi_cmdbuf[9] == 0x40)
			{
				LOG("command APPLE AUDIO TRACK SEARCH MSF = %02x:%02x:%02x\n", scsi_cmdbuf[5], scsi_cmdbuf[6], scsi_cmdbuf[7]);
				const uint32_t msf = (bcd_2_dec(scsi_cmdbuf[5]) << 16) | (bcd_2_dec(scsi_cmdbuf[6]) << 8) | bcd_2_dec(scsi_cmdbuf[7]);
				start_lba = cdrom_file::msf_to_lba(msf);
			}
			else if (scsi_cmdbuf[9] == 0x80)
			{
				LOG("command APPLE AUDIO TRACK SEARCH track = %d\n", start_track);
				if (start_track == 0)
				{
					cdda->stop_audio();
					scsi_status_complete(SS_GOOD);
					return;
				}
				else
				{
					start_lba = image->get_track_start(start_track - 1);
					m_stop_position = image->get_track_start(start_track);  // does this actually update the stop position?
				}
			}
			else if (scsi_cmdbuf[9] == 0x00)    // LBA
			{
				start_lba = (scsi_cmdbuf[5] << 24) | (scsi_cmdbuf[4] << 16) | (scsi_cmdbuf[3] << 8) || scsi_cmdbuf[2];
			}
			else
			{
				logerror("nscsi_cdrom_apple_device: Unknown APPLE AUDIO PLAY address mode %02x\n", scsi_cmdbuf[9]);
			}

			// PLAY / PAUSE bit
			if ((scsi_cmdbuf[1] & 0x10) == 0x10)
			{
				cdda->start_audio(start_lba, m_stop_position - start_lba);
			}
			else
			{
				cdda->start_audio(start_lba, m_stop_position - start_lba);
				cdda->pause_audio(true);
			}
			m_stopped = false;

			scsi_status_complete(SS_GOOD);
		}
		break;

	case APPLE_AUDIO_PLAY:
		{
			const uint8_t val = scsi_cmdbuf[5];
			const uint8_t start_track = (((val & 0xf0) >> 4) * 10) + (val & 0x0f);

			cdda->cancel_scan();
			if (cdda->audio_paused())
			{
				cdda->pause_audio(0);
			}

			// cmdbuf[9] bits 6 & 7:
			// 00 = address is LBA
			// 01 = address is M/S/F
			// 10 = address is track number
			uint32_t address = 0;
			if (scsi_cmdbuf[9] == 0x80)
			{
				LOG("command APPLE AUDIO PLAY track = %d\n", start_track);

				// The Mac version of Apple CD-ROM Explorer issues PLAY on track 0 to stop
				if (start_track == 0)
				{
					cdda->stop_audio();
					m_stopped = true;
					scsi_status_complete(SS_GOOD);
					return;
				}
				else
				{
					address = image->get_track_start(start_track - 1);
				}
			}
			else if (scsi_cmdbuf[9] == 0x40)
			{
				LOG("command APPLE AUDIO PLAY TRACK MSF = %02x:%02x:%02x\n", scsi_cmdbuf[3], scsi_cmdbuf[4], scsi_cmdbuf[5]);

				const uint32_t msf = (bcd_2_dec(scsi_cmdbuf[3]) << 16) | (bcd_2_dec(scsi_cmdbuf[4]) << 8) | bcd_2_dec(scsi_cmdbuf[5]);
				address = cdrom_file::msf_to_lba(msf);
			}
			else if (scsi_cmdbuf[9] == 0x00)
			{
				address = (scsi_cmdbuf[5] << 24) | (scsi_cmdbuf[4] << 16) | (scsi_cmdbuf[3] << 8) || scsi_cmdbuf[2];
				LOG("command APPLE AUDIO PLAY lba = %d\n", address);
			}
			else
			{
				fatalerror("nscsi_cdrom_apple_device: unknown mode for APPLE AUDIO PLAY %02x\n", scsi_cmdbuf[9]);
			}

			// cmdbuf[1] & 0x10:
			// 0 = address is the starting address, stop address was set by a STOP command
			// 1 = address is the stop address, start address was set by a TRACK SEARCH command
			if (scsi_cmdbuf[1] & 0x10)
			{
				const auto start = cdda->get_audio_lba();
				cdda->start_audio(start, address - start);
			}
			else
			{
				cdda->start_audio(address, m_stop_position - address);
				m_stopped = false;
			}
			scsi_status_complete(SS_GOOD);
		}
		break;

	case APPLE_AUDIO_SCAN:
		/*
		    3/4/5 = MSF of starting position
		    1 = 00 for forward scan, 0x10 for reverse
		    Scan ends with a PAUSE OFF command.
		*/
		LOG("command APPLE_AUDIO_SCAN: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			scsi_cmdbuf[0], scsi_cmdbuf[1], scsi_cmdbuf[2],
			scsi_cmdbuf[3], scsi_cmdbuf[4], scsi_cmdbuf[5],
			scsi_cmdbuf[6], scsi_cmdbuf[7], scsi_cmdbuf[8],
			scsi_cmdbuf[9]);

		if (!m_stopped)
		{
			uint32_t start_lba = 0;
			switch (scsi_cmdbuf[9] & 0xc0)
			{
			case 0x00: // stop at LBA
				start_lba = (scsi_cmdbuf[5] << 24) | (scsi_cmdbuf[4] << 16) | (scsi_cmdbuf[3] << 8) || scsi_cmdbuf[2];
				LOG("command APPLE AUDIO SCAN at LBA %d\n", m_stop_position);
				break;

			case 0x40: // stop at MSF
			{
				const uint32_t msf = (bcd_2_dec(scsi_cmdbuf[5]) << 16) | (bcd_2_dec(scsi_cmdbuf[6]) << 8) | bcd_2_dec(scsi_cmdbuf[7]);
				start_lba = cdrom_file::msf_to_lba(msf);
				LOG("command APPLE AUDIO SCAN at MSF %02x:%02x:%02x\n", scsi_cmdbuf[5], scsi_cmdbuf[6], scsi_cmdbuf[7]);
			}
			break;

			case 0x80: // stop at track number
			{
				const uint8_t start_track = bcd_2_dec(scsi_cmdbuf[5]);
				if (start_track >= image->get_last_track())
				{
					start_lba = image->get_track_start(0xaa);
				}
				else
				{
					start_lba = image->get_track_start(start_track + 1);
				}
				start_lba--;
				LOG("command APPLE AUDIO SCAN at end of track %d (LBA %d)\n", start_track, m_stop_position);
			}
			break;

			default:
				logerror("nscsi_cdrom_apple_device: Unknown APPLE AUDIO SCAN address mode %02x\n", scsi_cmdbuf[9]);
				break;
			}

			// begin seeking from the specified address
			cdda->set_audio_lba(start_lba);

			if ((scsi_cmdbuf[1] & 0x10) == 0)
			{
				cdda->scan_forward();
			}
			else
			{
				cdda->scan_reverse();
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case APPLE_AUDIO_PAUSE:
		LOG("command APPLE AUDIO PAUSE, on/off=%02x\n", scsi_cmdbuf[1]);
		if (scsi_cmdbuf[1] == 0x10)
		{
			// set pause
			if (!cdda->audio_paused())
			{
				cdda->pause_audio(1);
			}
		}
		else
		{
			// release pause and stop scanning
			cdda->cancel_scan();
			if (cdda->audio_paused())
			{
				cdda->pause_audio(0);
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case APPLE_AUDIO_STOP:
		switch (scsi_cmdbuf[9] & 0xc0)
		{
			case 0x00:  // stop at LBA
				m_stop_position = (scsi_cmdbuf[5] << 24) | (scsi_cmdbuf[4] << 16) | (scsi_cmdbuf[3] << 8) || scsi_cmdbuf[2];
				LOG("command APPLE AUDIO STOP at LBA %d\n", m_stop_position);
				break;

			case 0x40: // stop at MSF
				{
					const uint32_t msf = (bcd_2_dec(scsi_cmdbuf[5]) << 16) | (bcd_2_dec(scsi_cmdbuf[6]) << 8) | bcd_2_dec(scsi_cmdbuf[7]);
					m_stop_position = cdrom_file::msf_to_lba(msf);
					LOG("command APPLE AUDIO STOP at MSF %02x:%02x:%02x\n", scsi_cmdbuf[5], scsi_cmdbuf[6], scsi_cmdbuf[7]);
				}
				break;

			case 0x80: // stop at track number
				{
					const uint8_t start_track = bcd_2_dec(scsi_cmdbuf[5]);
					if (start_track >= image->get_last_track())
					{
						m_stop_position = image->get_track_start(0xaa);
					}
					else
					{
						m_stop_position = image->get_track_start(start_track + 1);
					}
					m_stop_position--;
					LOG("command APPLE AUDIO STOP at end of track %d (LBA %d)\n", start_track, m_stop_position);
				}
				break;

			default:
				logerror("nscsi_cdrom_apple_device: Unknown APPLE AUDIO STOP address mode %02x\n", scsi_cmdbuf[9]);
				break;
		}

		if ((!m_stopped) && (cdda->get_audio_lba() >= m_stop_position))
		{
			m_stopped = true;
			cdda->stop_audio();
		}

		scsi_status_complete(SS_GOOD);
		break;

	case APPLE_AUDIO_CONTROL:
		LOG("command APPLE AUDIO CONTROL, size %d\n", scsi_cmdbuf[8]);

		if (image->exists())
		{
			scsi_data_out(3, scsi_cmdbuf[8]);
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			return_no_cd();
		}
		break;

	case APPLE_EJECT:
		LOG("command APPLE EJECT\n");
		if (!m_removal_prevented)
		{
			cdda->stop_audio();
			m_stopped = true;
			image->unload();
			sense(false, SK_NOT_READY, SK_ASC_MEDIUM_NOT_PRESENT);
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			LOG("Eject not allowed by PREVENT_ALLOW_MEDIA_REMOVAL\n");
			sense(false, SK_ILLEGAL_REQUEST, 0x80);     // "Prevent bit is set"
			scsi_status_complete(SS_CHECK_CONDITION);
		}
		break;

	case SC_READ_6:
	case SC_READ_10:
	case SC_READ_12:
		cdda->stop_audio();
		m_stopped = true;
		[[fallthrough]];

	default:
		nscsi_cdrom_device::scsi_command();
		break;
	}
}

bool nscsi_cdrom_apple_device::scsi_command_done(uint8_t command, uint8_t length)
{
	switch (command & 0xf0)
	{
		case 0xc0:
			return length == 10;

		default:
			return nscsi_cdrom_device::scsi_command_done(command, length);
	}
}

void nscsi_cdrom_apple_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if (id != 3)
	{
		nscsi_cdrom_device::scsi_put_data(id, pos, data);
		return;
	}

	if (pos == 0)
	{
		cdda->set_output_gain(0, data / 255.0f);
	}
	else if (pos == 1)
	{
		cdda->set_output_gain(1, data / 255.0f);
	}

}
