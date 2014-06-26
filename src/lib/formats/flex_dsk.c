/*
 * flex_dsk.c  -  FLEX compatible disk images
 *
 *  Created on: 24/06/2014
 */

#include "emu.h"
#include "flex_dsk.h"

flex_format::flex_format() : wd177x_format(formats)
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

int flex_format::identify(io_generic *io, UINT32 form_factor)
{
	io_generic_read(io, &info, 256 * 2, sizeof(struct sysinfo_sector));

	if(((info.last_trk+1) * info.last_sec) * 256 == io_generic_size(io))
	{
		logerror("flex_dsk: %i tracks, %i sectors\n",info.last_trk+1,info.last_sec);
		return 100;
	}
	return 0;
}

bool flex_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int spt = info.last_sec;
	int bps = 256;
	int cell_count = 100000;
	int offset = 0;

	for(int track=0; track < info.last_trk+1; track++)
	{
		desc_pc_sector sects[80];
		UINT8 sect_data[20000];
		int sdatapos = 0;
		for(int i=0; i<spt; i++)
		{
			sects[i].track       = track;
			sects[i].head        = 0;  // no side select?
			sects[i].sector      = i+1;
			sects[i].size        = 1;
			sects[i].actual_size = bps;
			sects[i].deleted     = false;
			sects[i].bad_crc     = false;
			sects[i].data        = &sect_data[sdatapos];
			io_generic_read(io, sects[i].data, offset, bps);
			offset += bps;
			sdatapos += bps;
		}
		// gap sizes unverified
		build_wd_track_fm(track, 0, image, cell_count, spt, sects, 24, 16, 11);
	}
	return true;
}

const floppy_format_type FLOPPY_FLEX_FORMAT = &floppy_image_format_creator<flex_format>;
