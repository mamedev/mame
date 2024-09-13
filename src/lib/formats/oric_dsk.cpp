// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/oric_dsk.cpp

    Oric disk images

*********************************************************************/

#include "oric_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"
#include "osdcore.h"

#include <cstring>


const char *oric_dsk_format::name() const noexcept
{
	return "oric_dsk";
}

const char *oric_dsk_format::description() const noexcept
{
	return "Oric disk image";
}

const char *oric_dsk_format::extensions() const noexcept
{
	return "dsk";
}

bool oric_dsk_format::supports_save() const noexcept
{
	return true;
}

int oric_dsk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[256];
	/*auto const [err, actual] =*/ read_at(io, 0, h, 256); // FIXME: check for errors and premature EOF

	if(memcmp(h, "MFM_DISK", 8))
		return 0;

	int sides  = get_u32le(&h[ 8]);
	int tracks = get_u32le(&h[12]);
	int geom   = get_u32le(&h[16]);

	uint64_t size;
	if(io.length(size))
		return 0;
	if(sides < 0 || sides > 2 || geom != 1 || size != 256+6400*sides*tracks)
		return 0;

	return FIFID_SIGN|FIFID_SIZE|FIFID_STRUCT;
}

bool oric_dsk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t h[256];
	uint8_t t[6250+3];

	t[6250] = t[6251] = t[6252] = 0;
	read_at(io, 0, h, 256); // FIXME: check for errors and premature EOF

	int sides  = get_u32le(&h[ 8]);
	int tracks = get_u32le(&h[12]);

	int max_tracks, max_sides;
	image.get_maximal_geometry(max_tracks, max_sides);
	if (tracks > max_tracks) {
		osd_printf_error("oric_dsk: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", tracks, max_tracks);
		return false;
	}
	if (sides > max_sides) {
		osd_printf_warning("oric_dsk: Floppy disk has excess of heads for this drive that will be discarded (floppy heads=%d, drive heads=%d).\n", sides, max_sides);
		sides = max_sides;
	}

	for(int side=0; side<sides; side++)
		for(int track=0; track<tracks; track++) {
			read_at(io, 256+6400*(tracks*side + track), t, 6250); // FIXME: check for errors and premature EOF
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


const char *oric_jasmin_format::name() const noexcept
{
	return "oric_jasmin";
}

const char *oric_jasmin_format::description() const noexcept
{
	return "Oric Jasmin pure sector image";
}

const char *oric_jasmin_format::extensions() const noexcept
{
	return "dsk";
}

bool oric_jasmin_format::supports_save() const noexcept
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

bool oric_jasmin_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if(io.length(size))
		return false;

	bool const can_ds = variants.empty() || has_variant(variants, floppy_image::DSDD);

	if(size != 41*17*256 && (!can_ds || size != 41*17*256*2))
		return false;

	int const heads = size == 41*17*256 ? 1 : 2;

	auto const [err, data, actual] = read_at(io, 0, size);
	if(err || (actual != size))
		return false;

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
				sdesc[s].data = &data[256 * (sector + track*17 + head*17*41)];
				sdesc[s].deleted = false;
				sdesc[s].bad_crc = false;
			}

			build_wd_track_mfm(track, head, image, 100000, 17, sdesc, 38, 40);
		}

	image.set_form_variant(floppy_image::FF_3, heads == 2 ? floppy_image::DSDD : floppy_image::SSDD);

	return true;
}

bool oric_jasmin_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_actual_geometry(tracks, heads);

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
				/*auto const [err, actual] =*/ write_at(io, 256 * (sector + track*17 + head*17*41), data, 256); // FIXME: check for errors
			}
		}
	return true;
}

const oric_jasmin_format FLOPPY_ORIC_JASMIN_FORMAT;
