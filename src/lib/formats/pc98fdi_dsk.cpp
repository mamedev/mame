// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pc98fdi_dsk.h

    PC98FDI disk images

*********************************************************************/

#include <cassert>
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

int pc98fdi_format::identify(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);
	uint8_t h[32];

	io_generic_read(io, h, 0, 32);
	uint32_t hsize = little_endianize_int32(*(uint32_t *) (h + 0x8));
	uint32_t psize = little_endianize_int32(*(uint32_t *) (h + 0xc));
	uint32_t ssize = little_endianize_int32(*(uint32_t *) (h + 0x10));
	uint32_t scnt = little_endianize_int32(*(uint32_t *) (h + 0x14));
	uint32_t sides = little_endianize_int32(*(uint32_t *) (h + 0x18));
	uint32_t ntrk = little_endianize_int32(*(uint32_t *) (h + 0x1c));
	if(size == hsize + psize && psize == ssize*scnt*sides*ntrk)
		return 100;

	return 0;
}

bool pc98fdi_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	uint8_t h[32];

	io_generic_read(io, h, 0, 32);

	uint32_t hsize         = little_endianize_int32(*(uint32_t *)(h+0x8));
	uint32_t sector_size   = little_endianize_int32(*(uint32_t *)(h+0x10));
	uint32_t sector_count  = little_endianize_int32(*(uint32_t *)(h+0x14));
	uint32_t head_count    = little_endianize_int32(*(uint32_t *)(h+0x18));
	uint32_t track_count   = little_endianize_int32(*(uint32_t *)(h+0x1c));

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize;
	for(ssize=0; (128 << ssize) < sector_size; ssize++) {};

	desc_pc_sector sects[256];
	uint8_t sect_data[65536];

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
