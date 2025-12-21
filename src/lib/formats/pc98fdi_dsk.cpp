// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pc98fdi_dsk.cpp

    PC98FDI disk images

*********************************************************************/

#include "pc98fdi_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <tuple>


pc98fdi_format::pc98fdi_format()
{
}

const char *pc98fdi_format::name() const noexcept
{
	return "pc98_fdi";
}

const char *pc98fdi_format::description() const noexcept
{
	return "PC98 FDI disk image";
}

const char *pc98fdi_format::extensions() const noexcept
{
	return "fdi";
}

int pc98fdi_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	uint8_t h[32];
	auto const [err, actual] = read_at(io, 0, h, 32);
	if(err || (32 != actual))
		return 0;

	uint32_t const hsize = get_u32le(h + 0x8);
	uint32_t const psize = get_u32le(h + 0xc);
	uint32_t const ssize = get_u32le(h + 0x10);
	uint32_t const scnt = get_u32le(h + 0x14);
	uint32_t const sides = get_u32le(h + 0x18);
	uint32_t const ntrk = get_u32le(h + 0x1c);
	if(size == hsize + psize && psize == ssize*scnt*sides*ntrk)
		return FIFID_STRUCT;

	return 0;
}

bool pc98fdi_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;

	uint8_t h[32];
	std::tie(err, actual) = read_at(io, 0, h, 32);
	if(err || (32 != actual))
		return false;

	uint32_t const hsize         = get_u32le(h + 0x8);
	uint32_t const sector_size   = get_u32le(h + 0x10);
	uint32_t const sector_count  = get_u32le(h + 0x14);
	uint32_t const head_count    = get_u32le(h + 0x18);
	uint32_t const track_count   = get_u32le(h + 0x1c);

	int const cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize = 0;
	while ((128 << ssize) < sector_size)
		ssize++;

	desc_pc_sector sects[256];
	uint8_t sect_data[65536];

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			std::tie(err, actual) = read_at(io, hsize + sector_size*sector_count*(track*head_count + head), sect_data, sector_size*sector_count); // FIXME: check for errors and premature EOF

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
	}

	return true;
}

const pc98fdi_format FLOPPY_PC98FDI_FORMAT;
