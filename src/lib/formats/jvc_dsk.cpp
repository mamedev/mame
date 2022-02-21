// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    JVC

    Disk image format

    Used by Jeff Vavasour's CoCo Emulators

     Documentation taken from Tim Lindner's web site:
         http://tlindner.macmess.org/?page_id=86

     A. Header length
         The header length is determined by the file length modulo 256:
             headerSize = fileLength % 256;
         This means that the header is variable length and the minimum size
         is zero bytes, and the maximum size of 255 bytes.

    B. Header
         Here is a description of the header bytes:
             Byte Offset     Description            Default
             -----------     -----------            -------
                       0     Sectors per track      18
                       1     Side count              1
                       2     Sector size code        1
                       3     First sector ID         1
                       4     Sector attribute flag   0

     If the sector attribute flag is zero then the track count is determined
     by the formula:

         (fileLength - headerSize) / (sectorsPerTrack * (128 <<
             sectorSizeCode)) / sideCount

     If the sector attribute flag is non zero then the track count is
     determined by the more complex formula:

         (fileLength - headerSize) / (sectorsPerTrack * ((128 <<
             sectorSizeCode) + 1) ) / sideCount

     If the length of the header is to short to contain the geometry desired,
     then the default values are assumed. If the header length is zero the all
     of the geometry is assumed. When creating disk images it is desirable to
     make the header length as short as possible. The header should only be
     used to deviate from the default values.

     The sector data begins immediately after the header. If the header length
     is zero then the sector data is at the beginning file.

     C. Sectors per track
         This is the number of sectors per track (ones based). A value of 18
         means there are 18 sectors per track

     D. Side Count
         This is the number of sides in the disk image. Values of 1 or 2 are
         acceptable. If there are two sides then the tracks are interleaved.
         The first track in the image file is track zero side 1, the second
         track in the image file is track zero side 2.

     E. Sector size
         The is the same value that is stored in the wd179x ID field to
         determine sector size:

             0x00         128 bytes
             0x01         256 bytes
             0x02         512 bytes
             0x03        1024 bytes

     Other values are undefined. Every sector in the disk image must be the
     same size.

     F. First sector ID
         This determines the first sector ID for each track. Each successive
         sector adds one to the previous ID. If the first sector ID is 1, then
         the second sector has an ID of 2, and the third has an ID of 3.

     G. Sector Attribute Flag
         If this byte is non zero, then each sector contains an additional
         byte prepended to the sector data. If the attribute flag is zero then
         there are no extra bytes in front of the sector data.

     H. Sector attribute byte
         This byte is put at the beginning of every sector if the header flag
         is turned on. The information this byte contains is the same as the
         status register (of the wd179x) would contain when a 'Read Sector'
         command was issued. The bit fields are defined as:

         Bit position:
         ---------------
         7 6 5 4 3 2 1 0
         | | | | | | | |
         | | | | | | | +--- Not used. Set to zero.
         | | | | | | +----- Not used. Set to zero.
         | | | | | +------- Not used. Set to zero.
         | | | | +--------- Set on CRC error.
         | | | +----------- Set if sector not found.
         | | +------------- Record type: 1 - Deleted Data Mark, 0 - Data Mark.
         | +--------------- Not Used. Set to zero.
         +----------------- Not Used. Set to zero.

    TODO:
    - Support writing unusual formats?

***************************************************************************/

#include "jvc_dsk.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*


jvc_format::jvc_format()
{
}

const char *jvc_format::name() const
{
	return "jvc";
}

const char *jvc_format::description() const
{
	return "JVC disk image";
}

const char *jvc_format::extensions() const
{
	return "jvc,dsk";
}

bool jvc_format::parse_header(util::random_read &io, int &header_size, int &tracks, int &heads, int &sectors, int &sector_size, int &base_sector_id)
{
	// The JVC format has a header whose size is the size of the image modulo 256.  Currently, we only
	// handle up to five header bytes
	uint64_t size;
	if (io.length(size))
		return false;
	header_size = size % 256;
	uint8_t header[5];

	// if we know that this is a header of a bad size, we can fail immediately; otherwise read the header
	size_t actual;
	if (header_size >= sizeof(header))
		return false;
	if (header_size > 0)
		io.read_at(0, header, header_size, actual);

	// default values
	heads = 1;
	sectors = 18;
	sector_size = 256;
	base_sector_id = 1;

	switch (header_size)
	{
	case 5:
		osd_printf_info("jvc_format: sector attribute flag unsupported\n");
		return false;
	case 4: base_sector_id = header[3];
		[[fallthrough]];
	case 3: sector_size = 128 << header[2];
		[[fallthrough]];
	case 2: heads = header[1];
		[[fallthrough]];
	case 1: sectors = header[0];
		[[fallthrough]];
	case 0: tracks = (size - header_size) / sector_size / sectors / heads;
		break;
	}

	osd_printf_verbose("jvc_format: Floppy disk image geometry: %d tracks, %d head(s), %d sectors with %d bytes.\n", tracks, heads, sectors, sector_size);

	return tracks * heads * sectors * sector_size == (size - header_size);
}

int jvc_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int header_size, tracks, heads, sectors, sector_size, sector_base_id;
	return parse_header(io, header_size, tracks, heads, sectors, sector_size, sector_base_id) ? 50 : 0;
}

bool jvc_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	int header_size, track_count, head_count, sector_count, sector_size, sector_base_id;
	int max_tracks, max_heads;

	if (!parse_header(io, header_size, track_count, head_count, sector_count, sector_size, sector_base_id))
		return false;

	// safety check
	if (sector_count * sector_size > 10000)
	{
		osd_printf_error("jvc_format: incorrect track layout\n");
		return false;
	}

	image->get_maximal_geometry(max_tracks, max_heads);

	if (track_count > max_tracks)
	{
		osd_printf_error("jvc_format: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", track_count, max_tracks);
		return false;
	}

	if (head_count > max_heads)
	{
		osd_printf_error("jvc_format: Floppy disk has too many sides for this drive (floppy sides=%d, drive sides=%d).\n", head_count, max_heads);
		return false;
	}

	int file_offset = header_size;

	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count ; head++)
		{
			desc_pc_sector sectors[256];
			uint8_t sector_data[10000];
			int sector_offset = 0;
			// standard RS-DOS interleave
			static constexpr int interleave[18] = { 0, 11, 4, 15, 8, 1, 12, 5, 16, 9, 2, 13, 6, 17, 10, 3, 14, 7 };
			for (int i = 0; i < sector_count; i++)
			{
				sectors[interleave[i]].track = track;
				sectors[interleave[i]].head = head;
				sectors[interleave[i]].sector = sector_base_id + i;
				sectors[interleave[i]].actual_size = sector_size;
				sectors[interleave[i]].size = sector_size >> 8;
				sectors[interleave[i]].deleted = false;
				sectors[interleave[i]].bad_crc = false;
				sectors[interleave[i]].data = &sector_data[sector_offset];

				size_t actual;
				io.read_at(file_offset, sectors[interleave[i]].data, sector_size, actual);

				sector_offset += sector_size;
				file_offset += sector_size;
			}

			build_wd_track_mfm(track, head, image, 100000, sector_count, sectors, 22, 32, 24);
		}
	}

	return true;
}

bool jvc_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint64_t file_offset = 0;

	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	// we'll write a header if the disk is two-sided
	if (head_count == 2)
	{
		uint8_t header[2];
		header[0] = 18;
		header[1] = 2;
		size_t actual;
		io.write_at(file_offset, header, sizeof(header), actual);
		file_offset += sizeof(header);
	}

	// write disk data
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			auto bitstream = generate_bitstream_from_track(track, head, 2000, image);
			auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);

			for (int i = 0; i < 18; i++)
			{
				if (sectors[1 + i].size() != 256)
				{
					osd_printf_error("jvc_format: invalid sector size: %d\n", sectors[1 + i].size());
					return false;
				}

				size_t actual;
				io.write_at(file_offset, sectors[1 + i].data(), 256, actual);
				file_offset += 256;
			}
		}
	}

	return true;
}

bool jvc_format::supports_save() const
{
	return true;
}

const floppy_format_type FLOPPY_JVC_FORMAT = &floppy_image_format_creator<jvc_format>;
