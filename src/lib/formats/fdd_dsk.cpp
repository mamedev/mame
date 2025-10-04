// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/fdd_dsk.cpp

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

#include "fdd_dsk.h"

#include "ioprocs.h"

#include "osdcomm.h" // little_endianize_int32

#include <cstring>


fdd_format::fdd_format()
{
}

const char *fdd_format::name() const noexcept
{
	return "fdd";
}

const char *fdd_format::description() const noexcept
{
	return "FDD disk image";
}

const char *fdd_format::extensions() const noexcept
{
	return "fdd";
}

int fdd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[7];
	auto const [err, actual] = read_at(io, 0, h, 7); // FIXME: should it really be reading six bytes?  also check for premature EOF.
	if (err)
		return false;

	if (memcmp(h, "VFD1.0", 6) == 0)
		return FIFID_SIGN;

	return 0;
}

bool fdd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t hsec[0x0c];

	// sector map
	uint8_t num_secs[160];
	uint8_t tracks[160 * 26];
	uint8_t heads[160 * 26];
	uint8_t secs[160 * 26];
	uint8_t fill_vals[160 * 26];
	uint32_t sec_offs[160 * 26];
	uint8_t sec_sizes[160 * 26];

	int pos = 0xdc;

	for (int track = 0; track < 160; track++)
	{
		int curr_num_sec = 0; [[maybe_unused]] int curr_track_size = 0;
		for (int sect = 0; sect < 26; sect++)
		{
			// read sector map for this sector
			/*auto const [err, actual] =*/ read_at(io, pos, hsec, 0x0c); // FIXME: check for errors and premature EOF
			pos += 0x0c;

			if (hsec[0] == 0xff)    // unformatted/unused sector
				continue;

			tracks[(track * 26) + sect] = hsec[0];
			heads[(track * 26) + sect] = hsec[1];
			secs[(track * 26) + sect] = hsec[2];
			sec_sizes[(track * 26) + sect] = hsec[3];
			fill_vals[(track * 26) + sect] = hsec[4];
			sec_offs[(track * 26) + sect] = little_endianize_int32(*(uint32_t *)(hsec + 0x08));

			curr_track_size += (128 << hsec[3]);
			curr_num_sec++;
		}
		num_secs[track] = curr_num_sec;
	}

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;
	desc_pc_sector sects[256];
	uint8_t sect_data[65536];
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
				/*auto const [err, actual] =*/ read_at(io, sec_offs[cur_sec_map], sect_data + cur_pos, sector_size); // FIXME: check for errors and premature EOF

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

const fdd_format FLOPPY_FDD_FORMAT;
