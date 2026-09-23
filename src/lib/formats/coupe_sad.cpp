// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe SAD Disk Image

***************************************************************************/

#include "coupe_sad.h"
#include "ioprocs.h"
#include <cstring>

coupe_sad_format::coupe_sad_format() : wd177x_format(formats)
{
}

const char *coupe_sad_format::name() const noexcept
{
	return "coupe_sad";
}

const char *coupe_sad_format::description() const noexcept
{
	return "SAM Coupe SAD disk image";
}

const char *coupe_sad_format::extensions() const noexcept
{
	return "sad,dsk";
}

bool coupe_sad_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	std::vector<int> candidates;

	for (int i = 0; formats[i].form_factor; i++)
		candidates.push_back(i);

	check_compatibility(image, candidates);

	// exactly one format should match
	if (candidates.size() != 1)
		return false;

	// save disk data
	if (!wd177x_format::save(io, variants, image))
		return false;

	// add header
	const format &f = formats[candidates[0]];
	uint8_t header[HEADER_SIZE];

	memcpy(header, "Aley's disk backup", 18);
	header[18] = f.head_count;
	header[19] = f.track_count;
	header[20] = f.sector_count;
	header[21] = f.sector_base_size >> 6;

	auto const [err, actual] = write_at(io, 0, header, sizeof(header));
	return !err && actual == sizeof(header);
}

int coupe_sad_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[HEADER_SIZE];

	auto const [err, actual] = read_at(io, 0, header, sizeof(header));
	if (err || actual != sizeof(header))
		return -1;

	if (memcmp(header, "Aley's disk backup", 18))
		return -1;

	const int heads       = header[18];
	const int tracks      = header[19];
	const int sectors     = header[20];
	const int sector_size = header[21] << 6;

	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];

		if (f.head_count == heads &&
			f.track_count == tracks &&
			f.sector_count == sectors &&
			f.sector_base_size == sector_size)
		{
			uint64_t size;

			if (!io.length(size) && size == HEADER_SIZE + uint64_t(heads) * tracks * sectors * sector_size)
				return i;
		}
	}

	return -1;
}

int coupe_sad_format::get_image_offset(const format &f, int head, int track) const
{
	return HEADER_SIZE + (head * f.track_count + track) * compute_track_size(f);
}

const coupe_sad_format::format coupe_sad_format::formats[] =
{
	{   //  800k 3.5 inch double sided double density (standard)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 60, 22, 24
	},
	{   //  820k 3.5 inch double sided double density (82 tracks)
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 82, 2, 512, {}, 1, {}, 60, 22, 24
	},
	{}
};

const coupe_sad_format FLOPPY_COUPE_SAD_FORMAT;
