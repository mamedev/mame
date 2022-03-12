// license:BSD-3-Clause
// copyright-holders:tim lindner, 68bit
/*********************************************************************

    formats/os9_dsk.c

    OS-9 disk images

    OS-9 Level 2 Technical Reference, Chapter 5, Random Block File
    Manager, page 2

    Available: http://www.colorcomputerarchive.com/coco/Documents/Manuals/
        Operating Systems/OS-9 Level 2 Manual (Tandy).pdf

    Some OS9 floppy drivers use a base sector ID of zero, for example SWTPC
    and Gimix OS9, and some drivers use a base sector ID of one, for example
    COCO OS9. The disk images do not encode the sector IDs so it is a
    challenge to detect this base ID by looking at the disk content.

    The OS9 disk header, at LSN0, does not necessarily encode the base sector
    ID either. However it may optionally include a block of the device
    descriptor information from when it was formatted and although this
    optional content can vary across OS9 variants it appears to be regular
    enough for COCO compatible formats to reply on it to identify a COCO
    compatible disk and determine the base sector ID for these. All non-COCO
    formats are assumed to have a base sector ID of zero and not exceptions to
    this have been noted.

    Commonly the 'coco' bit, bit 5, in the optional 'type' byte is set in COCO
    format disks. Alternatively some OS9 formats set bit 7 of the optional
    'density' byte to indicate a COCO format disk, for example some Gimix
    drivers with COCO support. Some of the other optional information also can
    be checked for consistency to better identify the format.

    Some formats encode the first track, on the first head only, in single
    density even when the remainder of the disk is encoded in double density,
    and this can typically be identified from the disk geometry and the image
    size and the total number of logical sectors written in the header.
    E.g. At least some versions of OS9 for the SWTPC and the Gimix encode this
    first track in single density whereas COCO format disks are typically all
    double density. The number of sectors on track zero is also typically
    encoded in the optional header information and can be checked for
    consistency.

*********************************************************************/

#include "os9_dsk.h"
#include "imageutl.h"

#include "coretmpl.h" // BIT
#include "ioprocs.h"


os9_format::os9_format() : wd177x_format(formats)
{
}

const char *os9_format::name() const
{
	return "os9";
}

const char *os9_format::description() const
{
	return "OS-9 floppy disk image";
}

const char *os9_format::extensions() const
{
	return "os9,dsk";
}

int os9_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int const type = find_size(io, form_factor, variants);

	if (type != -1)
		return 75;

	return 0;
}

int os9_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint64_t size;
	if (io.length(size))
		return -1;

	uint8_t os9_header[0x60];
	size_t actual;
	io.read_at(0, os9_header, sizeof(os9_header), actual);

	int os9_total_sectors = pick_integer_be(os9_header, 0x00, 3);
	int os9_heads = util::BIT(os9_header[0x10], 0) ? 2 : 1;
	int os9_sectors = pick_integer_be(os9_header, 0x11, 2);

	if (os9_total_sectors <= 0 || os9_heads <= 0 || os9_sectors <= 0)
		return -1;

	// The optional header information, which may contain a copy of the
	// device descriptor used to format the disk. COCO format disks are
	// expected to include this optional information.
	int opt_dtype = os9_header[0x3f + 0];
	int opt_type = os9_header[0x3f + 3];
	int opt_density = os9_header[0x3f + 4];
	int opt_cylinders = pick_integer_be(os9_header, 0x3f + 5, 2);
	int opt_sides = os9_header[0x3f + 7];
	int opt_sectors_per_track = pick_integer_be(os9_header, 0x3f + 9, 2);
	int opt_track0_sectors = pick_integer_be(os9_header, 0x3f + 11, 2);
	int opt_interleave = os9_header[0x3f + 13];

	int opt_mfm = util::BIT(opt_density, 0);

	// The NitrOS9 rb1773 driver uses bit 1 of opt_type to distinguish
	// between a sector base ID of zero or one, so recognise that here.
	int opt_sector_base_id = util::BIT(opt_type, 1) ? 0 : 1;
	int opt_sector_size = util::BIT(opt_type, 2) ? 512 : 256;
	int opt_coco = util::BIT(opt_type, 5);

	// Some OS9 versions appear to use bit 7 of the opt_density rather
	// than bit 5 of opt_type to signify a COCO format disk. E.g. Gimix
	// OS9 is documented to use this bit and had a floppy driver that
	// could read both non-COCO and COCO format disks.
	if (util::BIT(opt_density, 7))
		opt_coco = 1;

	// COCO format disks are expected for have an opt_dtype of 1.
	if (opt_dtype != 1)
		opt_coco = 0;

	for (int i=0; formats[i].form_factor; i++) {
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		if ((os9_heads != f.head_count) || (os9_sectors != f.sector_count)) {
			continue;
		}

		unsigned int format_size = 0;
		for (int track=0; track < f.track_count; track++) {
			for (int head=0; head < f.head_count; head++) {
				const format &tf = get_track_format(f, head, track);
				format_size += compute_track_size(tf);
			}
		}

		if (format_size != size)
			continue;

		if (format_size < os9_total_sectors * 256)
			continue;

		// Accept the format if the total number of sectors used is
		// consistent with at least 10 sectors being used on track
		// zero, as happens when the driver or descriptor is coded for
		// a single density track zero with 10 sectors and so uses
		// only 10 of these track zero sectors.

		if (f.sector_count < 10 || format_size > (os9_total_sectors + f.sector_count - 10) * 256)
			continue;

		const format &tf = get_track_format(f, 0, 0);

		if (opt_coco) {
			// Looks like a COCO format disk.

			// Check the sector base ID.
			if (f.sector_base_id != opt_sector_base_id &&
				(f.sector_base_id != -1 || f.per_sector_id[0] != opt_sector_base_id))
				continue;
			if (tf.sector_base_id != opt_sector_base_id &&
				(tf.sector_base_id != -1 || tf.per_sector_id[0] != opt_sector_base_id))
				continue;

			// Check some other optional fields for consistency.

			// If the device descriptor is not MFM capable then
			// the disk it not expected to be MFM encoded.
			if (opt_mfm == 0 && f.encoding == floppy_image::MFM)
				continue;

			// Should not be more cylinders than suppored in the
			// device descriptor.
			if (f.track_count > opt_cylinders)
				continue;

			// Should not be more heads than suppored in the
			// device descriptor.
			if (f.head_count > opt_sides)
				continue;

			// Should not be more sectors per track than supported
			// in the device descriptor.
			if (f.sector_count > opt_sectors_per_track ||
				tf.sector_count > opt_track0_sectors) {
				continue;
			}

			// The sector size should be consistent.
			if (opt_sector_size != f.sector_base_size)
				continue;

			// The sector interleave should not be greater than
			// the number of sectors.
			if (opt_interleave > f.sector_count)
				continue;
		}
		else
		{
			// Not a recognizable COCO OS9 format.  The options
			// can not be relied on here, they are driver
			// dependent, and the sector base ID is assumed to be
			// zero. Check that format sector base ID is zero.
			if (f.sector_base_id != 0 && (f.sector_base_id != -1 || f.per_sector_id[0] != 0))
				continue;
			if (tf.sector_base_id != 0 && (tf.sector_base_id != -1 || tf.per_sector_id[0] != 0))
				continue;
		}

		LOG_FORMATS("os9_dsk: matching format index %d: tracks %d, sectors %d, sides: %d\n", i, f.track_count, f.sector_count, f.head_count);
		return i;
	}
	return -1;
}

const wd177x_format::format &os9_format::get_track_format(const format &f, int head, int track)
{
	int n = -1;

	for (int i = 0; formats[i].form_factor; i++) {
		if (&formats[i] == &f) {
			n = i;
			break;
		}
	}

	if (n < 0) {
		LOG_FORMATS("os9_dsk: Error format not found\n");
		return f;
	}

	if (head >= f.head_count) {
		LOG_FORMATS("os9_dsk: Error invalid head %d\n", head);
		return f;
	}

	if (track >= f.track_count) {
		LOG_FORMATS("os9_dsk: Error invalid track %d\n", track);
		return f;
	}

	if (track == 0 && head == 0) {
		const format &t0 = formats_track0[n];
		if (t0.form_factor)
			return t0;
	}

	return f;
}

const os9_format::format os9_format::formats[] = {
	// COCO formats, with a sector base ID of one.

	{ // 0 157.5K 5"25 double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 35, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{ // 1 315K 5"25 double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 35, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{ // 2 180K 5"25 double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{ // 3 360K 5"25 double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{ // 4 360K 5"25 double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},
	{ // 5 720K 5"25 double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, -1, {1, 7, 13, 2, 8, 14, 3, 9, 15, 4, 10, 16, 5, 11, 17, 6, 12, 18}, 18, 28, 20
	},

	// Non-COCO formats, with a sector base ID of zero.

	{ // 6 87.5K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 7 100K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 8 200K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 9 175K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 35, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 10 200K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 11 400K 5 1/4 inch single density
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 12 138.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 35, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 13 140K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 35, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 14 278.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 35, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 15 280K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 35, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 16 158.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 17 160K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 18 318.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 19 320K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 20 318.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 21 320K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 22 638.5 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 23 640K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, {0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13}, 18, 28, 20
	},
	{ // 24 155.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 35, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 25 157.5K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 35, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 26 311K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 35, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 27 315K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 35, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 28 178K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 29 180K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 30 356K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 31 360K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 32 358K 5 1/4 inch quad density (single density track 0)
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 33 360K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::MFM,
		2000, 18, 80, 1, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 34 716K 5 1/4 inch quad density (single density track 0)
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 35 720K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD, floppy_image::MFM,
		2000, 18, 80, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 80, 22, 24
	},
	{ // 36 288.75K 8 inch single density
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2000, 15, 77, 1, 256, {}, -1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13}, 40, 12, 12
	},
	{ // 37 577.5K 8 inch single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2000, 15, 77, 2, 256, {}, -1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13}, 40, 12, 12
	},
	{ // 38 308K 8 inch single density
		floppy_image::FF_8, floppy_image::SSSD, floppy_image::FM,
		2000, 16, 77, 1, 256, {}, -1, {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15}, 35, 12, 12
	},
	{ // 39 616K 8 inch single density
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2000, 16, 77, 2, 256, {}, -1, {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15}, 35, 12, 12
	},
	{ // 40 497.75K 8 inch double density (single density track 0)
		floppy_image::FF_8, floppy_image::SSDD, floppy_image::MFM,
		1000, 26, 77, 1, 256, {}, -1, {0, 19, 12, 5, 24, 17, 10, 3, 22, 15, 8, 1, 20, 13, 6, 25, 18, 11, 4, 23, 16, 9, 2, 21, 14, 7}, 80, 22, 24
	},
	{ // 41 500.5K 8 inch double density
		floppy_image::FF_8, floppy_image::SSDD, floppy_image::MFM,
		1000, 26, 77, 1, 256, {}, -1, {0, 19, 12, 5, 24, 17, 10, 3, 22, 15, 8, 1, 20, 13, 6, 25, 18, 11, 4, 23, 16, 9, 2, 21, 14, 7}, 80, 22, 24
	},
	{ // 42 995.5K 8 inch double density (single density track 0)
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		1000, 26, 77, 2, 256, {}, -1, {0, 19, 12, 5, 24, 17, 10, 3, 22, 15, 8, 1, 20, 13, 6, 25, 18, 11, 4, 23, 16, 9, 2, 21, 14, 7}, 80, 22, 24
	},
	{ // 43 1001K 8 inch double density
		floppy_image::FF_8, floppy_image::DSDD, floppy_image::MFM,
		1000, 26, 77, 2, 256, {}, -1, {0, 19, 12, 5, 24, 17, 10, 3, 22, 15, 8, 1, 20, 13, 6, 25, 18, 11, 4, 23, 16, 9, 2, 21, 14, 7}, 80, 22, 24
	},
	{ // 44 1440K 3 1/2 inch high density (single density track 0)
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 36, 80, 2, 256, {}, -1, {0, 25, 14, 3, 28, 17, 6, 31, 20, 9, 34, 23, 12, 1, 26, 15, 4, 29, 18, 7, 32, 21, 10, 35, 24, 13, 2, 27, 16, 5, 30, 19, 8, 33, 22, 11}, 80, 22, 24
	},
	{ // 45 1440K 3 1/2 inch high density.
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 36, 80, 2, 256, {}, -1, {0, 25, 14, 3, 28, 17, 6, 31, 20, 9, 34, 23, 12, 1, 26, 15, 4, 29, 18, 7, 32, 21, 10, 35, 24, 13, 2, 27, 16, 5, 30, 19, 8, 33, 22, 11}, 80, 22, 24
	},
	{}
};

const os9_format::format os9_format::formats_track0[] = {
	// COCO formats, with a sector base ID of one.

	{ // 0 157.5K 5"25 double density
	},
	{ // 1 315K 5"25 double density
	},
	{ // 2 180K 5"25 double density
	},
	{ // 3 360K 5"25 double density
	},
	{ // 4 360K 5"25 double density
	},
	{ // 5 720K 5"25 double density
	},

	// Non-COCO formats, with a sector base ID of zero.

	{ // 6 87.5K 5 1/4 inch single density
	},
	{ // 7 100K 5 1/4 inch single density
	},
	{ // 8 200K 5 1/4 inch single density
	},
	{ // 9 175K 5 1/4 inch single density
	},
	{ // 10 200K 5 1/4 inch single density
	},
	{ // 11 400K 5 1/4 inch single density
	},
	{ // 12 138.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 13 140K 5 1/4 inch double density
	},
	{ // 14 278.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 15 280K 5 1/4 inch double density
	},
	{ // 16 158.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 17 160K 5 1/4 inch double density
	},
	{ // 18 318.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 19 320K 5 1/4 inch double density
	},
	{ // 20 318.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 21 320K 5 1/4 inch double density
	},
	{ // 22 638.5 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 23 640K 5 1/4 inch double density
	},
	{ // 24 155.5K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 25 157.5K 5 1/4 inch double density
	},
	{ // 26 311K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 27 315K 5 1/4 inch double density
	},
	{ // 28 178K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 29 180K 5 1/4 inch double density
	},
	{ // 30 356K 5 1/4 inch double density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 31 360K 5 1/4 inch double density
	},
	{ // 32 358K 5 1/4 inch quad density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 33 360K 5 1/4 inch quad density
	},
	{ // 34 716K 5 1/4 inch quad density (single density track 0)
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 80, 2, 256, {}, -1, {0, 3, 6, 9, 2, 5, 8, 1, 4, 7}, 40, 16, 11
	},
	{ // 35 720K 5 1/4 inch quad density
	},
	{ // 36 288.75K 8 inch single density
	},
	{ // 37 577.5K 8 inch single density
	},
	{ // 38 308K 8 inch single density
	},
	{ // 39 616K 8 inch single density
	},
	{ // 40 497.75K 8 inch double density (single density track 0)
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2000, 15, 77, 1, 256, {}, -1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13}, 40, 12, 12
	},
	{ // 41 500.5K 8 inch double density
	},
	{ // 42 995.5K 8 inch double density (single density track 0)
		floppy_image::FF_8, floppy_image::DSSD, floppy_image::FM,
		2000, 15, 77, 2, 256, {}, -1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13}, 40, 12, 12
	},
	{ // 43 1001K 8 inch double density
	},
	{ // 44 1440K 3 1/2 inch high density (single density track 0)
		floppy_image::FF_35,  floppy_image::DSSD, floppy_image::FM,
		2000, 18, 80, 2, 256, {}, -1, {0, 5, 10, 15, 2, 7, 12, 17, 4, 9, 14, 1, 6, 11, 16, 3, 8, 13}, 40, 12, 12
	},
	{ // 45 1440K 3 1/2 inch high density.
	},
	{}
};

const floppy_format_type FLOPPY_OS9_FORMAT = &floppy_image_format_creator<os9_format>;
