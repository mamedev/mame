// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    TRS-80

    JV1/3 disk image formats

    Used by Jeff Vavasour's TRS-80 Emulators

    TODO:
    - Gap sizes unverified.
    - JV3: not working. Disk can be loaded, but it's unreadable.
    - JV3: cannot create.
    - JV1: if you try to create, MAME will crash.
    - all: if the disk image contains more tracks than the drive,
           then MAME could crash.

***************************************************************************/

#include "trs80_dsk.h"

jv1_format::jv1_format() : wd177x_format(formats)
{
}

const char *jv1_format::name() const
{
	return "jv1";
}

const char *jv1_format::description() const
{
	return "TRS-80 JV1 disk image";
}

const char *jv1_format::extensions() const
{
	return "jv1,dsk";
}

int jv1_format::get_track_dam_fm(const format &f, int head, int track)
{
	return (track == 17 && head == 0) ? FM_DDAM : FM_DAM;
}

const jv1_format::format jv1_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{
		floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
		4000, 10, 80, 1, 256, {}, 0, {}, 14, 11, 12
	},
	{}
};

const floppy_format_type FLOPPY_JV1_FORMAT = &floppy_image_format_creator<jv1_format>;


#define MAX_SECTORS 20

jv3_format::jv3_format()
{
}

const char *jv3_format::name() const
{
	return "jv3";
}

const char *jv3_format::description() const
{
	return "TRS-80 JV3 disk image";
}

const char *jv3_format::extensions() const
{
	return "jv3,dsk";
}

int jv3_format::identify(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint32_t image_size = io_generic_size(io);

	if (image_size < 0x2300)
		return 0;

	const uint32_t entries = 2901;
	const uint32_t header_size = entries *3 +1;
	uint8_t header[0x2200];
	io_generic_read(io, header, 0, header_size);

	// check the readonly flag
	if ((header[0x21ff] != 0) && (header[0x21ff] != 0xff))
		return 0;

	// Check for DMK
	uint8_t testbyte = 0;
	for (int i = 5; i < 16; i++)
		testbyte |= header[i];
	if (!testbyte)
		return 0;

	// Check for other disk formats
	if (header[1] >= MAX_SECTORS)
		return 0;

	// Make sure all data is in the image
	uint32_t data_ptr = header_size;
	uint32_t sect_ptr = 0;

	for (uint32_t sect = 0; sect < entries; sect++)
	{
		uint8_t track = header[sect_ptr++];
		uint8_t sector = header[sect_ptr++];
		uint8_t flags = header[sect_ptr++];

		if (track > 0x7f)
			break;
		if (sector >= MAX_SECTORS)
			return 0;
		if ((flags & 0xfc) == 0xfc)
			break;

		uint8_t size = (flags & 0x03)^1;
		data_ptr += 128 << size;
	}
	if (data_ptr > image_size)
		return 0;

	return 80;
}

bool jv3_format::load(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	printf("Disk detected as JV3\n");fflush(stdout);
	int drive_tracks, drive_sides;
	image->get_maximal_geometry(drive_tracks, drive_sides);
	uint32_t image_size = io_generic_size(io);
	const uint32_t entries = 2901;
	const uint32_t header_size = entries *3 +1;
	uint8_t header[0x2200];
	io_generic_read(io, header, 0, header_size);
	bool is_dd = false, is_ds = false;

	for (uint8_t curr_side = 0; curr_side < 2; curr_side++)
	{
		for (uint8_t curr_track = 0; curr_track < 0x80; curr_track++)
		{
			// Find out how many sectors are in this track
			uint8_t max_sect = 0, min_sect = 0xff;
			uint32_t sect_ptr = 0;
			for (uint32_t sect = 0; sect < entries; sect++)
			{
				uint8_t track = header[sect_ptr++];
				uint8_t sector = header[sect_ptr++];
				uint8_t flags = header[sect_ptr++];
				uint8_t side = (flags & 0x10) ? 1 : 0;
				if ((track == curr_track) && (side == curr_side))
				{
					if (sector > max_sect)
						max_sect = sector;
					if (sector < min_sect)
						min_sect = sector;
				}
			}

			// If there's any data for this track process it
			// We assume that all sectors from 0 to max_sect exist
			if (max_sect >= min_sect)
			{
				// Make sure we don't have too many sectors
				if (max_sect >= MAX_SECTORS)
				{
					osd_printf_error("Sector number %d exceeds maximum allowed (%d).\n",max_sect,MAX_SECTORS);
					return false;
				}

				max_sect++;  // now it is the number of sectors on this track

				desc_pc_sector sectors[MAX_SECTORS];
				// set defaults in case some sectors are missing from the image
				for (uint8_t i = 0; i < max_sect; i++)
				{
					sectors[i].track = curr_track;
					sectors[i].head = curr_side;
					sectors[i].sector = i;
					sectors[i].actual_size = 256;
					sectors[i].size = 1;
					sectors[i].deleted = true;
					sectors[i].bad_crc = false;
					sectors[i].data = 0;
				}
				sect_ptr = 0;
				uint32_t data_ptr = header_size;
				bool density = 0;
				// Search for sectors on this track & side
				for (uint32_t sect = 0; sect < entries; sect++)
				{
					uint8_t track = header[sect_ptr++];
					uint8_t sector = header[sect_ptr++];
					uint8_t flags = header[sect_ptr++];
					uint8_t side = (flags & 0x10) ? 1 : 0;
					uint8_t size = (flags & 0x03);
					if (track == 0xff)
						size ^= 2;
					else
						size ^= 1;
					uint16_t sector_size = 128 << size;
					uint8_t sector_data[1024];
					std::fill_n(sector_data,1024,0);
					if ((track == curr_track) && (side == curr_side))
					{
						// If we would exceed the image size then ignore this sector
						if ((data_ptr + sector_size) <= image_size)
						{
							// process the sector
							io_generic_read(io, sector_data, data_ptr, sector_size);
							sectors[sector].track = curr_track;
							sectors[sector].head = curr_side;
							sectors[sector].sector = sector;
							sectors[sector].actual_size = sector_size;
							sectors[sector].size = size;
							sectors[sector].deleted = (flags & 0x20) ? true : false;
							sectors[sector].bad_crc = (flags & 0x08) ? true : false;
							sectors[sector].data = &sector_data[0];
							density = (flags & 0x80) ? 1 : 0;
						}
					}
					data_ptr += sector_size;
				}

				// Protect against oversized disk
				if ((curr_track >= drive_tracks) || (curr_side >= drive_sides))
				{
					osd_printf_error("Disk exceeds drive capabilities\n");
					return false;
				}

				//printf("Side %d, Track %d, %s density\n",curr_side,curr_track,density ? "Double" : "Single");
				if (density)
				{
					is_dd = true;
					build_wd_track_mfm(curr_track, curr_side, image, 100000, max_sect, sectors, 32, 22, 31);
				}
				else
					build_wd_track_fm(curr_track, curr_side, image, 50000, max_sect, sectors, 14, 11, 12);

				if (curr_side)
					is_ds = true;
			}
		}
	}
	if (is_dd)
	{
		if (is_ds)
			image->set_variant(floppy_image::DSDD);
		else
			image->set_variant(floppy_image::SSDD);
	}
	else
	{
		if (is_ds)
			image->set_variant(floppy_image::DSSD);
		else
			image->set_variant(floppy_image::SSSD);
	}
	return true;
}

bool jv3_format::save(io_generic *io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	// To be done
	return false;
}


bool jv3_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_JV3_FORMAT = &floppy_image_format_creator<jv3_format>;

