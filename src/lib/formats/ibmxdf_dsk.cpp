// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/ibmxdf_dsk.c

    IBM Extended Density Format

    References:
    - http://www.os2museum.com/wp/the-xdf-diskette-format/
    - https://fdutils.linux.lu/Fdutils.html#SEC47 and xdfcopy.c

    Mapping XDF disk image to physical sectors on a 3.5" disk:

    First track uses standard 512-byte sectors:
    - lbn 0 (boot sector) = 0.129 (head.sector)
    - lbn 1-11 (FAT) = 0.130-0.139, then 1.129
    - lbn 12-19 (aux FS) = 0.1-0.8
    - lbn 20-22 = padding, not written to disk
    - lbn 23-36 (root directory) = 1.130-1.143
    - lbn 37-41 = padding, not written to disk
    - lbn 42-45 (start of data area) = 1.144-1.147

    All other tracks use mixed size sectors, sequenced like so:
    0.131, 0.132, 1.134, 0.130, 1.130, 0.134, 1.132, 1.131

    To do:
    - 5.25 HD and 3.5 ED formats
    - replace magic numbers

*********************************************************************/

#include "ibmxdf_dsk.h"

#include "ioprocs.h"


ibmxdf_format::ibmxdf_format() : wd177x_format(formats)
{
}

const char *ibmxdf_format::name() const
{
	return "ibmxdf";
}

const char *ibmxdf_format::description() const
{
	return "IBM XDF disk image";
}

const char *ibmxdf_format::extensions() const
{
	return "xdf,img";
}

int ibmxdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return 75;
	return 0;
}

int ibmxdf_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint64_t size;
	if (io.length(size))
		return -1;

	if (size != 1884160)
		return -1;

	return 0;
}

int ibmxdf_format::get_image_offset(const format &f, int head, int track)
{
	return (2 * track) * compute_track_size(formats[0]);
}

const wd177x_format::format &ibmxdf_format::get_track_format(const format &f, int head, int track)
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

	if (track > 0) {
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

	// Track 0

	if (head == 1) {
		const format &fh1t0 = formats_head1_track0[n];
		if (fh1t0.form_factor) {
			return fh1t0;
		}
		const format &fh1 = formats_head1[n];
		if (fh1.form_factor) {
			return fh1;
		}
		LOG_FORMATS("Error expected a head 1 format\n");
		return f;
	}

	// Head 0

	const format &ft0 = formats_track0[n];
	if (ft0.form_factor) {
		return ft0;
	}

	return f;
}


// Unverified gap sizes
const ibmxdf_format::format ibmxdf_format::formats[] = {
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 4, 80, 2, 0, { 1024, 512, 2048, 8192 }, -1, { 131, 130, 132, 134 }, 50, 22, 74
	},
	{}
};

const ibmxdf_format::format ibmxdf_format::formats_head1[] = {
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 4, 80, 2, 0, { 2048, 512, 1024, 8192 }, -1, { 132, 130, 131, 134 }, 50, 22, 74
	},
	{}
};

const ibmxdf_format::format ibmxdf_format::formats_track0[] = {
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 19, 80, 2, 512, {}, -1, { 1, 138, 129, 139, 130, 2, 131, 3, 132, 4, 133, 5, 134, 6, 135, 7, 136, 8, 137 }, 50, 22, 74
	},
	{}
};

const ibmxdf_format::format ibmxdf_format::formats_head1_track0[] = {
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 19, 80, 2, 512, {}, -1, { 144, 135, 145, 136, 146, 137, 147, 138, 129, 139, 130, 140, 131, 141, 132, 142, 133, 143, 134 }, 50, 22, 74
	},
	{}
};

bool ibmxdf_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	int type = find_size(io, form_factor, variants);
	if(type == -1)
		return false;

	const format &f = formats[type];

	for(int track=0; track < f.track_count; track++)
		for(int head=0; head < f.head_count; head++) {
			uint8_t sectdata[23 * 2 * 512]; // XXX magic
			desc_s sectors[40];
			floppy_image_format_t::desc_e *desc;
			int current_size;
			int end_gap_index;
			const format &tf = get_track_format(f, head, track);

			desc = get_desc_mfm(tf, current_size, end_gap_index);

			int total_size = 200000000/tf.cell_size;
			int remaining_size = total_size - current_size;
			if(remaining_size < 0) {
				osd_printf_error("ibmxdf_format: Incorrect track layout, max_size=%d, current_size=%d\n", total_size, current_size);
				return false;
			}

			// Fixup the end gap
			desc[end_gap_index].p2 = remaining_size / 16;
			desc[end_gap_index + 1].p2 = remaining_size & 15;
			desc[end_gap_index + 1].p1 >>= 16-(remaining_size & 15);

			desc[16].p1 = get_track_dam_mfm(tf, head, track);

			build_sector_description(tf, sectdata, sectors, track, head);
			int const track_size = compute_track_size(f) * 2; // read both sides at once
			size_t actual;
			io.read_at(get_image_offset(f, head, track), sectdata, track_size, actual);
			generate_track(desc, track, head, sectors, tf.sector_count, total_size, image);
		}

	image->set_variant(f.variant);

	return true;
}

static const int offsets[2][4] = { { 0, 11264, 1024, 12288 }, { 20480, 11776, 22528, 3072 } }; // XXX magic

void ibmxdf_format::build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const
{
	switch (track)
	{
	case 0:
		if (head == 0) {
			for(int i=0; i<f.sector_count; i++) {
				if (f.per_sector_id[i] < 129) // aux fs
					sectors[i].data = sectdata + f.sector_base_size * (f.per_sector_id[i] + 11);
				else
					sectors[i].data = sectdata + f.sector_base_size * (f.per_sector_id[i] - 129);
				sectors[i].size = f.sector_base_size;
				sectors[i].sector_id = f.per_sector_id[i];
			}
		}
		else {
			for(int i=0; i<f.sector_count; i++) {
				if (f.per_sector_id[i] == 129)
					sectors[i].data = sectdata + f.sector_base_size * 11;
				else if (f.per_sector_id[i] < 144)
					sectors[i].data = sectdata + f.sector_base_size * (f.per_sector_id[i] - 130 + 23);
				else // skip 5 sectors marked bad in the FAT
					sectors[i].data = sectdata + f.sector_base_size * (f.per_sector_id[i] - 130 + 28);
				sectors[i].size = f.sector_base_size;
				sectors[i].sector_id = f.per_sector_id[i];
			}
		}
		break;

	default:
		for(int i=0; i<f.sector_count; i++) {
			sectors[i].data = sectdata + offsets[head][i];
			sectors[i].size = f.per_sector_size[i];
			sectors[i].sector_id = f.per_sector_id[i];
		}
		break;
	}
}

const floppy_format_type FLOPPY_IBMXDF_FORMAT = &floppy_image_format_creator<ibmxdf_format>;

