// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/aim_dsk.h

    AIM disk images

    References:
    - http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385
    - http://agatcomp.ru/Soft/agat.shtml

*********************************************************************/

#include <assert.h>

#include "aim_dsk.h"


aim_format::aim_format()
{
}


const char *aim_format::name() const
{
	return "aim";
}


const char *aim_format::description() const
{
	return "AIM disk image";
}


const char *aim_format::extensions() const
{
	return "aim";
}


int aim_format::identify(io_generic *io, uint32_t form_factor)
{
	if (io_generic_size(io) == 2068480)
	{
		return 100;
	}

	return 0;
}


bool aim_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	image->set_variant(floppy_image::DSQD);

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
			bool header = false;

			// Read track
			io_generic_read(io, &track_data[0], ( heads * track + head ) * track_size, track_size);

			for (int offset = 0; offset < track_size; offset += 2)
			{
				switch (track_data[offset + 1] & 1)
				{
				case 0:
					if (data_count == 0)
						header = (track_data[offset] == 0x95) ? true : false;
					data_count++;
					mfm_w(raw_track_data, 8, track_data[offset]);
					break;

				case 1:
					if (header && data_count < 11) // XXX hack
					{
						for (; data_count < 12; data_count++)
						{
							mfm_w(raw_track_data, 8, 0xaa);
						}
					}
					raw_w(raw_track_data, 16, 0x8924);
					raw_w(raw_track_data, 16, 0x5555);
					data_count = 0;
					break;
				}
			}

			generate_track_from_levels(track, head, raw_track_data, 0, image);
		}
	}

	return true;
}


const floppy_format_type FLOPPY_AIM_FORMAT = &floppy_image_format_creator<aim_format>;
