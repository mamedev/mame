// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pc98fdi_dsk.h

    PC98FDI disk images

*********************************************************************/

#include <assert.h>
#include "pc98fdi_dsk.h"

pc98fdi_format::pc98fdi_format()
{
}

const char *pc98fdi_format::name() const
{
	return "pc98_fdi";
}

const char *pc98fdi_format::description() const
{
	return "PC98 FDI disk image";
}

const char *pc98fdi_format::extensions() const
{
	return "fdi";
}

int pc98fdi_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);
	UINT8 h[32];

	io_generic_read(io, h, 0, 32);
	UINT32 hsize = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0x8));
	UINT32 psize = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0xc));
	UINT32 ssize = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0x10));
	UINT32 scnt = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0x14));
	UINT32 sides = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0x18));
	UINT32 ntrk = LITTLE_ENDIANIZE_INT32(*(UINT32 *) (h + 0x1c));
	if(size == hsize + psize && psize == ssize*scnt*sides*ntrk)
		return 100;

	return 0;
}

bool pc98fdi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 h[32];

	io_generic_read(io, h, 0, 32);

	UINT32 hsize         = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x8));
	UINT32 sector_size   = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x10));
	UINT32 sector_count  = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x14));
	UINT32 head_count    = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x18));
	UINT32 track_count   = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x1c));

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize;
	for(ssize=0; (128 << ssize) < sector_size; ssize++) {};

	desc_pc_sector sects[256];
	UINT8 sect_data[65536];

	for(int track=0; track < track_count; track++)
		for(int head=0; head < head_count; head++) {
			io_generic_read(io, sect_data, hsize + sector_size*sector_count*(track*head_count + head), sector_size*sector_count);

			for(int i=0; i<sector_count; i++) {
				sects[i].track       = track;
				sects[i].head        = head;
				sects[i].sector      = i+1;
				sects[i].size        = ssize;
				sects[i].actual_size = sector_size;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = sect_data + i*sector_size;
			}

			build_pc_track_mfm(track, head, image, cell_count, sector_count, sects, calc_default_pc_gap3_size(form_factor, sector_size));
		}

	return true;
}

bool pc98fdi_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_PC98FDI_FORMAT = &floppy_image_format_creator<pc98fdi_format>;
