// license:BSD-3-Clause
// copyright-holders: 68bit
/*
 * mdos_dsk.c  -  Motorola MDOS compatible disk images
 *
 * The format is largely IBM 3740 compatible, with 77 tracks, and 26 sectors
 * per track, and a sector size of 128 bytes. Single sided disks were initially
 * supported and later software supported double sided disk with 52 sectors per
 * cylinder. The sectors are numbered from one, and the sectors indexes on the
 * second side continue from the first side rather than starting again at
 * one.
 *
 * The disk drives run at 360rpm, and the bit clock frequency at 500kHz.  This
 * gives 6 revs per second, and 500000 / 6 = 83333 cells per track. MAME uses
 * disk rotation position units of 1/200000000 of a turn and the cells size in
 * this unit is 200000000 / (500000 / 6) = 2400.
 *
 * The M68SFDC2 user guide details the format gaps. "The Post-Index Gap is
 * defined as the 32 bytes between Index Address Mark and the ID Address Mark
 * for sector 1 (excluding the address mark bytes). This gap is always 32 bytes
 * in length and is not affect by any up-dating process. The ID Gap consists of
 * 17 bytes between the ID Field and the Data Field. This gap may vary in size
 * slightly after the Data Field has been updated. The Data Gap consists of 33
 * bytes between the Data Field and the next ID File. This gap may also vary
 * slightly after the Data Field has been updated. The Pre-Index Gap is a space
 * of 320 bytes between the last data field on the track and the Index Address
 * Mark. This gap may also vary in length."
 *
 * These gaps do appear to include the leading zero bytes before the sync
 * marks, in contrast to the gaps defined in the formats below so corrections
 * have been applied. The trailing gap has been checked and it close to the 320
 * bytes expected.
 *
 * The MDOS 3.05 FORMAT command with (the current emulator timing) varies from
 * the above slightly. An index marker has been added, 106 bytes after the
 * start of the track - a 100 byte gap and then 6 bytes of zeros and an index
 * marker 0xf77a. This index marker does not appear to be used. The post-index
 * marker gap is 30 bytes between the index marker and the address marker - a
 * gap of 24 bytes and then 6 bytes of zeros and then the address marker
 * 0xf57e. The ID Gap consist of roughly 18 bytes between the ID Field and the
 * Data file - 12 bytes gap and 6 bytes of zero, and then the data marker
 * 0xf565. The Data Gap consists of 33 bytes - a gap of 27 bytes and 6 bytes of
 * zeros and then the address marker 0xf57e. The Pre-Index gap is a space of
 * 209 bytes, but this now extends to the start of the track to the index
 * marker.
 *
 * TODO The MDOS 3 FORMAT command writes a zero in the address mark 'head'
 * field irrespective of the disk head and the software ignores this field when
 * read.
 */

#include "mdos_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"


mdos_format::mdos_format() : wd177x_format(formats)
{
}

const char *mdos_format::name() const
{
	return "mdos";
}

const char *mdos_format::description() const
{
	return "Motorola MDOS compatible disk image";
}

const char *mdos_format::extensions() const
{
	return "dsk";
}

int mdos_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return 75;

	return 0;
}

bool mdos_format::check_ascii(uint8_t *str, size_t len, const char* name)
{
	LOG_FORMATS(" %s: \"", name);
	for (int i = 0; i < len; i++) {
		uint8_t ch = str[i];
		if (ch == 0) {
			LOG_FORMATS("\\0");
		} else if (ch < 0x20 || ch > 0x7f) {
			return 0;
		} else {
			LOG_FORMATS("%c", ch);
		}
	}
	LOG_FORMATS("\"\n");
	return 1;
}

int mdos_format::parse_date_field(uint8_t *str)
{
	uint8_t high = str[0];
	uint8_t low = str[1];

	if (high < 0x30 || high > 0x39 || low < 0x30 || low > 0x39)
		return -1;

	return (high - 0x30) * 10 + (low - 0x30);
}

int mdos_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	size_t actual;
	uint64_t size;
	if (io.length(size))
		return -1;

	// Look at the disk id sector.
	io.read_at(0, &info, sizeof(struct disk_id_sector), actual);

	LOG_FORMATS("MDOS floppy dsk: size %d bytes, %d total sectors, %d remaining bytes, expected form factor %x\n", (uint32_t)size, (uint32_t)size / 128, (uint32_t)size % 128, form_factor);

	// The 'unused' area is not necessarily zero filled and is ignoded
	// in the identification of a MDOS format image.

	// Expect an ASCII id, version, revision, and 'user name' strings.
	if (!check_ascii(info.id, sizeof(info.id), "id"))
		return -1;

	if (!check_ascii(info.version, sizeof(info.version), "version"))
		return -1;

	if (!check_ascii(info.revision, sizeof(info.revision), "revision"))
		return -1;

	if (!check_ascii(info.date, sizeof(info.date), "date"))
		return -1;

	if (!check_ascii(info.username, sizeof(info.username), "username"))
		return -1;

	// The date should be the numeric day, month and year.
	int month = parse_date_field(info.date);
	int day = parse_date_field(info.date + 2);
	int year = parse_date_field(info.date + 4);

	LOG_FORMATS(" day %d, month %d, year %d\n", day, month, year);
	if (day < 1 || day > 32 || month < 1 || month > 12 || year < 0)
		return -1;

	// The RIB address seems to be a sequence of 16 bit cluster numbers,
	// ending in a zero word. The maximum size appears to be 20 bytes, or
	// 10 words. The area beyond the zero word is not always zero filled,
	// and is ignored here. Check that it is consistent with this.
	for (int i = 0; i < 10; i++) {
		uint8_t high = info.rib_addr[i * 2];
		uint8_t low = info.rib_addr[i * 2 + 1];
		uint16_t cluster = (high << 8) | low;

		if (cluster == 0)
			break;

		// The lowest value cluster here appears to be 92, just after
		// the director sectors.
		if (cluster < 92) {
			LOG_FORMATS(" id sector rib address %d unexpectedly low\n", cluster);
			return -1;
		}

		if (cluster * 4 * 128 + 4 * 128 > size) {
			LOG_FORMATS(" id sector rib address %d beyond disk extent\n", cluster);
			return -1;
		}
	}

	// Check that the cluster allocation table and the cluster available
	// table are consistent with the disk size, that no clusters beyond
	// the extent of the disk are free or available.

	uint8_t cluster_allocation[128], cluster_available[128];
	io.read_at(1 * 128, &cluster_allocation, sizeof(cluster_allocation), actual);
	io.read_at(2 * 128, &cluster_available, sizeof(cluster_available), actual);

	for (int cluster = 0; cluster < sizeof(cluster_allocation) * 8; cluster++) {
		if (cluster * 4 * 128 + 4 * 128 > size) {
			// This cluster does not fit within the disk size, so
			// should be marked as not free and not available.
			uint8_t mask = 1 << (7 - (cluster & 7));
			uint8_t offset = cluster >> 3;
			if ((cluster_allocation[offset] & mask) == 0) {
				LOG_FORMATS("  unexpected free cluster %d beyond disk extent\n", cluster);
				return -1;
			}
			if ((cluster_available[offset] & mask) == 0) {
				LOG_FORMATS("  unexpected available cluster %d beyond disk extent\n", cluster);
				return -1;
			}
		}
	}

	for (int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		LOG_FORMATS(" checking format %d with form factor %02x, %d sectors, %d heads\n",
				i, f.form_factor, f.sector_count, f.head_count);
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		unsigned int format_size = 0;
		for (int track=0; track < f.track_count; track++) {
			for (int head=0; head < f.head_count; head++) {
				const format &tf = get_track_format(f, head, track);
				format_size += compute_track_size(tf);
			}
		}

		LOG_FORMATS(" image size %u\n", format_size);
		if (format_size != size)
			continue;

		LOG_FORMATS("MDOS matching format index %d\n", i);
		return i;
	}
	return -1;
}

const wd177x_format::format &mdos_format::get_track_format(const format &f, int head, int track)
{
	int n = -1;

	for (int i = 0; formats[i].form_factor; i++) {
		if (&formats[i] == &f) {
			n = i;
			break;
		}
	}

	if (n < 0) {
		LOG_FORMATS("Error format not found\n");
		return f;
	}

	if (head >= f.head_count) {
		LOG_FORMATS("Error invalid head %d\n", head);
		return f;
	}

	if (track >= f.track_count) {
		LOG_FORMATS("Error invalid track %d\n", track);
		return f;
	}

	if (head == 1) {
		const format &fh1 = formats_head1[n];
		if (!fh1.form_factor) {
			LOG_FORMATS("Error expected a head 1 format\n");
			return f;
		}
		return fh1;
	}

	return f;
}

const mdos_format::format mdos_format::formats[] = {
	{ // 55 250.25K 8 inch single sided single density
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2400, 26, 77, 1, 128, {}, 1, {}, 32-6, 17-6, 33-6
	},
	{ // 57 500.5K 8 inch double sided single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2400, 26, 77, 2, 128, {}, 1, {}, 32-6, 17-6, 33-6
	},
	{}
};

const mdos_format::format mdos_format::formats_head1[] = {
	{ // 55 250.25K 8 inch single sided single density
	},
	{ // 57 500.5K 8 inch double sided single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2400, 26, 77, 2, 128, {}, 27, {}, 32-6, 17-6, 33-6
	},
	{}
};

const floppy_format_type FLOPPY_MDOS_FORMAT = &floppy_image_format_creator<mdos_format>;
