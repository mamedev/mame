// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/fdd_dsk.h

    PC98 FDD disk images

    0xC3FC header, followed by track data
    Sector map starts at offset 0xDC, with 12bytes for each sector

    Each entry of the sector map has the following structure
    - 0x0 = track number (if 0xff the sector/track is unformatted/unused)
    - 0x1 = head number
    - 0x2 = sector number
    - 0x3 = sector size (128 << this byte)
    - 0x4 = fill byte. if it's not 0xff, then this sector in the original
            disk consisted of this single value repeated for the whole
            sector size, and the sector is skipped in the .fdd file.
            if it's 0xff, then this sector is wholly contained in the .fdd
            file
    - 0x5 = ??
    - 0x6 = ??
    - 0x7 = ??
    - 0x8-0x0b = absolute offset of the data for this sector, or 0xfffffff
                 if the sector was skipped in the .fdd (and it has to be
                 filled with the value at 0x4)

 TODO:
    - Investigate remaining sector map bytes (maybe related to protections?)

*********************************************************************/

#include <assert.h>

#include "fdd_dsk.h"

fdd_format::fdd_format()
{
}

const char *fdd_format::name() const
{
	return "fdd";
}

const char *fdd_format::description() const
{
	return "FDD disk image";
}

const char *fdd_format::extensions() const
{
	return "fdd";
}

int fdd_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[7];
	io_generic_read(io, h, 0, 7);

	if (strncmp((const char *)h, "VFD1.0", 6) == 0)
		return 100;

	return 0;
}

bool fdd_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 hsec[0x0c];

	// sector map
	UINT8 num_secs[160];
	UINT8 tracks[160 * 26];
	UINT8 heads[160 * 26];
	UINT8 secs[160 * 26];
	UINT8 fill_vals[160 * 26];
	UINT32 sec_offs[160 * 26];
	UINT8 sec_sizes[160 * 26];

	int pos = 0xdc;

	for (int track = 0; track < 160; track++)
	{
		int curr_num_sec = 0, curr_track_size = 0;
		for (int sect = 0; sect < 26; sect++)
		{
			// read sector map for this sector
			io_generic_read(io, hsec, pos, 0x0c);
			pos += 0x0c;

			if (hsec[0] == 0xff)    // unformatted/unused sector
				continue;

			tracks[(track * 26) + sect] = hsec[0];
			heads[(track * 26) + sect] = hsec[1];
			secs[(track * 26) + sect] = hsec[2];
			sec_sizes[(track * 26) + sect] = hsec[3];
			fill_vals[(track * 26) + sect] = hsec[4];
			sec_offs[(track * 26) + sect] = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(hsec + 0x08));

			curr_track_size += (128 << hsec[3]);
			curr_num_sec++;
		}
		num_secs[track] = curr_num_sec;
	}

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;
	desc_pc_sector sects[256];
	UINT8 sect_data[65536];
	int cur_sec_map = 0, sector_size;

	for (int track = 0; track < 160; track++)
	{
		int cur_pos = 0;
		for (int i = 0; i < num_secs[track]; i++)
		{
			cur_sec_map = track * 26 + i;
			sector_size = 128 << sec_sizes[cur_sec_map];

			if (sec_offs[cur_sec_map] == 0xffffffff)
				memset(sect_data + cur_pos, fill_vals[cur_sec_map], sector_size);
			else
				io_generic_read(io, sect_data + cur_pos, sec_offs[cur_sec_map], sector_size);

			sects[i].track       = tracks[cur_sec_map];
			sects[i].head        = heads[cur_sec_map];
			sects[i].sector      = secs[cur_sec_map];
			sects[i].size        = sec_sizes[cur_sec_map];
			sects[i].actual_size = sector_size;
			sects[i].deleted     = false;
			sects[i].bad_crc     = false;
			sects[i].data        = sect_data + cur_pos;
			cur_pos += sector_size;
		}

		build_pc_track_mfm(track / 2, track % 2, image, cell_count, num_secs[track], sects, calc_default_pc_gap3_size(form_factor, (128 << sec_sizes[track * 26])));
	}

	return true;
}

bool fdd_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_FDD_FORMAT = &floppy_image_format_creator<fdd_format>;
