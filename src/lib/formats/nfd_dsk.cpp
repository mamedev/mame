// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/nfd_dsk.cpp

    PC98 NFD disk images (info from: http://www.geocities.jp/t98next/dev.html )

    Revision 0
    ==========

    header structure (variable length > 0x120, header length = DWORD at 0x110)
    0x000-0x00F = T98FDDIMAGE.R* followed by 2 0x00 bytes, * = format revision (0 or 1 so far)
    0x010-0x10F = space for image info / comments
    0x110-0x113 = header length (DWORD)
    0x114       = write protect (any value > 0 means not writeable)
    0x115       = number of heads
    0x116-0x11F = reserved
    0x120-EOHeader = sector map (0x10 for each sector of the disk!)
    last 0x10 are fixed to 0x00, probably it marks the end of sector map?

    sector map structure
    0x0     = track number
    0x1     = head
    0x2     = sector number
    0x3     = sector size (in 128byte chunks)
    0x4     = MFM/FM (1 = MFM, 0 = FM)?
    0x5     = DDAM/DAM (1 = DDAM, 0 = DAM)
    0x6-0x9 = READ DATA (FDDBIOS) Results (Status, St0, St1, St2) ??
    0xA     = PDA (disk type)
    0xB-0xF = reserved and equal to 0x00 (possibly available for future format extensions?)


    Revision 1
    ==========

    header structure (variable length > 0x120, header length = DWORD at 0x110)
    0x000-0x11F = same as Rev. 0 format
    0x120-0x3AF = 164 DWORDs containing, for each track, the absolute position of the sector maps
                  for sectors of the track. for unformatted/unused tracks 0 is used
    0x3B0-0x3B3 = absolute position of addintional info in the header, if any
    0x3B4-0x3BF = reserved
    0x120-EOHeader = sector map + special data for each track:
                     first 0x10 of each track = #sectors (WORD), #extra data (WORD), reserved 0xc bytes zeroed
                     then 0x10 for each sector of this track and 0x10 for each extra data chunk

    sector map structure
    0x0     = track number
    0x1     = head
    0x2     = sector number
    0x3     = sector size (in 128byte chunks)
    0x4     = MFM/FM (1 = MFM, 0 = FM)?
    0x5     = DDAM/DAM (1 = DDAM, 0 = DAM)
    0x6-0x9 = READ DATA (FDDBIOS) Results (Status, St0, St1, St2) ??
    0xA     = RETRY DATA (1 = Yes, 0 = No)
    0xB     = PDA (disk type)
    0xC-0xF = reserved and equal to 0x00 (possibly available for future format extensions?)

    extra data map structure
    0x0     = command
    0x1     = track number
    0x2     = head
    0x3     = sector number
    0x4     = sector size (in 128byte chunks)
    0x5-0x8 = READ DATA (FDDBIOS) Results (Status, St0, St1, St2) ??
    0x9     = Number of times to RETRY loading data
    0xA-0xD = length of RETRY DATA
    0xE     = PDA (disk type)
    0xF     = reserved and equal to 0x00 (possibly available for future format extensions?)

    TODO:
    - add support for write protect header bit? apparently, some disks try to write and
      fail to boot if they succeed which is the reason this bit was added
    - add support for DDAM in Rev. 0 (need an image which set it in some sector)
    - investigate the READ DATA bytes of sector headers
    - investigate RETRY DATA chunks

 *********************************************************************/

#include "nfd_dsk.h"

#include "ioprocs.h"

#include "osdcomm.h" // little_endianize_int*

#include <cstring>


nfd_format::nfd_format()
{
}

const char *nfd_format::name() const noexcept
{
	return "nfd";
}

const char *nfd_format::description() const noexcept
{
	return "NFD disk image";
}

const char *nfd_format::extensions() const noexcept
{
	return "nfd";
}

int nfd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[16];
	auto const [err, actual] = read_at(io, 0, h, 16); // TODO: does it really need 16 bytes?  it only looks at 14.
	if (err || (16 != actual))
		return 0;

	if (!memcmp(h, "T98FDDIMAGE.R0", 14) || !memcmp(h, "T98FDDIMAGE.R1", 14))
		return FIFID_SIGN;

	return 0;
}

bool nfd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;
	uint8_t h[0x120], hsec[0x10];
	read_at(io, 0, h, 0x120); // FIXME: check for errors and premature EOF
	int format_version = !memcmp(h, "T98FDDIMAGE.R0", 14) ? 0 : 1;

	// sector map (the 164th entry is only used by rev.1 format, loops with track < 163 are correct for rev.0)
	uint8_t disk_type = 0;
	uint8_t num_secs[164];
	uint8_t num_specials[164];
	uint32_t track_sizes[164];
	uint8_t tracks[164 * 26];
	uint8_t heads[164 * 26];
	uint8_t secs[164 * 26];
	uint8_t mfm[164 * 26];
	uint8_t sec_sizes[164 * 26];

	uint32_t hsize = little_endianize_int32(*(uint32_t *)(h+0x110));

	int pos = 0x120;

	// set up sector map
	if (format_version == 1)
	{
		for (int track = 0; track < 164; track++)
		{
			int curr_track_size = 0;
			// read sector map absolute location
			read_at(io, pos, hsec, 4); // FIXME: check for errors and premature EOF
			pos += 4;
			uint32_t secmap_addr = little_endianize_int32(*(uint32_t *)(hsec));

			if (secmap_addr)
			{
				// read actual sector map for the sectors of this track
				// for rev.1 format the first 0x10 are a track summary:
				// first WORD is # of sectors, second WORD is # of special data sectors
				read_at(io, secmap_addr, hsec, 0x10); // FIXME: check for errors and premature EOF
				secmap_addr += 0x10;
				num_secs[track] = little_endianize_int16(*(uint16_t *)(hsec));
				num_specials[track] = little_endianize_int16(*(uint16_t *)(hsec + 0x2));

				for (int sect = 0; sect < num_secs[track]; sect++)
				{
					read_at(io, secmap_addr, hsec, 0x10); // FIXME: check for errors and premature EOF

					if (track == 0 && sect == 0)
						disk_type = hsec[0xb];  // can this change across the disk? I don't think so...
					secmap_addr += 0x10;

					tracks[(track * 26) + sect] = hsec[0];
					heads[(track * 26) + sect] = hsec[1];
					secs[(track * 26) + sect] = hsec[2];
					sec_sizes[(track * 26) + sect] = hsec[3];
					mfm[(track * 26) + sect] = hsec[4];

					curr_track_size += (128 << hsec[3]);
				}

				if (num_specials[track] > 0)
				{
					for (int sect = 0; sect < num_specials[track]; sect++)
					{
						read_at(io, secmap_addr, hsec, 0x10); // FIXME: check for errors and premature EOF
						secmap_addr += 0x10;
						curr_track_size += (hsec[9] + 1) * little_endianize_int32(*(uint32_t *)(hsec + 0x0a));
					}
				}
			}
			else
			{
				num_secs[track] = 0;
				num_specials[track] = 0;
			}
			track_sizes[track] = curr_track_size;
		}
	}
	else
	{
		for (int track = 0; track < 163 && pos < hsize; track++)
		{
			int curr_num_sec = 0, curr_track_size = 0;
			for (int sect = 0; sect < 26; sect++)
			{
				// read sector map for this sector
				// for rev.0 format each sector uses 0x10 bytes
				read_at(io, pos, hsec, 0x10); // FIXME: check for errors and premature EOF

				if (track == 0 && sect == 0)
					disk_type = hsec[0xa];  // can this change across the disk? I don't think so...
				pos += 0x10;

				if (hsec[0] == 0xff)    // unformatted/unused sector
					continue;

				tracks[(track * 26) + sect] = hsec[0];
				heads[(track * 26) + sect] = hsec[1];
				secs[(track * 26) + sect] = hsec[2];
				sec_sizes[(track * 26) + sect] = hsec[3];
				mfm[(track * 26) + sect] = hsec[4];

				curr_track_size += (128 << hsec[3]);
				curr_num_sec++;
			}

			num_secs[track] = curr_num_sec;
			track_sizes[track] = curr_track_size;
		}
	}

	// shouln't this be set-up depending on disk_type? gaplus does not like having less than 166666 cells
	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	switch (disk_type)
	{
		case 0x10:  // 640K disk, 2DD
			image.set_variant(floppy_image::DSDD);
			break;
		//case 0x30:    // 1.44M disk, ?? (no images found)
		//  break;
		case 0x90:  // 1.2M disk, 2HD
		default:
			image.set_variant(floppy_image::DSHD);
			break;
	}

	desc_pc_sector sects[256];
	uint8_t sect_data[65536];
	int cur_sec_map = 0, sector_size;
	pos = hsize;

	for (int track = 0; track < 163 && pos < size; track++)
	{
		read_at(io, pos, sect_data, track_sizes[track]); // FIXME: check for errors and premature EOF

		for (int i = 0; i < num_secs[track]; i++)
		{
			cur_sec_map = track * 26 + i;
			sector_size = 128 << sec_sizes[cur_sec_map];
			sects[i].track       = tracks[cur_sec_map];
			sects[i].head        = heads[cur_sec_map];
			sects[i].sector      = secs[cur_sec_map];
			sects[i].size        = sec_sizes[cur_sec_map];
			sects[i].actual_size = sector_size;
			sects[i].deleted     = false;
			sects[i].bad_crc     = false;
			sects[i].data        = sect_data + i * sector_size;
		}
		pos += track_sizes[track];

		// notice that the operation below might fail if sectors of the same track have variable sec_sizes,
		// because the gap3 calculation would account correctly only for the first sector...
		// examined images had constant sec_sizes in the each track, so probably this is not an issue
		if (mfm[track * 26])
			build_pc_track_mfm(track / 2, track % 2, image, cell_count, num_secs[track], sects, calc_default_pc_gap3_size(form_factor, (128 << sec_sizes[track * 26])));
		else
			build_pc_track_fm(track / 2, track % 2, image, cell_count / 2, num_secs[track], sects, calc_default_pc_gap3_size(form_factor, (128 << sec_sizes[track * 26])));
	}

	return true;
}

bool nfd_format::supports_save() const noexcept
{
	return false;
}

const nfd_format FLOPPY_NFD_FORMAT;
