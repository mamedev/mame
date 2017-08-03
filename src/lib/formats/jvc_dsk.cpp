// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    JVC

    Disk image format

    Named after its creator, Jeff Vavasour

    TODO:
    - Support writing unusal formats?

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
	case 0:	tracks = (size - header_size) / sector_size / sectors / heads;
		break;
	}

	// os-9 format disk images often don't have a header, but can contain
	// various geometries. we try to open the file as os-9 image here and
	// see if the values we get make sense
	if (header_size == 0 && size > 0x20)
	{
		uint8_t os9_header[0x20];
		io_generic_read(io, os9_header, 0, 0x20);

		int os9_total_sectors = pick_integer_be(os9_header, 0x00, 3);
		int os9_heads = BIT(os9_header[0x10], 0) ? 2 : 1;
		int os9_sectors = pick_integer_be(os9_header, 0x11, 2);
		int os9_tracks = os9_total_sectors / os9_sectors / os9_heads;

		// now let's see if we have valid info
		if ((os9_tracks * os9_heads * os9_sectors * 256) == size)
		{
			tracks = os9_tracks;
			heads = os9_heads;
			sectors = os9_sectors;

			osd_printf_verbose("OS-9 format disk image detected.\n");
		}
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

			for (int i = 0; i < sector_count; i++)
			{
				sectors[i].track = track;
				sectors[i].head = head;
				sectors[i].sector = sector_base_id + i;
				sectors[i].actual_size = sector_size;
				sectors[i].size = sector_size >> 8;
				sectors[i].deleted = false;
				sectors[i].bad_crc = false;
				sectors[i].data = &sector_data[sector_offset];

				io_generic_read(io, sectors[i].data, file_offset, sector_size);

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
