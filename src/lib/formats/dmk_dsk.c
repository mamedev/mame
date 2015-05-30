// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    formats/dmk_dsk.h

    DMK disk images

TODO:
- Add write/format support.
- Add single density support.
- Check support on other drivers besides msx.

*********************************************************************/

#include <assert.h>

#include "dmk_dsk.h"


dmk_format::dmk_format()
{
}


const char *dmk_format::name() const
{
	return "dmk";
}


const char *dmk_format::description() const
{
	return "DMK disk image";
}


const char *dmk_format::extensions() const
{
	return "dmk";
}


int dmk_format::identify(io_generic *io, UINT32 form_factor)
{
	const int header_size = 16;
	UINT8 header[header_size];

	UINT64 size = io_generic_size(io);

	io_generic_read(io, header, 0, header_size);

	int tracks = header[1];
	int track_size = ( header[3] << 8 ) | header[2];
	int heads = (header[4] & 0x10) ? 1 : 2;

	// The first header byte must be 00 or FF
	if (header[0] != 0x00 && header[0] != 0xff)
	{
		return 0;
	}

	// Bytes C-F must be zero
	if (header[0x0c] != 0 || header[0xd] != 0 || header[0xe] != 0 || header[0xf] != 0)
	{
		return 0;
	}

	// Check track size within limits
	if (track_size < 0x80 || track_size > 0x3FFF )
	{
		return 0;
	}

	if (size == header_size + heads * tracks * track_size)
	{
		return 70;
	}

	return 0;
}


bool dmk_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	const int header_size = 16;
	UINT8 header[header_size];

	io_generic_read(io, header, 0, header_size);

	const int tracks = header[1];
	const int track_size = ( header[3] << 8 ) | header[2];
	const int heads = (header[4] & 0x10) ? 1 : 2;
	const bool is_sd = (header[4] & 0x40) ? true : false;
	//const int raw_track_size = 2 * 8 * ( track_size - 0x80 );

	if (is_sd)
	{
		if (heads == 2)
		{
			image->set_variant(floppy_image::DSSD);
		}
		else
		{
			image->set_variant(floppy_image::SSSD);
		}
	}
	else
	{
		if (heads == 2)
		{
			image->set_variant(floppy_image::DSDD);
		}
		else
		{
			image->set_variant(floppy_image::SSDD);
		}
	}

	for (int track = 0; track < tracks; track++)
	{
		for (int head = 0; head < heads; head++)
		{
			std::vector<UINT8> track_data(track_size);
			std::vector<UINT32> raw_track_data;
			int iam_location = -1;
			int idam_location[64];
			int dam_location[64];

			// Read track
			io_generic_read(io, &track_data[0], header_size + ( heads * track + head ) * track_size, track_size);

			for (int i = 0; i < 64; i++)
			{
				idam_location[i] = -1;
				dam_location[i] = -1;
			}

			// Find IDAM locations
			UINT16 track_header_offset = 0;
			UINT16 track_offset = ( ( track_data[track_header_offset + 1] << 8 ) | track_data[track_header_offset] ) & 0x3fff;
			track_header_offset += 2;

			while ( track_offset != 0 && track_offset >= 0x83 && track_offset < track_size && track_header_offset < 0x80 )
			{
				// Assume 3 bytes before IDAM pointers are the start of IDAM indicators
				idam_location[(track_header_offset/2) - 1] = track_offset - 3;

				// Scan for DAM location
				for (int i = track_offset + 53; i > track_offset + 10; i--)
				{
					if ((track_data[i] == 0xfb || track_data[i] == 0xf8) && track_data[i-1] == 0xa1 && track_data[i-2] == 0xa1)
					{
						dam_location[(track_header_offset/2) - 1] = i - 3;
						break;
					}
				}

				track_offset = ( ( track_data[track_header_offset + 1] << 8 ) | track_data[track_header_offset] ) & 0x3fff;
				track_header_offset += 2;
			};

			// Find IAM location
			for(int i = idam_location[0] - 1; i >= 3; i--)
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
			for (int offset = 0x80; offset < track_size; offset++)
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


bool dmk_format::save(io_generic *io, floppy_image *image)
{
	return false;
}


bool dmk_format::supports_save() const
{
	return false;
}


const floppy_format_type FLOPPY_DMK_FORMAT = &floppy_image_format_creator<dmk_format>;
