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

#include "dmk_dsk.h"

#include "ioprocs.h"


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


int dmk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	const int header_size = 16;
	uint8_t header[header_size];
	size_t actual;
	io.read_at(0, header, header_size, actual);

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
		return FIFID_STRUCT|FIFID_SIZE;
	}

	return 0;
}


static uint32_t wide_fm(uint32_t val)
{
	uint32_t res = 0;
	for (int i = 15; i >= 0; i--) {
		if (val & (1 << i))
			res |= (1 << (i*2 + 1));
	}
	return res;
}
static uint32_t data_to_wide_fm(uint8_t val)
{
	uint16_t res = 0;
	for (int i = 7; i >= 0; i--) {
		res |= (1 << (i*2 + 1));	// clock
		if (val & (1 << i))
			res |= (1 << i*2);		// data
	}
	return wide_fm(res);
}

bool dmk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	size_t actual;

	const int header_size = 16;
	uint8_t header[header_size];
	io.read_at(0, header, header_size, actual);

	const int tracks = header[1];
	const int track_size = ( header[3] << 8 ) | header[2];
	const int heads = (header[4] & 0x10) ? 1 : 2;
	const bool is_sd = (header[4] & 0x40) ? true : false;
	//const int raw_track_size = 2 * 8 * ( track_size - 0x80 );

// Must see: if any MFM sectors present, must be DD
// is_sd really controls the stride
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
			std::vector<uint8_t> track_data(track_size);
			std::vector<uint32_t> raw_track_data;
			int iam_location = -1;
			int idam_location[64];
			int dam_location[64];
			bool sector_is_mfm[64];

			// Read track
			io.read_at(header_size + (heads * track + head) * track_size, &track_data[0], track_size, actual);

			for (int i = 0; i < 64; i++)
			{
				idam_location[i] = -1;
				dam_location[i] = -1;
				sector_is_mfm[i] = true;
			}

			// Find IDAM locations
			uint16_t track_header_offset = 0;
			uint16_t track_offset = ( ( track_data[track_header_offset + 1] << 8 ) | track_data[track_header_offset] ) & 0x3fff;
			bool is_mfm = (track_data[track_header_offset + 1] & 0x80) ? true : false;
			track_header_offset += 2;

			// track_offset >= 0x83 is for '-3' indexing at to get past the 128 bytes of index info
			int max_idam = -1;
			while ( track_offset != 0 && track_offset >= 0x83 && track_offset < track_size && track_header_offset < 0x80 )
			{
				int sector_index = (track_header_offset/2) - 1;
				sector_is_mfm[sector_index] = is_mfm;
				max_idam = sector_index;

				// Assume 3 bytes before IDAM pointers are the start of IDAM indicators
				int mark_offset = sector_is_mfm[sector_index] ? 3 : 0;
				idam_location[sector_index] = track_offset - mark_offset;

				// TODO: stride has to modify the search window
				int stride = is_mfm ? 1 : 2;//STRIDE
				// Scan for DAM location (PNP: scan original worked backwards, seems wrong)
				//for (int i = track_offset + 53; i > track_offset + 10; i--)
				for (int i = track_offset + 10*stride; i < track_offset + 53*stride; i++)
				{
					if ((track_data[i] >= 0xf8 && track_data[i] <= 0xfb))
					{
						if (!sector_is_mfm[sector_index] || (track_data[i-1] == 0xa1 && track_data[i-2] == 0xa1))
						{
							dam_location[sector_index] = i - mark_offset;
							break;
						}
					}
				}

				is_mfm = (track_data[track_header_offset + 1] & 0x80) ? true : false;
				track_offset = ( ( track_data[track_header_offset + 1] << 8 ) | track_data[track_header_offset] ) & 0x3fff;
				track_header_offset += 2;
			}

			if (max_idam >= 0 && max_idam < 63)
			{
				// Prevent encoding frame switching at end of disk.
				sector_is_mfm[max_idam + 1] = sector_is_mfm[max_idam];
			}

			// TODO: FM equivalent
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

			bool enc_mfm = sector_is_mfm[0];
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

				// If "close" to idam consider switching encoding
				if (offset + 8 >= idam_location[idam_index])
				{
					bool new_enc = sector_is_mfm[idam_index];
					if (new_enc != enc_mfm)
					{
						enc_mfm = new_enc;
					}
				}
				if (offset == idam_location[idam_index]
					|| (!enc_mfm && offset + 1 == idam_location[idam_index])	//STRIDE
					)
				{
					if (enc_mfm)
					{
						raw_w(raw_track_data, 16, 0x4489);
						raw_w(raw_track_data, 16, 0x4489);
						raw_w(raw_track_data, 16, 0x4489);
						offset += 3;
					}
					else
					{
						raw_w(raw_track_data, 32, wide_fm(0xf57e));
						offset += 2;	//STRIDE
					}
					idam_index += 1;
				}

				if (offset == dam_location[dam_index]
					|| (!enc_mfm && offset + 1 == dam_location[dam_index])	//STRIDE
					)
				{
					if (enc_mfm)
					{
						raw_w(raw_track_data, 16, 0x4489);
						raw_w(raw_track_data, 16, 0x4489);
						raw_w(raw_track_data, 16, 0x4489);
						offset += 3;
					}
					else
					{
/*
6a 6b 6e 6f
01 01 01 01
10 10 10 10
10 10 11 11
10 11 10 11
 8  9  a  b
*/
						uint16_t dam;
						switch (track_data[offset])
						{
							default:
							case 0xfb: dam = 0xf56f; break;
							case 0xfa: dam = 0xf56e; break;
							case 0xf9: dam = 0xf56b; break;
							case 0xf8: dam = 0xf56a; break;
						}
						raw_w(raw_track_data, 32, wide_fm(dam));
						offset += 2;	//STRIDE
					}
					dam_index += 1;
				}

				if (enc_mfm)
					mfm_w(raw_track_data, 8, track_data[offset]);
				else
					raw_w(raw_track_data, 32, data_to_wide_fm(track_data[offset]));

	
				//STRIDE TODO: brutal hack, proper stride handling is complicated
				if (!enc_mfm)
					offset++;
			}

			generate_track_from_levels(track, head, raw_track_data, 0, image);
		}
	}

	return true;
}

bool dmk_format::supports_save() const
{
	return false;
}


const dmk_format FLOPPY_DMK_FORMAT;
