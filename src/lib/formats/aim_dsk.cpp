// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/aim_dsk.cpp

    AIM disk images

    References:
    - http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385
    - http://agatcomp.ru/Soft/agat.shtml

*********************************************************************/

#include "aim_dsk.h"

#include "ioprocs.h"


aim_format::aim_format()
{
}


const char *aim_format::name() const noexcept
{
	return "aim";
}


const char *aim_format::description() const noexcept
{
	return "AIM disk image";
}


const char *aim_format::extensions() const noexcept
{
	return "aim";
}


int aim_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == 2068480)
		return FIFID_SIZE;

	return 0;
}


bool aim_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	image.set_variant(floppy_image::DSQD);

	const int tracks = 80;
	const int track_size = 6464 * 2;
	const int heads = 2;

	for (int track = 0; track < tracks; track++)
	{
		for (int head = 0; head < heads; head++)
		{
			std::vector<uint8_t> track_data(track_size);
			std::vector<uint32_t> raw_track_data;
			int data_count = 0;
			int splice_pos = -1;
			bool is_idam = false, is_dam = false;

			// Read track
			/*auto const [err, actual] =*/ read_at(io, (heads * track + head) * track_size, &track_data[0], track_size);
			// FIXME: check for error and premature EOF

			// Find first sector header or index mark
			for (int offset = 0; offset < track_size; offset += 2)
			{
				if (track_data[offset + 1] == 1 && splice_pos < 0)
				{
					splice_pos = std::max(0, offset - (20 * 2));
				}
				if (track_data[offset + 1] == 3)
				{
					splice_pos = offset;
				}
			}

			for (int offset = splice_pos; offset < track_size + splice_pos; offset += 2)
			{
				switch (track_data[(offset + 1) % track_size])
				{
				case 0: // regular data
					if (data_count == 0)
					{
						is_idam = (track_data[offset % track_size] == 0x95);
						is_dam = (track_data[offset % track_size] == 0x6a);
					}
					data_count++;
					// stored GAP3 may be too long (standard size is 22)
					if (!(is_dam && data_count > 256 + 2 + 2 + 22))
					{
						mfm_w(raw_track_data, 8, track_data[offset % track_size]);
					}
					break;

				case 1: case 0x80: // sync mark
					// stored GAP2 may be too short (standard formatter writes 5, sprite os uses 8)
					if (is_idam && data_count < 6 + 5)
					{
						for (; data_count < 6 + 5; data_count++)
						{
							mfm_w(raw_track_data, 8, 0xaa);
						}
					}
					mfm_w(raw_track_data, 8, 0xa4);
					raw_w(raw_track_data, 1, 0);
					mfm_w(raw_track_data, 8, 0xff);
					data_count = 0;
					break;

				// TELETEXT.AIM and others
				case 0xff:
					break;

				default:
					break;
				}
			}

			for (int i = raw_track_data.size(); i < 102144; i += 16)
			{
				mfm_w(raw_track_data, 8, 0xaa);
			}
			if (raw_track_data.size() < 102144)
			{
				mfm_w(raw_track_data, 102144 - raw_track_data.size(), 0xaa);
			}
			generate_track_from_levels(track, head, raw_track_data, 0, image);
		}
	}

	return true;
}


const aim_format FLOPPY_AIM_FORMAT;
