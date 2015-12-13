// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/dcp_dsk.h

    PC98 DCP & DCU disk images

    0xA2 header, followed by track data
    header[0] - disk format
    header[1-0xA1] - track map (1=track used, 0=track unused/unformatted)
    header[0xA2] - all tracks used?
                   (there seems to be a diff in its usage between DCP and DCU)

    TODO:
     - add support for track map. images available for tests were all
       of type 0x01, with all 154 tracks present. combined with pete_j
       reporting some images have faulty track map, we need some more
       test cases to properly handle these disks!

*********************************************************************/

#include <assert.h>

#include "dcp_dsk.h"

dcp_format::dcp_format()
{
}

const char *dcp_format::name() const
{
	return "dcx";
}

const char *dcp_format::description() const
{
	return "DCP/DCU disk image";
}

const char *dcp_format::extensions() const
{
	return "dcp,dcu";
}

int dcp_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);
	UINT8 h[0xa2];
	int heads, tracks, spt, bps, count_tracks = 0;
	bool is_hdb = false;

	io_generic_read(io, h, 0, 0xa2);

	// First byte is the disk format (see below in load() for details)
	switch (h[0])
	{
		case 0x01:
		default:
			heads = 2; tracks = 77;
			spt = 8; bps = 1024;
			break;
		case 0x02:
			heads = 2; tracks = 80;
			spt = 15; bps = 512;
			break;
		case 0x03:
			heads = 2; tracks = 80;
			spt = 18; bps = 512;
			break;
		case 0x04:
			heads = 2; tracks = 80;
			spt = 8; bps = 512;
			break;
		case 0x05:
			heads = 2; tracks = 80;
			spt = 9; bps = 512;
			break;
		case 0x08:
			heads = 2; tracks = 80;
			spt = 9; bps = 1024;
			break;
		case 0x11:
			is_hdb = true;
			heads = 2; tracks = 77;
			spt = 26; bps = 256;
			break;
		case 0x19:
			heads = 2; tracks = 80;
			spt = 16; bps = 256;
			break;
		case 0x21:
			heads = 2; tracks = 80;
			spt = 26; bps = 256;
			break;
	}

	// bytes 0x01 to 0xa1 are track map (0x01 if track is used, 0x00 if track is unformatted/unused)
	for (int i = 1; i < 0xa1; i++)
		if (h[i])
			count_tracks++;

	// in theory track map should be enough (former check), but some images have it wrong!
	// hence, if this check fails, we also allow for images with all tracks and wrong track map
	if (size - 0xa2 == (heads * count_tracks * spt * bps) || size - 0xa2 == (heads * tracks * spt * bps))
		return 100;

	// for disk type 0x11 the head 0 track 0 has 26 sectors of half width, so we need to compensate calculation
	if (is_hdb && (size - 0xa2 + (0x80 * 26) == (heads * count_tracks * spt * bps) || size - 0xa2 + (0x80 * 26) == (heads * tracks * spt * bps)))
		return 100;

	return 0;
}

bool dcp_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 h[0xa2];
	int heads, tracks, spt, bps;
	bool is_hdb = false;

	io_generic_read(io, h, 0, 0xa2);

	// First byte is the disk format:
	switch (h[0])
	{
		case 0x01:
		default:
			//01h: 2HD-8 sector (1.25MB) (BKDSK .HDM) (aka 2HS)
			//2 sides, 77 tracks, 8 sectors/track, 1024 bytes/sector = 1261568 bytes (360rpm)
			heads = 2;
			tracks = 77;
			spt = 8;
			bps = 1024;
			break;
		case 0x02:
			//02H: 2HD-15 sector (1.21MB) (BKDSK .HD5) (aka 2HC)
			//2 sides, 80 tracks, 15 sectors/track, 512 bytes/sector = 1228800 bytes (360rpm)
			heads = 2;
			tracks = 80;
			spt = 15;
			bps = 512;
			break;
		case 0x03:
			//03H: 2HQ-18 sector (1.44MB) (BKDSK .HD4) (aka 2HDE)
			//2 sides, 80 tracks, 18 sectors/track, 512 bytes/sector = 1474560 bytes (300rpm)
			heads = 2;
			tracks = 80;
			spt = 18;
			bps = 512;
			break;
		case 0x04:
			//04H: 2DD-8 sector (640KB) (BKDSK .DD6)
			//2 sides, 80 tracks, 8 sectors/track, 512 bytes/sector = 655360 bytes (300rpm)
			heads = 2;
			tracks = 80;
			spt = 8;
			bps = 512;
			break;
		case 0x05:
			//05h: 2DD-9 sector ( 720KB) (BKDSK .DD9)
			//2 sides, 80 tracks, 9 sectors/track, 512 bytes/sector = 737280 bytes (300rpm)
			heads = 2;
			tracks = 80;
			spt = 9;
			bps = 512;
			break;
		case 0x08:
			//08h: 2HD-9 sector (1.44MB)
			//2 sides, 80 tracks, 9 sectors/track, 1024 bytes/sector = 1474560 bytes (300rpm)(??)
			heads = 2;
			tracks = 80;
			spt = 9;
			bps = 1024;
			break;
		case 0x11:
			//11h: BASIC-2HD (BKDSK .HDB)
			//Head 0 Track 0 - FM encoding - 26 sectors of 128 bytes = 1 track
			//Head 1 Track 0 - MFM encoding - 26 sectors of 256 bytes = 1 track
			//Head 0 Track 1 to Head 1 Track 77 - 26 sectors of 256 bytes = 152 tracks
			//2 sides, 77 tracks, 26 sectors/track, 256 bytes/sector (except for head 0 track 0) = 1021696 bytes (360rpm)
			is_hdb = true;
			heads = 2;
			tracks = 77;
			spt = 26;
			bps = 256;
			break;
		case 0x19:
			//19h: BASIC 2DD (BKDSK .DDB)
			//2 sides, 80 tracks, 16 sectors/track, 256 bytes/sector = 655360 bytes (300rpm)
			heads = 2;
			tracks = 80;
			spt = 16;
			bps = 256;
			break;
		case 0x21:
			//21H: 2HD-26 sector
			//2 sides, 80 tracks, 26 sectors/track, 256 bytes/sector = 1064960 bytes (??rpm)(??)
			heads = 2;
			tracks = 80;
			spt = 26;
			bps = 256;
			break;
	}

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize;
	for (ssize = 0; (128 << ssize) < bps; ssize++) {};

	desc_pc_sector sects[256];
	UINT8 sect_data[65536];

	if (!is_hdb)
	{
		for (int track = 0; track < tracks; track++)
			for (int head = 0; head < heads; head++)
			{
				io_generic_read(io, sect_data, 0xa2 + bps * spt * (track * heads + head), bps * spt);

				for (int i = 0; i < spt; i++)
				{
					sects[i].track       = track;
					sects[i].head        = head;
					sects[i].sector      = i + 1;
					sects[i].size        = ssize;
					sects[i].actual_size = bps;
					sects[i].deleted     = false;
					sects[i].bad_crc     = false;
					sects[i].data        = sect_data + i * bps;
				}

				build_pc_track_mfm(track, head, image, cell_count, spt, sects, calc_default_pc_gap3_size(form_factor, bps));
			}
	}
	else    // FIXME: the code below is untested, because no image was found... there might be some silly mistake in the disk geometry!
	{
		// Read Head 0 Track 0 is FM with 26 sectors of 128bytes instead of 256
		io_generic_read(io, sect_data, 0xa2, 128 * spt);

		for (int i = 0; i < spt; i++)
		{
			sects[i].track       = 0;
			sects[i].head        = 0;
			sects[i].sector      = i + 1;
			sects[i].size        = 0;
			sects[i].actual_size = 128;
			sects[i].deleted     = false;
			sects[i].bad_crc     = false;
			sects[i].data        = sect_data + i * 128;
		}

		build_pc_track_fm(0, 0, image, cell_count, spt, sects, calc_default_pc_gap3_size(form_factor, 128));

		// Read Head 1 Track 0 is MFM with 26 sectors of 256bytes
		io_generic_read(io, sect_data, 0xa2 + 128 * spt, bps * spt);

		for (int i = 0; i < spt; i++)
		{
			sects[i].track       = 0;
			sects[i].head        = 1;
			sects[i].sector      = i + 1;
			sects[i].size        = ssize;
			sects[i].actual_size = bps;
			sects[i].deleted     = false;
			sects[i].bad_crc     = false;
			sects[i].data        = sect_data + i * bps;
		}

		build_pc_track_mfm(0, 1, image, cell_count, spt, sects, calc_default_pc_gap3_size(form_factor, bps));

		// Read other tracks as usual
		UINT32 data_offs = 0xa2 + (26 * 0x80) + (26 * 0x100);
		for (int track = 1; track < tracks; track++)
			for (int head = 0; head < heads; head++)
			{
				io_generic_read(io, sect_data, data_offs + bps * spt * ((track - 1) * heads + head), bps * spt);

				for (int i = 0; i < spt; i++)
				{
					sects[i].track       = track;
					sects[i].head        = head;
					sects[i].sector      = i + 1;
					sects[i].size        = ssize;
					sects[i].actual_size = bps;
					sects[i].deleted     = false;
					sects[i].bad_crc     = false;
					sects[i].data        = sect_data + i * bps;
				}

				build_pc_track_mfm(track, head, image, cell_count, spt, sects, calc_default_pc_gap3_size(form_factor, bps));
			}
	}

	return true;
}

bool dcp_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_DCP_FORMAT = &floppy_image_format_creator<dcp_format>;
