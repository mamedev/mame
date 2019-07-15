// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    JVC

    Disk image format

    Used by Jeff Vavasour's CoCo Emulators

     Documentation taken from Tim Linder's web site:
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

#include "emu.h"
#include "imageutl.h"
#include "jvc_dsk.h"

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

bool jvc_format::parse_header(io_generic *io, int &header_size, int &tracks, int &heads, int &sectors, int &sector_size, int &base_sector_id)
{
	// The JVC format has a header whose size is the size of the image modulo 256.  Currently, we only
	// handle up to five header bytes
	uint64_t size = io_generic_size(io);
	header_size = size % 256;
	uint8_t header[5];

	// if we know that this is a header of a bad size, we can fail
	// immediately; otherwise read the header
	if (header_size >= sizeof(header))
		return false;
	if (header_size > 0)
		io_generic_read(io, header, 0, header_size);

	// default values
	heads = 1;
	sectors = 18;
	sector_size = 256;
	base_sector_id = 1;

	switch (header_size)
	{
	case 5: emu_fatalerror("jvc_format: sector attribute flag unsupported\n");
		break;
	case 4: base_sector_id = header[3];
		// no break
	case 3: sector_size = 128 << header[2];
		// no break
	case 2: heads = header[1];
		// no break
	case 1: sectors = header[0];
		// no break
	case 0: tracks = (size - header_size) / sector_size / sectors / heads;
		break;
	}

	osd_printf_verbose("Floppy disk image geometry: %d tracks, %d head(s), %d sectors with %d bytes.\n", tracks, heads, sectors, sector_size);

	return tracks * heads * sectors * sector_size == (size - header_size);
}

int jvc_format::identify(io_generic *io, uint32_t form_factor)
{
	int header_size, tracks, heads, sectors, sector_size, sector_base_id;
	return parse_header(io, header_size, tracks, heads, sectors, sector_size, sector_base_id) ? 50 : 0;
}

bool jvc_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	int header_size, track_count, head_count, sector_count, sector_size, sector_base_id;

	if (!parse_header(io, header_size, track_count, head_count, sector_count, sector_size, sector_base_id))
		return false;

	// safety check
	if (sector_count * sector_size > 10000)
		emu_fatalerror("jvc_format: incorrect track layout\n");

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

				io_generic_read(io, sectors[interleave[i]].data, file_offset, sector_size);

				sector_offset += sector_size;
				file_offset += sector_size;
			}

			build_wd_track_mfm(track, head, image, 100000, sector_count, sectors, 22, 32, 24);
		}
	}

	return true;
}

bool jvc_format::save(io_generic *io, floppy_image *image)
{
	uint8_t bitstream[500000/8];
	uint8_t sector_data[50000];
	desc_xs sectors[256];
	uint64_t file_offset = 0;

	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	// we'll write a header if the disk is two-sided
	if (head_count == 2)
	{
		uint8_t header[2];
		header[0] = 18;
		header[1] = 2;
		io_generic_write(io, header, file_offset, sizeof(header));
		file_offset += sizeof(header);
	}

	// write disk data
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			int track_size;
			generate_bitstream_from_track(track, head, 2000, bitstream, track_size, image);
			extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sector_data, sizeof(sector_data));

			for (int i = 0; i < 18; i++)
			{
				if (sectors[1 + i].size != 256)
					emu_fatalerror("jvc_format: invalid sector size: %d\n", sectors[1 + i].size);

				io_generic_write(io, sectors[1 + i].data, file_offset, sectors[1 + i].size);
				file_offset += sectors[1 + i].size;
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
