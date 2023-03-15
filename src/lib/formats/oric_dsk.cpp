// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/oric_dsk.c

    Oric disk images

*********************************************************************/

#include "oric_dsk.h"

#include "ioprocs.h"

#include <cstring>


const char *oric_dsk_format::name() const
{
	return "oric_dsk";
}

const char *oric_dsk_format::description() const
{
	return "Oric disk image";
}

const char *oric_dsk_format::extensions() const
{
	return "dsk";
}

bool oric_dsk_format::supports_save() const
{
	return true;
}

int oric_dsk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[256];
	size_t actual;
	io.read_at(0, h, 256, actual);

	if(memcmp(h, "MFM_DISK", 8))
		return 0;

	int sides  = (h[11] << 24) | (h[10] << 16) | (h[ 9] << 8) | h[ 8];
	int tracks = (h[15] << 24) | (h[14] << 16) | (h[13] << 8) | h[12];
	int geom   = (h[19] << 24) | (h[18] << 16) | (h[17] << 8) | h[16];

	uint64_t size;
	if(io.length(size))
		return 0;
	if(sides < 0 || sides > 2 || geom != 1 || size != 256+6400*sides*tracks)
		return 0;

	return FIFID_SIGN|FIFID_SIZE|FIFID_STRUCT;
}

bool oric_dsk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	size_t actual;
	uint8_t h[256];
	uint8_t t[6250+3];

	t[6250] = t[6251] = t[6252] = 0;
	io.read_at(0, h, 256, actual);

	int sides  = (h[11] << 24) | (h[10] << 16) | (h[ 9] << 8) | h[ 8];
	int tracks = (h[15] << 24) | (h[14] << 16) | (h[13] << 8) | h[12];

	for(int side=0; side<sides; side++)
		for(int track=0; track<tracks; track++) {
			io.read_at(256+6400*(tracks*side + track), t, 6250, actual);
			std::vector<uint32_t> stream;
			int sector_size = 128;
			for(int i=0; i<6250; i++) {
				if(t[i] == 0xc2 && t[i+1] == 0xc2 && t[i+2] == 0xc2) {
					raw_w(stream, 16, 0x5224);
					raw_w(stream, 16, 0x5224);
					raw_w(stream, 16, 0x5224);
					i += 2;
					continue;
				}
				if(t[i] == 0xa1 && t[i+1] == 0xa1 && t[i+2] == 0xa1) {
					raw_w(stream, 16, 0x4489);
					raw_w(stream, 16, 0x4489);
					raw_w(stream, 16, 0x4489);
					int copy;
					if(t[i+3] == 0xfe) {
						copy = 7;
						sector_size = 128 << (t[i+7] & 3);
					} else if(t[i+3] == 0xfb)
						copy = sector_size+3;
					else
						copy = 0;
					for(int j=0; j<copy; j++)
						mfm_w(stream, 8, t[i+3+j]);
					i += 2+copy;
					continue;
				}
				mfm_w(stream, 8, t[i]);
			}
			generate_track_from_levels(track, side, stream, 0, image);
		}

	return true;
}

const oric_dsk_format FLOPPY_ORIC_DSK_FORMAT;


const char *oric_jasmin_format::name() const
{
	return "oric_jasmin";
}

const char *oric_jasmin_format::description() const
{
	return "Oric Jasmin pure sector image";
}

const char *oric_jasmin_format::extensions() const
{
	return "dsk";
}

bool oric_jasmin_format::supports_save() const
{
	return true;
}

int oric_jasmin_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	bool const can_ds = variants.empty() || has_variant(variants, floppy_image::DSDD);
	if(size == 41*17*256 || (can_ds && size == 41*17*256*2))
		return FIFID_SIZE;

	return 0;
}

bool oric_jasmin_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if(io.length(size))
		return false;

	bool const can_ds = variants.empty() || has_variant(variants, floppy_image::DSDD);

	if(size != 41*17*256 && (!can_ds || size != 41*17*256*2))
		return false;

	int const heads = size == 41*17*256 ? 1 : 2;

	std::vector<uint8_t> data(size);
	size_t actual;
	io.read_at(0, data.data(), size, actual);

	for(int head = 0; head != heads; head++)
		for(int track = 0; track != 41; track++) {
			desc_pc_sector sdesc[17];
			for(int s = 0; s != 17; s++) {
				int sector = (s + 6*(track+1)) % 17;
				sdesc[s].track = track;
				sdesc[s].head = head;
				sdesc[s].sector = sector + 1;
				sdesc[s].size = 1;
				sdesc[s].actual_size = 256;
				sdesc[s].data = data.data() + 256 * (sector + track*17 + head*17*41);
				sdesc[s].deleted = false;
				sdesc[s].bad_crc = false;
			}

			build_wd_track_mfm(track, head, image, 100000, 17, sdesc, 38, 40);
		}

	image->set_form_variant(floppy_image::FF_3, heads == 2 ? floppy_image::DSDD : floppy_image::SSDD);

	return true;
}

bool oric_jasmin_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);

	bool can_ds = variants.empty() || has_variant(variants, floppy_image::DSDD);
	if(heads == 2 && !can_ds)
		return false;
	if(heads == 0)
		heads = 1;

	uint8_t zero[256];
	memset(zero, 0, 256);

	for(int head = 0; head != heads; head++)
		for(int track = 0; track != 41; track++) {
			auto sectors = extract_sectors_from_bitstream_mfm_pc(generate_bitstream_from_track(track, head, 2000, image));
			for(unsigned int sector = 0; sector != 17; sector ++) {
				uint8_t const *const data = (sector+1 < sectors.size() && !sectors[sector+1].empty()) ? sectors[sector+1].data() : zero;
				size_t actual;
				io.write_at(256 * (sector + track*17 + head*17*41), data, 256, actual);
			}
		}
	return true;
}

const oric_jasmin_format FLOPPY_ORIC_JASMIN_FORMAT;
