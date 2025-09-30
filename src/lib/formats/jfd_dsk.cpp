// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/jfd_dsk.cpp

    JASPP Floppy Disk image format

    JFD file structure
    ------------------

    A JFD file is a gzip compressed version of the original JFD
    file.

    Official files should be named "[optional text name] Fxxxxxnn"
    (see below for xxxx / nn explanation) eg "Fire & Ice (1995)
    (Warner Interactive Entertainment) F1015401" or shortened to
    "F1015401" on an Arc that doesn't support long file names.

    Compressed file always starts:
    1F 8B 08 00 00 00 00 00 00 0B EC BD
    ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^
    |  |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |  +- OS
    |  |  |  |  |  |  |  |  +---- xfl
    |  |  |  |  +--+--+--+------- time
    |  |  |  +------------------- gzip flags
    |  |  +---------------------- gzip compression*
    |  |
    +--+------------------------- gzip header

    *  Compression method: 8 is the only supported format


    Original JFD file structure:

    +0   "JFDI" identifier
    +4   Min. version of ADFFS required to handle file * 100
          eg 1.25 would be 125
    +8   Memory allocation required to load file in bytes
          ie uncompressed file size
    +12  Disc sequence number - SSSSnnnn
          Where:
            SSSS is the number of discs in this set
            nnnn is the number of this disc
    +16  Game ID number (xxxxx) - official release ID of this
          floppy set.  Unofficial releases should be an ID of zero
    +20  Image version number: VVVwwwww
          Where:
            VVV - major release version, 0 being initial release
          wwwww - minor version, reset to 0 on a major release
                  and is incremented on each flush / save as
    +24  Offset to Track Table, the end of which is denoted by
          the start of the Sector Table
    +28  Offset to Sector Table, end of which is denoted by
          the start of the Data Table
    +32  Offset to Data Table
    +36  Offset to optional Delta Track Table.  Changes to the
          Track table are written here post initial release
          0 denotes not present
    +40  Offset to optional Delta Sector Table.  Changes to the
          Sector table are written here post initial release
          0 denotes not present
    +44  Offset to optional Delta Data Table.  Changes to the
          Data table are written here post initial release
          0 denotes not present
    +48  Disc title.  eg "Fire & Ice (1995) (Warner Interactive
          Entertainment) disc 1 of 2", zero terminated.
    +304 ... future expansion space (additions in 2.04+)

    +304 Bit flags:
          0 - Disk write protected
          1 - Disk is RW, write delta changes back to JFD
          2 - Protect CMOS required
          3 - Protect Modules required
          4 - Hide Hourglass
          5 - Requires Shift-Break to load
          6 - Remap video memory to 1FFFFFF downwards

          8 - ARM3 compatible
          9 - ARM250 compatible
         10 - ARM610/710 compatible
         11 - ARM7500 compatible
         12 - StrongArm compatible
         13 = ARMv5/v5/v7 compatible

         16 - RiscOS 2 compatible
         17 - RiscOS 3.1 compatible
         18 - RiscOS 3.5 compatible
         19 - RiscOS 3.7 compatible
         20 - RiscOS 3.8 / 4.x compatible
         21 - RiscOS 5.x compatible
         22 - RiscOS 6.x compatible
    +308 FPS of the original game * 2 (eg for 12.5 FPS use 25)
    +309 - 311 reserved
    +312 Length of Obey file in bytes
    +316 ... future expansion space (currently 0 bytes)

    [Track Table]
    +0 Track 0 offset within Sector Table
    +4 Track 1 offset within Sector Table
    ...
    +n*4 Track n offset within Sector Table
           n can be any value, although to allow writes should be at
           least 160.  For reference, APD's are 166
           A blank track is denoted by FFFFFFFF

    [Sector Table]
    +0 ttBdnnCs - Sector Header description (see below)
    +4 Offset within Data Table of sector data, if this value is
         FFFFFFFF no data is associated with the sector.  In the
         case of a sector with a CRC flag set in C set, the sector
         is a stub to produce a specific error.

    .. repeats for all sectors in track, two doubles (ie 8 bytes) per
         sector

    +n FFFFFFFF - end of track marker

    The above repeats for all tracks containing sector data.


    [Data Table]
    ... X bytes of sector data in native byte format.
         Not necessarily in disc order, as sectors are located
         individually during reads.

    [Obey file]
    ... Obey file used to boot the floppy


    Sector Header description:  ttBdnnCs
    Where:

    tt - time of sector from start of index marker in milliseconds
         max being 200 (optional, required for duplicate sectors)
    B  - Option bits
           bit 0 - sector data contains DiscOp 3 data for the track
           bit 1 - reserved
           bit 2 - reserved
           bit 3 - reserved
    d  - Density value, where:
           1 - single
           2 - double
           4 - quad
           8 - octal
    nn - Sector number
    C  - CRC bits, where:
           bit 0 - ID CRC invalid
           bit 1 - Data CRC invalid
           bit 2 - reserved
           bit 3 - Slow sector (read at normal floppy speed)
    s  - 1772 sector size, where:
           0 - 128 bytes
           1 - 256 bytes
           2 - 512 bytes
           3 - 1024 bytes

*********************************************************************/

#include "formats/jfd_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_*

#include <zlib.h>


static const uint8_t JFD_HEADER[4] = { 'J', 'F', 'D', 'I' };
static const uint8_t GZ_HEADER[2] = { 0x1f, 0x8b };

jfd_format::jfd_format()
{
}

const char *jfd_format::name() const noexcept
{
	return "jfd";
}

const char *jfd_format::description() const noexcept
{
	return "JASPP Floppy Disk image";
}

const char *jfd_format::extensions() const noexcept
{
	return "jfd";
}

int jfd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size) || !size)
		return 0;

	std::vector<uint8_t> img(size);
	auto const [ioerr, actual] = read_at(io, 0, &img[0], size);
	if (ioerr || (actual != size))
		return 0;

	int err;
	std::vector<uint8_t> gz_ptr(4);
	z_stream d_stream;

	if (!memcmp(&img[0], GZ_HEADER, sizeof(GZ_HEADER))) {
		d_stream.zalloc = nullptr;
		d_stream.zfree = nullptr;
		d_stream.opaque = nullptr;
		d_stream.next_in = &img[0];
		d_stream.avail_in = size;
		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out = 4;

		err = inflateInit2(&d_stream, MAX_WBITS | 16);
		if (err != Z_OK) return 0;

		err = inflate(&d_stream, Z_SYNC_FLUSH);
		if (err != Z_OK) return 0;

		err = inflateEnd(&d_stream);
		if (err != Z_OK) return 0;

		img = std::move(gz_ptr);
	}

	if (!memcmp(&img[0], JFD_HEADER, sizeof(JFD_HEADER))) {
		return FIFID_SIGN;
	}

	return 0;
}

bool jfd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	std::vector<uint8_t> img(size);
	auto const [ioerr, actual] = read_at(io, 0, &img[0], size);
	if (ioerr || (actual != size))
		return false;

	int err;
	std::vector<uint8_t> gz_ptr;
	z_stream d_stream;
	int inflate_size = get_u32le(&img[size - 4]);

	if (!memcmp(&img[0], GZ_HEADER, sizeof(GZ_HEADER))) {
		gz_ptr.resize(inflate_size);

		d_stream.zalloc    = nullptr;
		d_stream.zfree     = nullptr;
		d_stream.opaque    = nullptr;
		d_stream.next_in   = &img[0];
		d_stream.avail_in  = size;
		d_stream.next_out  = &gz_ptr[0];
		d_stream.avail_out = inflate_size;

		err = inflateInit2(&d_stream, MAX_WBITS | 16);
		if (err != Z_OK) {
			osd_printf_error("inflateInit2 error: %d\n", err);
			return false;
		}
		err = inflate(&d_stream, Z_FINISH);
		if (err != Z_STREAM_END && err != Z_OK) {
			osd_printf_error("inflate error: %d\n", err);
			return false;
		}
		err = inflateEnd(&d_stream);
		if (err != Z_OK) {
			osd_printf_error("inflateEnd error: %d\n", err);
			return false;
		}
		size = inflate_size;
		img = std::move(gz_ptr);
	}

	osd_printf_verbose("jfd_dsk: loading %s\n", &img[48]);

	uint32_t offset_track  = little_endianize_int32(*(uint32_t *)(&img[24])); /* Track Table  */
	uint32_t offset_sector = little_endianize_int32(*(uint32_t *)(&img[28])); /* Sector Table */
	uint32_t offset_data   = little_endianize_int32(*(uint32_t *)(&img[32])); /* Data Table   */

	desc_pc_sector sects[256];
	uint8_t den[256];
	uint8_t spt, ssize;
	uint32_t header, discop3;
	std::vector<uint32_t> track_data;

	int track_count = (offset_sector - offset_track) / 4;

	for (int track = 0; track < track_count; track++) {
		uint32_t track_offset = little_endianize_int32(*(uint32_t *)(&img[offset_track + (track * 4)]));
		spt = 0;
		discop3 = 0;
		den[0] = 2;

		if (track_offset == 0xffffffff) /* unformatted track */
		{
			sects[0].track  = track / 2;
			sects[0].head   = track % 2;
			sects[0].sector = 0;
			sects[0].size   = 0;
		}
		else
		{
			for (int i = 0; i < 256; i++) {
				header = little_endianize_int32(*(uint32_t *)(&img[offset_sector + track_offset + (i * 8)])); /* Sector Header */
				if (header == 0xffffffff)
					break;

				uint32_t data_offset = little_endianize_int32(*(uint32_t *)(&img[offset_sector + track_offset + (i * 8) + 4]));

				den[i] = (header >> 16) & 0x0f; /* sector density */
				if (den[i] != 2)
				{
					osd_printf_warning("jfd_dsk: track %d sector %d is %s density, not supported\n", track, i, den[0] == 1 ? "single" : "quad");
				}
				if (((header >> 20) & 0x0f) == 0x01) /* DiscOp3 data */
				{
					discop3 = little_endianize_int32(*(uint32_t *)(&img[offset_data + data_offset]));
					for (int i = 0; i < discop3; i++)
					{
						mfm_w(track_data, 8, img[offset_data + data_offset + 4 + i]);
					}
					osd_printf_warning("jfd_dsk: track %d contains DiscOp3 data (@%x), not supported\n", track, offset_data + data_offset);
					break;
				}
				else
				{
					spt = i + 1;
					sects[i].track   = track / 2;
					sects[i].head    = track % 2;
					sects[i].sector  = (header >> 8) & 0xff;
					sects[i].bad_crc = (header >> 4) & 0x02;
					sects[i].deleted = false;
					ssize = header & 0x03;
					sects[i].size = ssize;
					if (data_offset != 0xffffffff)
					{
						sects[i].actual_size = 128 << ssize;
						sects[i].data = &img[offset_data + data_offset];
					}
					else
					{
						sects[i].actual_size = 0;
					}
					if (((header >> 24) & 0xff) != 0xff) /* Sector in sector */
					{
						osd_printf_warning("jfd_dsk: track %d sector %d specifies time of sector from start of index marker, not supported\n", track, i);
					}
				}
			}
		}

		if (discop3)
		{
			generate_track_from_levels(track / 2, track % 2, track_data, 0, image);
		}
		else
		{
			if (den[0] == 1)
				build_wd_track_fm(track / 2, track % 2, image, 50000, spt, sects, 10, 40, 10);
			else
				build_wd_track_mfm(track / 2, track % 2, image, den[0] * 50000, spt, sects, 90, 32, 22);
		}
	}
	image.set_variant(floppy_image::DSDD);

	return true;
}

const jfd_format FLOPPY_JFD_FORMAT;
