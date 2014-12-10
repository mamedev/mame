// license:BSD-3-Clause
// copyright-holders:etabeta
/*********************************************************************

    formats/pc98dip_dsk.h

    PC98DIP disk images
 
    0x100 header, followed by track data

    TODO:
    - Investigate header structure 
    - can this format be used to support different disc types?

*********************************************************************/

#include "emu.h"
#include "pc98dip_dsk.h"

pc98dip_format::pc98dip_format()
{
}

const char *pc98dip_format::name() const
{
	return "pc98_dip";
}

const char *pc98dip_format::description() const
{
	return "PC98 DIP disk image";
}

const char *pc98dip_format::extensions() const
{
	return "dip";
}

int pc98dip_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);

	if (size == 0x134000 + 0x100)
		return 100;

	return 0;
}

bool pc98dip_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int heads, tracks, spt, bps;

	//For the moment we only support this disk structure...
	//2 sides, 77 tracks, 8 sectors/track, 1024 bytes/sector = 1261568 bytes (360rpm)
	heads = 2;
	tracks = 77;
	spt = 8;
	bps = 1024;
	
	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;
	
	int ssize;
	for (ssize = 0; (128 << ssize) < bps; ssize++);
	
	desc_pc_sector sects[256];
	UINT8 sect_data[bps * spt];
	
	for (int track = 0; track < tracks; track++)
		for (int head = 0; head < heads; head++) 
		{
			io_generic_read(io, sect_data, 0x100 + bps * spt * (track * heads + head), bps * spt);
			
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

	return true;
}

bool pc98dip_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_PC98DIP_FORMAT = &floppy_image_format_creator<pc98dip_format>;
