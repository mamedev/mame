// license:BSD-3-Clause
// copyright-holders: 68bit
/*
 * mdos_dsk.c  -  Motorola MDOS compatible disk images
 *
 * The format is largely IBM 3740 compatible with a sector size of 128
 * bytes. The sectors are numbered from one, and the sectors indexes on the
 * second side continue from the first side rather than starting again at
 * one. The address mark 'head' field is filled with zero irrespective of the
 * disk head and the software ignores this field when read.
 *
 * The 8 inch disk format has 77 tracks, and 26 sectors per track, and 52
 * sectors per cylinder for double sided disks. The disk drives run at 360rpm
 * or 6 revs per second and the bit clock frequency is 500kHz which gives
 * 500000 / 6 = 83333 cells per track. MAME uses disk rotation position units
 * of 1/200000000 of a turn and the cell size in this unit is 200000000 /
 * (500000 / 6) = 2400.
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
 * The XDOS format disk is largely compatible with MDOS, adding 5.25 inch disk
 * formats. The XDOS4 format command also numbers sectors indexes starting at
 * 1 and continuing on the second side for double sided disk, and also writes
 * a zero in the address mark 'head' field irrespective of the disk head. The
 * interleave for the 5.25 inch disk format is 2. The 5.25 inch disk drives
 * run at 300rpm or 5 revs per second and the bit clock is 250kHz which gives
 * 250000 / 5 = 50000 cells per track. MAME uses disk rotation position units
 * of 1/200000000 of a turn and the cell size in this unit is 200000000 /
 * (250000 / 5) = 4000. The disk format is:
 *
 * 80 x 0xff,
 * For each sector (16):
 *   6 x 0x00, 0xF57E, 4 x data, 2 x CRC,
 *   12 x 0xff,
 *   6 x 0x00, 0xF56F, 128 x data, 2 x CRC,
 *   27 x FF
 *
 * giving a total of 3104 bytes and at 16 cells per byte gives 49664 cells.
 */

#include "mdos_dsk.h"
#include "formats/basicdsk.h"
#include "formats/imageutl.h"


static bool check_ascii(uint8_t *str, size_t len, const char* name)
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

static int parse_date_field(uint8_t *str)
{
	uint8_t high = str[0];
	uint8_t low = str[1];

	if (high < 0x30 || high > 0x39 || low < 0x30 || low > 0x39)
		return -1;

	return (high - 0x30) * 10 + (low - 0x30);
}

struct disk_id_sector
{
	uint8_t id[8];
	uint8_t version[2];
	uint8_t revision[2];
	uint8_t date[6];
	uint8_t username[20];
	uint8_t rib_addr[20];
	uint8_t unused[70];
};

// The 'unused' area is not necessarily zero filled and is ignored in the
// identification of a MDOS format image.
//
// XDOS stores a table of the usable sectors in the id sector, see the table
// below. The total number of usable sectors is the total rounded double to
// the nearest multiple of four - allocation is in multiples of four sector
// clusters.
//
// 0x70    disks 0-3 single sided sectors per cylinder
// 0x71,2  disks 0-3 single sided total usable sectors
// 0x73    disks 0-3 double sided sectors per cylinder
// 0x74,5  disks 0-3 double sided total usable sectors
// 0x76    disks 4-7 single sided sectors per cylinder
// 0x77,8  disks 4-7 single sided total usable sectors
// 0x79    disks 4-7 double sided sectors per cylinder
// 0x7a,b  disks 4-7 double sided total usable sectors
//
static int check_id_sector(struct disk_id_sector *info, uint64_t size)
{
	// Expect an ASCII id, version, revision, and 'user name' strings.
	if (!check_ascii(info->id, sizeof(info->id), "id"))
		return 0;

	if (!check_ascii(info->version, sizeof(info->version), "version"))
		return 0;

	if (!check_ascii(info->revision, sizeof(info->revision), "revision"))
		return 0;

	if (!check_ascii(info->date, sizeof(info->date), "date"))
		return 0;

	if (!check_ascii(info->username, sizeof(info->username), "username"))
		return 0;

	// The date should be the numeric day, month and year.
	// Permit the day and month to be in either order.
	int day_month1 = parse_date_field(info->date);
	int day_month2 = parse_date_field(info->date + 2);
	int year = parse_date_field(info->date + 4);

	LOG_FORMATS(" day/month %d, day/month %d, year %d\n", day_month1, day_month2, year);
	if (day_month1 < 1 || day_month1 > 32 ||
		day_month2 < 1 || day_month2 > 32 ||
		(day_month1 > 12 && day_month2 > 12) ||
		year < 0)
	{
		return 0;
	}

	// The RIB address seems to be a sequence of 16 bit cluster numbers,
	// ending in a zero word. The maximum size appears to be 20 bytes, or
	// 10 words. The area beyond the zero word is not always zero filled,
	// and is ignored here. Check that it is consistent with this.
	for (int i = 0; i < 10; i++) {
		uint8_t high = info->rib_addr[i * 2];
		uint8_t low = info->rib_addr[i * 2 + 1];
		uint16_t cluster = (high << 8) | low;

		if (cluster == 0)
			break;

		// The lowest value cluster here appears to be 6, just after
		// the director sectors.
		if (cluster < 6) {
			LOG_FORMATS(" id sector rib address %d unexpectedly low\n", cluster);
			return 0;
		}

		if (cluster * 4 * 128 + 4 * 128 > size) {
			LOG_FORMATS(" id sector rib address %d beyond disk extent\n", cluster);
			return 0;
		}
	}

	return 1;
}

static int check_alloc_tables(uint8_t cluster_allocation[128], uint8_t cluster_available[128], uint64_t size)
{
	// Check that the cluster allocation table and the cluster available
	// table are consistent with the disk size, that no clusters beyond
	// the extent of the disk are free or available.

	for (int cluster = 0; cluster < 128 * 8; cluster++) {
		if (cluster * 4 * 128 + 4 * 128 > size) {
			// This cluster does not fit within the disk size, so
			// should be marked as not free and not available.
			uint8_t mask = 1 << (7 - (cluster & 7));
			uint8_t offset = cluster >> 3;
			if ((cluster_allocation[offset] & mask) == 0) {
				LOG_FORMATS("  unexpected free cluster %d beyond disk extent\n", cluster);
				return 0;
			}
			if ((cluster_available[offset] & mask) == 0) {
				LOG_FORMATS("  unexpected available cluster %d beyond disk extent\n", cluster);
				return 0;
			}
		}
	}

	return 1;
}


static floperr_t mdos_dsk_readheader(floppy_image_legacy *floppy, struct basicdsk_geometry *geometry)
{
	struct disk_id_sector info;
	uint64_t size;

	size = floppy_image_size(floppy);

	if (size < 3 * 128 || size % 128 != 0)
		return FLOPPY_ERROR_INVALIDIMAGE;

	floppy_image_read(floppy, &info, 0, sizeof(info));

	if (!check_id_sector(&info, size))
		return FLOPPY_ERROR_INVALIDIMAGE;

	uint8_t cluster_allocation[128], cluster_available[128];
	floppy_image_read(floppy, &cluster_allocation, 1 * 128, sizeof(cluster_allocation));
	floppy_image_read(floppy, &cluster_available, 2 * 128, sizeof(cluster_available));

	if (!check_alloc_tables(cluster_allocation, cluster_available, size))
		return FLOPPY_ERROR_INVALIDIMAGE;

	memset(geometry, 0, sizeof(*geometry));
	geometry->first_sector_id = 1;
	geometry->sector_length = 128;

	if (size == 81920)
	{
		geometry->tracks = 40;
		geometry->sectors = 16;
		geometry->heads = 1;
	}
	else if (size == 2 * 81920)
	{
		geometry->tracks = 40;
		geometry->sectors = 16;
		geometry->heads = 2;
	}
	else if (size == 256256)
	{
		geometry->tracks = 77;
		geometry->sectors = 26;
		geometry->heads = 1;
	}
	else if (size == 2 * 256256)
	{
		geometry->tracks = 77;
		geometry->sectors = 26;
		geometry->heads = 2;
	}
	else
	{
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t mdos_dsk_post_format(floppy_image_legacy *floppy, util::option_resolution *params)
{
	int heads, tracks, sectors, total_sectors;
	struct disk_id_sector info;
	time_t t;
	struct tm *ltime;
	floperr_t err;

	heads   = params->lookup_int(PARAM_HEADS);
	tracks  = params->lookup_int(PARAM_TRACKS);
	sectors = params->lookup_int(PARAM_SECTORS);
	total_sectors = heads * tracks * sectors;

	if (total_sectors < 3)
		return FLOPPY_ERROR_INVALIDIMAGE;

	memset(&info, 0, sizeof(info));

	memset(info.id, 0x20, sizeof(info.id));
	info.id[0] = 'M';
	info.id[1] = 'D';
	info.id[2] = 'O';
	info.id[3] = 'S';

	memset(info.version, 0x20, sizeof(info.version));
	info.version[1] = '2';

	memset(info.revision, '0', sizeof(info.revision));

	time(&t);
	ltime = localtime(&t);
	info.date[0] = (ltime->tm_mon + 1) / 10 + '0';
	info.date[1] = (ltime->tm_mon + 1) % 10 + '0';
	info.date[2] = ltime->tm_mday / 10 + '0';
	info.date[3] = ltime->tm_mday % 10 + '0';
	info.date[4] = (ltime->tm_year % 100) / 10 + '0';
	info.date[5] = ltime->tm_year % 10 + '0';

	memset(info.username, 0x20, sizeof(info.username));

	err = floppy_write_sector(floppy, 0, 0, 1, 0, &info, 128, 0);
	if (err)
		return err;

	uint8_t alloc[128];
	memset(alloc, 0xff, sizeof(alloc));

	for (int cluster = 0; cluster < 128 * 8; cluster++) {
		uint8_t mask = 1 << (7 - (cluster & 7));
		uint8_t offset = cluster >> 3;
		uint8_t bit = (cluster * 4 + 4 > total_sectors) ? mask : 0;
		alloc[offset] |= bit;
	}

	err = floppy_write_sector(floppy, 0, 0, 2, 0, alloc, 128, 0);
	if (err)
		return err;

	err = floppy_write_sector(floppy, 0, 0, 3, 0, alloc, 128, 0);
	if (err)
		return err;

	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(mdos_dsk_identify)
{
	struct basicdsk_geometry geometry;
	*vote = mdos_dsk_readheader(floppy, &geometry) ? 0 : 100;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(mdos_dsk_construct)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads              = params->lookup_int(PARAM_HEADS);
		geometry.tracks             = params->lookup_int(PARAM_TRACKS);
		geometry.sectors            = params->lookup_int(PARAM_SECTORS);
		geometry.first_sector_id    = params->lookup_int(PARAM_FIRST_SECTOR_ID);
		geometry.sector_length      = params->lookup_int(PARAM_SECTOR_LENGTH);
	}
	else
	{
		/* open */
		err = mdos_dsk_readheader(floppy, &geometry);
		if (err)
			return err;
	}

	/* actually construct the image */
	err = basicdsk_construct(floppy, &geometry);
	floppy_callbacks(floppy)->post_format = mdos_dsk_post_format;

	return err;
}

LEGACY_FLOPPY_OPTIONS_START( mdos )
	LEGACY_FLOPPY_OPTION( mdos_dsk, "dsk", "Motorola MDOS disk image", mdos_dsk_identify,  mdos_dsk_construct, nullptr,
		HEADS([1]-2)
		TRACKS([40]-255)
		SECTORS(1-[16]-255)
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION( imd, "imd", "IMD floppy disk image",  imd_dsk_identify, imd_dsk_construct, nullptr, nullptr) \
LEGACY_FLOPPY_OPTIONS_END0

/* ----------------------------------------------------------------------- */

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

int mdos_format::identify(io_generic *io, uint32_t form_factor)
{
	int type = find_size(io, form_factor);

	if (type != -1)
		return 75;
	return 0;
}

int mdos_format::find_size(io_generic *io, uint32_t form_factor)
{
	struct disk_id_sector info;
	uint64_t size = io_generic_size(io);

	if (size < 3 * 128 || size % 128 != 0)
		return -1;

	// Look at the disk id sector.
	io_generic_read(io, &info, 0, sizeof(struct disk_id_sector));

	if (!check_id_sector(&info, size))
		return -1;

	uint8_t cluster_allocation[128], cluster_available[128];
	io_generic_read(io, &cluster_allocation, 1 * 128, sizeof(cluster_allocation));
	io_generic_read(io, &cluster_available, 2 * 128, sizeof(cluster_available));

	if (!check_alloc_tables(cluster_allocation, cluster_available, size))
		return -1;

	for (int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];

		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		unsigned int format_size = 0;
		for (int track=0; track < f.track_count; track++) {
			for (int head=0; head < f.head_count; head++) {
				const format &tf = get_track_format(f, head, track);
				format_size += compute_track_size(tf);
			}
		}

		if (format_size != size)
			continue;

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
	{ // 80K 5 1/4 inch single sided single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 40, 1, 128, {}, -1, {1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16}, 80, 12, 27
	},
	{ // 80K 5 1/4 inch double sided single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 40, 2, 128, {}, -1, {1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16}, 80, 12, 27
	},
	{ // 250.25K 8 inch single sided single density
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2400, 26, 77, 1, 128, {}, 1, {}, 32-6, 17-6, 33-6
	},
	{ // 500.5K 8 inch double sided single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2400, 26, 77, 2, 128, {}, 1, {}, 32-6, 17-6, 33-6
	},
	{}
};

const mdos_format::format mdos_format::formats_head1[] = {
	{
	},
	{ // 80K 5 1/4 inch double sided single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 40, 2, 128, {}, -1, {17,25,18,26,19,27,20,28,21,29,22,30,23,31,24,32}, 80, 12, 27
	},
	{ // 250.25K 8 inch single sided single density
	},
	{ // 500.5K 8 inch double sided single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2400, 26, 77, 2, 128, {}, 27, {}, 32-6, 17-6, 33-6
	},
	{}
};

const floppy_format_type FLOPPY_MDOS_FORMAT = &floppy_image_format_creator<mdos_format>;
