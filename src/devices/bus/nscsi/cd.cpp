// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "bus/nscsi/cd.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device, "scsi_cdrom", "SCSI CD-ROM")
DEFINE_DEVICE_TYPE(NSCSI_CDROM_SGI, nscsi_cdrom_sgi_device, "scsi_cdrom_sgi", "SCSI CD-ROM SGI")
DEFINE_DEVICE_TYPE(NSCSI_RRD45, nscsi_dec_rrd45_device, "nrrd45", "RRD45 CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM3301, nscsi_toshiba_xm3301_device, "nxm3301", "XM-3301TA CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5301SUN, nscsi_toshiba_xm5301_sun_device, "nxm5301sun", "XM-5301B Sun 4x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5401SUN, nscsi_toshiba_xm5401_sun_device, "nxm5401sun", "XM-5401B Sun 4x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5701, nscsi_toshiba_xm5701_device, "nxm5701", "XM-5701B 12x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_XM5701SUN, nscsi_toshiba_xm5701_sun_device, "nxm5701sun", "XM-5701B Sun 12x CD-ROM (New)")
DEFINE_DEVICE_TYPE(NSCSI_CDROM_APPLE, nscsi_cdrom_apple_device, "scsi_cdrom_apple", "Apple SCSI CD-ROM")

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
{
}

nscsi_cdrom_sgi_device::nscsi_cdrom_sgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_SGI, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
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

nscsi_cdrom_apple_device::nscsi_cdrom_apple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_APPLE, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
{
}

nscsi_cdrom_device::nscsi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_full_device(mconfig, type, tag, owner, clock)
	, cdrom(nullptr)
	, bytes_per_block(bytes_per_sector)
	, lba(0)
	, cur_sector(0)
	, image(*this, "image")
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

void nscsi_cdrom_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, image).set_interface("cdrom");
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
		throw emu_fatalerror("nscsi_cdrom_device(%s): block size must be a factor of sector size", tag());

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

void nscsi_cdrom_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if(id != 2) {
		nscsi_full_device::scsi_put_data(id, pos, data);
		return;
	}

	// process mode parameter header and one block descriptor
	if(pos < sizeof(mode_data)) {
		mode_data[pos] = data;

		// is this the last byte of the mode parameter block descriptor?
		if(pos == sizeof(mode_data) - 1)
			// is there exactly one block descriptor?
			if(mode_data[3] == 8)
				set_block_size((mode_data[9] << 16) | (mode_data[10] << 8) | (mode_data[11] << 0));
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
	if((cdrom != image->get_cdrom_file()) && (scsi_cmdbuf[0] != SC_INQUIRY))
	{
		// clear media change condition
		cdrom = image->get_cdrom_file();
		cur_sector = -1;

		// report unit attention condition
		sense(false, SK_UNIT_ATTENTION);
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
			std::fill_n(scsi_cmdbuf, 36, 0);

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

			if(size > 36)
				size = 36;
			scsi_data_in(SBUF_MAIN, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}

	case SC_MODE_SELECT_6:
		LOG("command MODE SELECT 6 length %d\n", scsi_cmdbuf[4]);

		// accept mode select parameter data
		if(scsi_cmdbuf[4])
			scsi_data_out(2, scsi_cmdbuf[4]);

		scsi_status_complete(SS_GOOD);
		break;

	case SC_START_STOP_UNIT:
		LOG("command %s UNIT%s\n", (scsi_cmdbuf[4] & 0x1) ? "START" : "STOP",
			(scsi_cmdbuf[4] & 0x2) ? (scsi_cmdbuf[4] & 0x1) ? " (LOAD)" : " (EJECT)" : "");
		scsi_status_complete(SS_GOOD);
		break;

	case SC_RECIEVE_DIAG_RES: {
		LOG("command RECIEVE DIAGNOSTICS RESULTS");
		int size = (scsi_cmdbuf[3] << 8) | scsi_cmdbuf[4];
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

	case SC_SEND_DIAGNOSTICS: {
		LOG("command SEND DIAGNOSTICS");
		int size = (scsi_cmdbuf[3] << 8) | scsi_cmdbuf[4];
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
		if(!cdrom) {
			return_no_cd();
			break;
		}

		LOG("command READ CAPACITY\n");

		// get the last used block on the disc
		const u32 temp = cdrom_get_track_start(cdrom, 0xaa) * (bytes_per_sector / bytes_per_block) - 1;

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
		lba = (scsi_cmdbuf[2]<<24) | (scsi_cmdbuf[3]<<16) | (scsi_cmdbuf[4]<<8) | scsi_cmdbuf[5];
		blocks = (scsi_cmdbuf[7] << 8) | scsi_cmdbuf[8];

		LOG("command READ EXTENDED start=%08x blocks=%04x\n", lba, blocks);
		if(!cdrom) {
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

		int page = scsi_cmdbuf[2] & 0x3f;
		int size = scsi_cmdbuf[4];
		int pos = 1;
		scsi_cmdbuf[pos++] = 0x00; // medium type
		scsi_cmdbuf[pos++] = 0x80; // WP, cache

		// get the last used block on the disc
		const u32 temp = cdrom_get_track_start(cdrom, 0xaa) * (bytes_per_sector / bytes_per_block) - 1;
		scsi_cmdbuf[pos++] = 0x08; // Block descriptor length

		scsi_cmdbuf[pos++] = 0x00; // density code
		scsi_cmdbuf[pos++] = (temp>>16) & 0xff;
		scsi_cmdbuf[pos++] = (temp>>8) & 0xff;
		scsi_cmdbuf[pos++] = (temp & 0xff);
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = 0;
		scsi_cmdbuf[pos++] = (bytes_per_block>>8)&0xff;
		scsi_cmdbuf[pos++] = (bytes_per_block & 0xff);

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

			default:
				if (page != 0x3f) {
					LOG("mode sense page %02x unhandled\n", p);
					fail = true;
				}
				break;
			}
		}
		scsi_cmdbuf[0] = pos;
		if(pos > size)
			pos = size;

		if (!fail) {
			scsi_data_in(0, pos);
			scsi_status_complete(SS_GOOD);
		} else {
			scsi_status_complete(SS_CHECK_CONDITION);
			sense(false, SK_ILLEGAL_REQUEST, SK_ASC_INVALID_FIELD_IN_CDB);
		}
		break;
	}

	case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
		// TODO: support eject prevention
		LOG("command %s MEDIUM REMOVAL\n", (scsi_cmdbuf[4] & 0x1) ? "PREVENT" : "ALLOW");
		scsi_status_complete(SS_GOOD);
		break;

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
			"ATIP"
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
		u16 size = (scsi_cmdbuf[7] << 7) | scsi_cmdbuf[8];
		u8 format = scsi_cmdbuf[2] & 15;

		/// SFF8020 legacy format field (see T10/1836-D Revision 2g page 643)
		if(!format)
			format = (scsi_cmdbuf[9] >> 6) & 3;

		LOG("command READ TOC PMA ATIP, format %s msf=%d size=%d\n", format_names[format], msf, size);

		int pos = 0;
		switch (format) {
		case 0: {
			int start_track = scsi_cmdbuf[6];
			int end_track = cdrom_get_last_track(cdrom);

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
			scsi_cmdbuf[pos++] = (len>>8) & 0xff;
			scsi_cmdbuf[pos++] = (len & 0xff);
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = cdrom_get_last_track(cdrom);

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
				scsi_cmdbuf[pos++] = cdrom_get_adr_control(cdrom, cdrom_track);
				scsi_cmdbuf[pos++] = track;
				scsi_cmdbuf[pos++] = 0;

				u32 tstart = cdrom_get_track_start(cdrom, cdrom_track);

				if(msf)
					tstart = to_msf(tstart+150);

				scsi_cmdbuf[pos++] = (tstart>>24) & 0xff;
				scsi_cmdbuf[pos++] = (tstart>>16) & 0xff;
				scsi_cmdbuf[pos++] = (tstart>>8) & 0xff;
				scsi_cmdbuf[pos++] = (tstart & 0xff);
			}
			break;
		}

		case 1: {
			int len = 2 + (8 * 1);

			scsi_cmdbuf[pos++] = (len>>8) & 0xff;
			scsi_cmdbuf[pos++] = (len & 0xff);
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = 1;

			scsi_cmdbuf[pos++] = 0;
			scsi_cmdbuf[pos++] = cdrom_get_adr_control(cdrom, 0);
			scsi_cmdbuf[pos++] = 1;
			scsi_cmdbuf[pos++] = 0;

			u32 tstart = cdrom_get_track_start(cdrom, 0);

			if (msf)
				tstart = to_msf(tstart+150);

			scsi_cmdbuf[pos++] = (tstart>>24) & 0xff;
			scsi_cmdbuf[pos++] = (tstart>>16) & 0xff;
			scsi_cmdbuf[pos++] = (tstart>>8) & 0xff;
			scsi_cmdbuf[pos++] = (tstart & 0xff);
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
};

void nscsi_cdrom_sgi_device::scsi_command()
{
	switch (scsi_cmdbuf[0]) {
	case SGI_HD2CDROM:
		LOG("command SGI_HD2CDROM");
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

	default:
		return nscsi_full_device::scsi_command_done(command, length);
	}
}
/*
   The Apple II SCSI Card firmware demands that ASC on a failing TEST_UNIT_READY be either 0x28 or 0xb0.
   0x28 is MEDIA_CHANGED, 0xb0 is vendor-specific.  If the drive returns the normal 0x3A for disc-not-present,
   the firmware assumes the drive is broken and retries the TEST_UNIT_READY for 60 seconds before giving up
   and booting the machine.
*/
void nscsi_cdrom_apple_device::scsi_command()
{
	switch (scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
		LOG("command TEST UNIT READY (AppleCD)\n");
		if(cdrom)
		{
			scsi_status_complete(SS_GOOD);
		}
		else
		{
			sense(false, SK_NOT_READY, 0xb0);
			scsi_status_complete(SS_CHECK_CONDITION);
		}
		break;

	default:
		nscsi_cdrom_device::scsi_command();
		break;
	}
}
