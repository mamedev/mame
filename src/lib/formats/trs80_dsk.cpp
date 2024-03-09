// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

TRS-80

JV1/3 disk image formats

Used by Jeff Vavasour's TRS-80 Emulators

Description of JV1:
  This format has no header or identifying characteristics. It is
  simply a string of sectors one after the other in order. There's
  no DAM or inter-sector bytes. The expected format is 10 sectors
  per track, 256 bytes per sector, single-sided, single-density.
  There's no limit on the number of tracks. It's up to the emulation
  to decide how to represent things. This format is intended only
  for the TRS-80 Model 1 with Level 2 basic. As it happens, the
  format is readable by the Model 3, but it's probably not a good
  idea to attempt writing on it.
  BUGS: If you try to create a JV1 disk, MAME will crash.

Description of JV3:
  There's no header, but the format is obvious to the practised eye.
  Firstly there's a 3 byte descriptor per sector, occurs 2901 times.
  This is followed by a byte to indicate if the disk is read-only. The
  total size of this area is 0x2200 bytes. The sector descriptors can
  be in any order. Unused descriptors are FF FF (FC | size). These
  can be intermixed with valid descriptors.
  Byte 1 - track number (0xFF indicates unused entry).
  Byte 2 - sector number.
  Byte 3 - flag byte. The bits are assigned as follows:
    Bits 0,1 - size in multiples of 128 bytes xor 1 for used tracks, or
       xor 2 for unused tracks.
       128 bytes: 1 (used), 2 (unused)
       256 bytes: 0 (used), 3 (unused)
       512 bytes: 3 (used), 0 (unused)
      1024 bytes: 2 (used), 1 (unused)
    bit 2: non-ibm flag. Some FDCs, such as the 1771, can have sector
       sizes from 16 to 1024 in any multiple of 16. MAME doesn't
       support this, and the format itself gives no indication of the
       size. So, it's impossible to support it.
    bit 3: sector has a CRC error.
    bit 4: side (0 = side 0, 1 = side 1)
    bits 5,6: DAM code:
       0xF8 (deleted):   0x60 (single-density), 0x20 (double-density)
       0xF9 (undefined): 0x40 (single-density), invalid/unused (double-density)
       0xFA (undefined): 0x20 (single-density), invalid/unused (double-density)
       0xFB (normal)   : 0x00 (single and double-density)
    bit 7: density (1 = dden/MFM). MAME doesn't support different densities in
       a track, so it's defaulted to single-density unless at least 1 sector is
       marked as double-density.
    After 0x2200 comes the data. There's no DAM bytes or inter-sector bytes
    included. If the last part of the descriptors is all unused, there's no
    need for those sectors to be in the image. If 2901 entries is not enough,
    the format allows another set of descriptors and data. However no TRS-80
    disks need this extra area, so it's unsupported. This format is intended
    only for the TRS-80 Models 3 and 4. Save/Create is currently unsupported.

    What we support: up to 2 sides, 96 tracks, 18 sectors per track.
                     We support all the flags except the non-ibm flag.
                     We do not support multiple sets of descriptors and data.
                     We do not support multiple densities per track.

***************************************************************************/

#include "trs80_dsk.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*


jv1_format::jv1_format() : wd177x_format(formats)
{
}

const char *jv1_format::name() const noexcept
{
	return "jv1";
}

const char *jv1_format::description() const noexcept
{
	return "TRS-80 JV1 disk image";
}

const char *jv1_format::extensions() const noexcept
{
	return "jv1,dsk";
}

int jv1_format::get_track_dam_fm(const format &f, int head, int track) const
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

const jv1_format FLOPPY_JV1_FORMAT;


#define MAX_SECTORS 19
#define MAX_TRACKS 96

jv3_format::jv3_format()
{
}

const char *jv3_format::name() const noexcept
{
	return "jv3";
}

const char *jv3_format::description() const noexcept
{
	return "TRS-80 JV3 disk image";
}

const char *jv3_format::extensions() const noexcept
{
	return "jv3,dsk";
}

int jv3_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t image_size;
	if (io.length(image_size))
		return 0;

	if (image_size < 0x2200)
		return 0; // too small, silent return

	auto const [err, data, actual] = read_at(io, 0, image_size);
	if (err || (actual != image_size))
		return 0;
	const uint32_t entries = 2901;
	const uint32_t header_size = entries *3 +1;

	// check the readonly flag
	if ((data[0x21ff] != 0) && (data[0x21ff] != 0xff))
		return 0; // flag is incorrect, silent return

	// Check for DMK
	uint8_t testbyte = 0;
	for (int i = 5; i < 16; i++)
		testbyte |= data[i];
	if (!testbyte)
		return 0; // DMK, silent return

	// Check for other disk formats
	if (data[1] >= MAX_SECTORS)
		return 0; // some other format, silent return

	// Full validation of the disk
	bool sector_array[2][MAX_TRACKS][MAX_SECTORS];
	for (uint8_t side = 0; side < 2; side++)
		for (uint8_t track = 0; track < MAX_TRACKS; track++)
			for (uint8_t sector = 0; sector < MAX_SECTORS; sector++)
				sector_array[side][track][sector] = false; // fill_n won't compile

	uint32_t data_ptr = header_size, last_data = header_size, size = 0;

	// printf errors may indicate it's another format, and so are ignorable.
	for (uint32_t sect = 0; sect < entries; sect++)
	{
		uint8_t track = data[sect*3];
		uint8_t sector = data[sect*3+1];
		uint8_t flag_size = data[sect*3+2] & 3;
		uint8_t flag_side = (data[sect*3+2] & 0x10) ? 1 : 0;

		if (track < 0xff)
		{
			// validate so we don't overrun the array
			if (track >= MAX_TRACKS)
			{
				osd_printf_info("jv3_format::identify - track %d exceeds maximum allowed (%d)\n",track,MAX_TRACKS-1);
				return 0;
			}

			if (sector >= MAX_SECTORS)
			{
				osd_printf_info("jv3_format::identify - sector %d exceeds maximum allowed (%d)\n",sector,MAX_SECTORS-1);
				return 0;
			}

			// check if sector already exists
			if (sector_array[flag_side][track][sector])
			{
				osd_printf_info("jv3_format::identify - side %d track %d sector %d is duplicated\n",flag_side,track,sector);
				return 0;
			}

			sector_array[flag_side][track][sector] = true;

			size = 128 << (flag_size ^1);
			data_ptr += size;
			last_data = data_ptr;
		}
		else
		{
			size = 128 << (flag_size ^2);
			data_ptr += size;
		}
	}

	// Is all data in the image? (unused tracks at the end are optional)
	if (last_data > image_size)
	{
		osd_printf_info("jv3_format::identify - disk is missing some data. Expected 0x%X, Actual = 0x%X\n",last_data,image_size);
		return 0;
	}

	return FIFID_STRUCT;
}

bool jv3_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	// disk has already been validated in every way except if it exceeds drive tracks, we do that below
	osd_printf_info("Disk detected as JV3\n");
	int drive_tracks, drive_sides;
	image.get_maximal_geometry(drive_tracks, drive_sides);
	uint64_t image_size;
	if (io.length(image_size))
		return false;
	auto const [err, data, actual] = read_at(io, 0, image_size);
	if (err || (actual != image_size))
		return 0;
	const uint32_t entries = 2901;
	const uint32_t header_size = entries *3 +1;
	bool is_dd = false, is_ds = false;

	for (uint8_t curr_side = 0; curr_side < 2; curr_side++)
	{
		for (uint8_t curr_track = 0; curr_track < MAX_TRACKS; curr_track++)
		{
			// Find out how many sectors are in this track
			uint8_t max_sect = 0, min_sect = 0xff;
			for (uint32_t sect = 0; sect < entries; sect++)
			{
				uint8_t track = data[sect*3];
				uint8_t sector = data[sect*3+1];
				uint8_t flags = data[sect*3+2];
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
			if (max_sect >= min_sect)
			{
				// Protect against oversized disk
				if ((curr_track >= drive_tracks) || (curr_side >= drive_sides))
				{
					osd_printf_error("Disk exceeds drive capabilities\n");
					return false;
				}

				max_sect++;  // now it is the max possible number of sectors on this track

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

				uint32_t data_ptr = header_size;
				bool ddensity = false;
				// Search for sectors on this track & side
				for (uint32_t sect = 0; sect < entries; sect++)
				{
					uint8_t track = data[sect*3];
					uint8_t sector = data[sect*3+1];
					uint8_t flags = data[sect*3+2];
					uint8_t side = (flags & 0x10) ? 1 : 0;
					uint8_t size = (flags & 0x03);
					if (track == 0xff)
						size ^= 2;
					else
						size ^= 1;
					uint16_t sector_size = 128 << size;
					if ((track == curr_track) && (side == curr_side))
					{
						// process the sector
						sectors[sector].track = curr_track;
						sectors[sector].head = curr_side;
						sectors[sector].sector = sector;
						sectors[sector].actual_size = sector_size;
						sectors[sector].size = size;
						uint8_t dam = flags & 0xe0;
						sectors[sector].deleted = ((dam == 0x60) || (dam == 0xa0)) ? true : false;
						sectors[sector].bad_crc = (flags & 0x08) ? true : false;
						sectors[sector].data = &data[data_ptr];
						if (flags & 0x80)
							ddensity = true;
					}
					data_ptr += sector_size;
				}

				//osd_printf_verbose("Side %d, Track %d, %s density\n",curr_side,curr_track,ddensity ? "Double" : "Single");
				if (ddensity)
				{
					is_dd = true;
					build_wd_track_mfm(curr_track, curr_side, image, 100000, max_sect, sectors, 32, 31, 22);
				}
				else
					build_wd_track_fm(curr_track, curr_side, image, 50000, max_sect, sectors, 14, 12, 11);

				if (curr_side)
					is_ds = true;
			}
		}
	}
	if (is_dd)
	{
		if (is_ds)
			image.set_variant(floppy_image::DSDD);
		else
			image.set_variant(floppy_image::SSDD);
	}
	else
	{
		if (is_ds)
			image.set_variant(floppy_image::DSSD);
		else
			image.set_variant(floppy_image::SSSD);
	}
	return true;
}

bool jv3_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int track_count, head_count;
	image.get_actual_geometry(track_count, head_count);

	if (track_count)
	{
		// If the disk already exists, find out if it's writable
		uint64_t image_size;
		if (!io.length(image_size) && (image_size >= 0x2200))
		{
			uint8_t flag;
			auto const [err, actual] = read_at(io, 0x21ff, &flag, 1);
			if (err || (1 != actual)) // TODO: should we save over the top if we couldn’t read the read-only flag?
				return false;
			if (flag == 0)
				return false;   // disk is read-only
		}
	}

	uint32_t data_ptr = 0x2200, sect_ptr = 0;


	// prepare default header, which we will partially overwrite later
	uint8_t header[0x2200];
	std::fill_n(header, std::size(header), 0xFF);

	if (!track_count)
	{
		track_count = 35;
		head_count = 1;
	}

	// write disk data
	for (uint8_t head = 0; head < head_count; head++)
	{
		for (uint8_t track = 0; track < track_count; track++)
		{
			auto bitstream = generate_bitstream_from_track(track, head, 2000, image);
			auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
			bool ddensity = true;
			if (sectors.empty())
			{
				bitstream = generate_bitstream_from_track(track, head, 4000, image);
				sectors = extract_sectors_from_bitstream_fm_pc(bitstream);
				ddensity = false;
			}

			if (sectors.empty())
			{
				// write dummy data. If we write nothing at all, mame hangs
				uint8_t dummy[256];
				std::fill_n(dummy, std::size(dummy), 0xE5);
				for (uint8_t i = 0; i < 10; i++)
				{
					header[sect_ptr++] = track;
					header[sect_ptr++] = i;
					header[sect_ptr++] = head ? 0x10 : 0;
					/*auto const [err, actual] =*/ write_at(io, data_ptr, &dummy[0], 256); // FIXME: check for errors
					data_ptr += 256;
				}
			}
			else
			{
				// How many sectors?
				uint8_t count = 0;
				for (const auto &s : sectors)
					if (!s.empty())
						count++;

				// Save them
				for (uint8_t i = 0; i < count; i++)
				{
					header[sect_ptr++] = track;
					header[sect_ptr++] = i;
					uint8_t flags = (ddensity) ? 0x80 : 0;
					flags |= (sectors[i].size() >> 8) ^1;
					flags |= head ? 0x10 : 0;
					header[sect_ptr++] = flags;
					/*auto const [err, actual] =*/ write_at(io, data_ptr, sectors[i].data(), sectors[i].size()); // FIXME: check for errors
					data_ptr += sectors[i].size();
				}
			}
		}
	}
	// Save the header
	/*auto const [err, actual] =*/ write_at(io, 0, header, 0x2200); // FIXME: check for errors

	return true;
}


bool jv3_format::supports_save() const noexcept
{
	return true;
}

const jv3_format FLOPPY_JV3_FORMAT;

