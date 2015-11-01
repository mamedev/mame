// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * flex_dsk.c  -  FLEX compatible disk images
 *
 *  Created on: 24/06/2014
 */

	#include "emu.h" // logerror
#include "flex_dsk.h"

flex_format::flex_format()
{
}

const char *flex_format::name() const
{
	return "flex";
}

const char *flex_format::description() const
{
	return "FLEX compatible disk image";
}

const char *flex_format::extensions() const
{
	return "dsk";
}

bool flex_format::supports_save() const
{
	return true;
}

int flex_format::identify(io_generic *io, UINT32 form_factor)
{
	io_generic_read(io, &info, 256 * 2, sizeof(struct sysinfo_sector));

	if(((info.last_trk+1) * info.last_sec) * 256 == io_generic_size(io))
	{
		osd_printf_verbose("flex_dsk: %i tracks, %i sectors\n",info.last_trk+1,info.last_sec);
		return 100;
	}
	return 0;
}

bool flex_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int spt = info.last_sec;
	int bps = 256;
	int cell_count = (form_factor == floppy_image::FF_525) ? 50000 : 100000;
	int offset = 0;
	int head_num = 1;
	int total_tracks = info.last_trk+1;
	bool double_sided = false;

	if(total_tracks == 40 && spt == 36)
		double_sided = true;
	if(total_tracks == 77 && spt == 30)
		double_sided = true;
	if(total_tracks == 80 && spt == 40)  // 800kB
		double_sided = true;
	if(total_tracks == 80 && spt == 72)  // 1.44MB
		double_sided = true;
	if(spt >= 20)
		double_sided = true;

	if(double_sided)
	{
		spt = spt / 2;
		head_num = 2;
	}

	for(int track=0; track < total_tracks; track++)
		for(int head=0;head < head_num;head++)
		{
			desc_pc_sector sects[80];
			UINT8 sect_data[20000];
			int sdatapos = 0;
			for(int i=0; i<spt; i++)
			{
				sects[i].track       = track;
				sects[i].head        = head;  // no side select?
				if(head == 0)
					sects[i].sector      = i+1;
				else
					sects[i].sector      = i+1+spt;
				sects[i].actual_size = bps;
				sects[i].size        = 1;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = &sect_data[sdatapos];
				io_generic_read(io, sects[i].data, offset, bps);
				offset += bps;
				sdatapos += bps;
			}
			// gap sizes unverified
			if(total_tracks == 35 && spt == 18 && (track >= 1 && track <= 2))  // handle Gimix Flex 3.6 disk image, which the boot sector loads tracks 1 and 2 as MFM
				build_wd_track_mfm(track, head, image, cell_count*2, spt, sects, 50, 32, 22);
			else
				build_wd_track_fm(track, head, image, cell_count, spt, sects, 24, 16, 11);
		}
	return true;
}

const floppy_format_type FLOPPY_FLEX_FORMAT = &floppy_image_format_creator<flex_format>;
