// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/trd_dsk.cpp

    TRD disk images

*********************************************************************/

#include "formats/trd_dsk.h"

#include "ioprocs.h"

#include <tuple>


trd_format::trd_format() : wd177x_format(formats)
{
}

const char *trd_format::name() const noexcept
{
	return "trd";
}

const char *trd_format::description() const noexcept
{
	return "TRD floppy disk image";
}

const char *trd_format::extensions() const noexcept
{
	return "trd";
}

int trd_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	int index = -1;
	for (int i = 0; formats[i].form_factor; i++)
	{
		const format &f = formats[i];
		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
			continue;

		uint64_t format_size = f.track_count * f.head_count * compute_track_size(f);

		if (size == format_size)
		{
			index = i; // at least size match, save it for the case if there will be no exact matches

			std::error_condition err;
			size_t actual;
			uint8_t sectdata[0x100];
			if (f.encoding == floppy_image::MFM)
				std::tie(err, actual) = read_at(io, 0x800, sectdata, 0x100);
			else
			{
				std::tie(err, actual) = read_at(io, 0x100, sectdata, 0x100);
				for (int i = 0; i < 0x100; i++)
					sectdata[i] ^= 0xff;
			}
			if (err || (0x100 != actual))
				continue;

			uint8_t disktype = sectdata[0xe3]; // 16 - DS80, 17 - DS40, 18 - SS80, 19 - SS40
			if (disktype < 0x16 || disktype > 0x19)
				continue;

			uint8_t heads = ((disktype == 0x16) || disktype == 0x17) ? 2 : 1;
			uint8_t tracks = ((disktype == 0x16) || disktype == 0x18) ? 80 : 40;

			if (f.sector_count != sectdata[0xe7] || f.head_count != heads || f.track_count != tracks)
				continue;

			return i;
		}
	}
	return index;
}

const trd_format::format trd_format::formats[] = {
	// GAP4A should be 0 bytes
	{   // 5"25 640K 80 track double sided double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},
	{   // 5"25 320K 80 track single sided double density
		floppy_image::FF_525,  floppy_image::SSQD, floppy_image::MFM,
		2000, 16, 80, 1, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},
	{   // 5"25 320K 40 track double sided double density
		floppy_image::FF_525,  floppy_image::DSSD, floppy_image::MFM,
		2000, 16, 40, 2, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},
	{   // 5"25 160K 40 track single sided double density
		floppy_image::FF_525,  floppy_image::SSSD, floppy_image::MFM,
		2000, 16, 40, 1, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},

	{   // 5"25 400K 80 track double sided single density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::FM,
		// GAPs and interleave probably not correct
		4000, 10, 80, 2, 256, {}, 1, {}, 40, 10, 10
	},
	{   // 5"25 280K 80 track double sided single density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::FM,
		// GAPs and interleave probably not correct
		4000,  7, 80, 2, 256, {}, 1, {}, 40, 10, 10
	},
	{}
};

const trd_format FLOPPY_TRD_FORMAT;
