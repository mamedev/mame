// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, tim lindner
/*********************************************************************

    formats/sdf_dsk.cpp

    SDF disk images. Format created by Darren Atkinson for use with
    his CoCoSDC floppy disk emulator.

    http://cocosdc.blogspot.com/p/sd-card-socket-sd-card-socket-is-push.html

*********************************************************************/

#include "sdf_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"


sdf_format::sdf_format()
{
}


const char *sdf_format::name() const noexcept
{
	return "sdf";
}


const char *sdf_format::description() const noexcept
{
	return "SDF disk image";
}


const char *sdf_format::extensions() const noexcept
{
	return "sdf";
}


int sdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[HEADER_SIZE];

	uint64_t size;
	if (io.length(size) || (size < HEADER_SIZE))
	{
		return 0;
	}

	auto const [err, actual] = read_at(io, 0, header, HEADER_SIZE);
	if (err || (HEADER_SIZE != actual))
	{
		return 0;
	}

	int tracks = header[4];
	int heads = header[5];

	// Check magic bytes
	if (header[0] != 'S' || header[1] != 'D' || header[2] != 'F' || header[3] != '1')
	{
		return 0;
	}

	// Check heads
	if (heads != 1 && heads != 2)
	{
		return 0;
	}

	if (size == HEADER_SIZE + heads * tracks * TOTAL_TRACK_SIZE)
	{
		return FIFID_SIGN|FIFID_SIZE|FIFID_STRUCT;
	}

	return 0;
}


bool sdf_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t header[HEADER_SIZE];
	std::vector<uint8_t> track_data(TOTAL_TRACK_SIZE);
	std::vector<uint32_t> raw_track_data;

	read_at(io, 0, header, HEADER_SIZE); // FIXME: check for errors and premature EOF

	const int tracks = header[4];
	const int heads = header[5];

	if (heads == 2)
	{
		image.set_variant(floppy_image::DSDD);
	}
	else
	{
		image.set_variant(floppy_image::SSDD);
	}

	for (int track = 0; track < tracks; track++)
	{
		for (int head = 0; head < heads; head++)
		{
			int iam_location = -1;
			int idam_location[SECTOR_SLOT_COUNT+1];
			int dam_location[SECTOR_SLOT_COUNT+1];
			raw_track_data.clear();

			// Read track
			read_at(io, HEADER_SIZE + (heads * track + head) * TOTAL_TRACK_SIZE, &track_data[0], TOTAL_TRACK_SIZE); // FIXME: check for errors and premature EOF

			int sector_count = track_data[0];

			if (sector_count > SECTOR_SLOT_COUNT) return false;

			// Transfer IDAM and DAM locations to table
			for (int i = 0; i < SECTOR_SLOT_COUNT+1; i++)
			{
				if (i < sector_count )
				{
					idam_location[i] = (get_u16le(&track_data[ 8 * (i+1)]) & 0x3FFF) - 4;
					dam_location[i] = (get_u16le(&track_data[ 8 * (i+1) + 2]) & 0x3FFF) - 4;

					if (idam_location[i] > TOTAL_TRACK_SIZE) return false;
					if (dam_location[i] > TOTAL_TRACK_SIZE) return false;
				}
				else
				{
					idam_location[i] = -1;
					dam_location[i] = -1;
				}
			}

			// Find IAM location
			for (int i = idam_location[0] - 1; i >= TRACK_HEADER_SIZE + 3; i--)
			{
				// It's usually 3 bytes but several dumped tracks seem to contain only 2 bytes
				if (track_data[i] == 0xfc && track_data[i-1] == 0xc2 && track_data[i-2] == 0xc2)
				{
					iam_location = i - 3;
					break;
				}
			}

			int idam_index = 0;
			int dam_index = 0;
			for (int offset = TRACK_HEADER_SIZE; offset < TRACK_HEADER_SIZE + TRACK_SIZE; offset++)
			{
				if (offset == iam_location)
				{
					// Write IAM
					raw_w(raw_track_data, 16, 0x5224);
					raw_w(raw_track_data, 16, 0x5224);
					raw_w(raw_track_data, 16, 0x5224);
					offset += 3;
				}

				if (offset == idam_location[idam_index])
				{
					raw_w(raw_track_data, 16, 0x4489);
					raw_w(raw_track_data, 16, 0x4489);
					raw_w(raw_track_data, 16, 0x4489);
					idam_index += 1;
					offset += 3;
				}

				if (offset == dam_location[dam_index])
				{
					raw_w(raw_track_data, 16, 0x4489);
					raw_w(raw_track_data, 16, 0x4489);
					raw_w(raw_track_data, 16, 0x4489);
					dam_index += 1;
					offset += 3;
				}

				mfm_w(raw_track_data, 8, track_data[offset]);
			}

			generate_track_from_levels(track, head, raw_track_data, 0, image);
		}
	}

	return true;
}


const sdf_format FLOPPY_SDF_FORMAT;
