// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Carl

#include "86f_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cstring>
#include <tuple>


namespace {

#define _86F_FORMAT_HEADER   "86BF"

#pragma pack(1)

struct _86FIMG
{
	uint8_t headername[4];
	uint8_t minor_version;
	uint8_t major_version;
	uint16_t flags;
	uint32_t firsttrackoffs;
};

#pragma pack()

enum
{
	SURFACE_DESC = 1,
	TYPE_MASK = 6,
	TYPE_DD = 0,
	TYPE_HD = 2,
	TYPE_ED = 4,
	TYPE_ED2000 = 6,
	TWO_SIDES = 8,
	WRITE_PROTECT = 0x10,
	RPM_MASK = 0x60,
	RPM_0 = 0,
	RPM_1 = 0x20,
	RPM_15 = 0x40,
	RPM_2 = 0x60,
	EXTRA_BC = 0x80,
	ZONED_RPM = 0x100,
	ZONE_PREA2_1 = 0,
	ZONE_PREA2_2 = 0x200,
	ZONE_A2 = 0x400,
	ZONE_C64 = 0x600,
	ENDIAN_BIG = 0x800,
	RPM_FAST = 0x1000,
	TOTAL_BC = 0x1000
};

} // anonymous namespace


_86f_format::_86f_format() : floppy_image_format_t()
{
}

const char *_86f_format::name() const noexcept
{
	return "86f";
}

const char *_86f_format::description() const noexcept
{
	return "86f floppy image";
}

const char *_86f_format::extensions() const noexcept
{
	return "86f";
}

int _86f_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[4];
	auto const [err, actual] = read_at(io, 0, &header, sizeof(header));
	if (err) {
		return 0;
	}
	if (!memcmp(header, _86F_FORMAT_HEADER, 4)) {
		return FIFID_SIGN;
	}
	return 0;
}

void _86f_format::generate_track_from_bitstream_with_weak(int track, int head, const uint8_t *trackbuf, const uint8_t *weak, int index_cell, int track_size, floppy_image &image) const
{
	int j = 0;
	std::vector<uint32_t> &dest = image.get_buffer(track, head);
	dest.clear();

	for(int i=index_cell; i != track_size; i++, j++) {
		int databit = trackbuf[i >> 3] & (0x80 >> (i & 7));
		int weakbit = weak ? weak[i >> 3] & (0x80 >> (i & 7)) : 0;
		if(weakbit && databit)
			dest.push_back(floppy_image::MG_D | (j*2+1));
		else if(weakbit && !databit)
			dest.push_back(floppy_image::MG_N | (j*2+1));
		else if(databit)
			dest.push_back(floppy_image::MG_F | (j*2+1));
	}
	for(int i=0; i != index_cell; i++, j++) {
		int databit = trackbuf[i >> 3] & (0x80 >> (i & 7));
		int weakbit = weak ? weak[i >> 3] & (0x80 >> (i & 7)) : 0;
		if(weakbit && databit)
			dest.push_back(floppy_image::MG_D | (j*2+1));
		else if(weakbit && !databit)
			dest.push_back(floppy_image::MG_N | (j*2+1));
		else if(databit)
			dest.push_back(floppy_image::MG_F | (j*2+1));
	}

	normalize_times(dest, track_size*2);
	image.set_write_splice_position(track, head, 0, 0);
}

bool _86f_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;
	_86FIMG header;

	// read header
	std::tie(err, actual) = read_at(io, 0, &header, sizeof(header));
	if(err || (actual != sizeof(header)))
		return false;

	int drivecyl, driveheads;
	image.get_maximal_geometry(drivecyl, driveheads);
	bool skip_odd = drivecyl < 50;
	int imagesides = header.flags & TWO_SIDES ? 2 : 1;
	int sides = (imagesides == 2) && (driveheads == 2) ? 2 : 1;

	std::vector<uint32_t> tracklist;
	int tracklistsize = header.firsttrackoffs - 8;
	tracklist.resize(tracklistsize / 4);
	std::tie(err, actual) = read_at(io, 8, &tracklist[0], tracklistsize);
	if(err || (actual != tracklistsize))
		return false;

	uint32_t tracklen = 0;
	if((header.flags & (TOTAL_BC | EXTRA_BC | RPM_MASK)) != (TOTAL_BC | EXTRA_BC)) {
		switch(header.flags & (RPM_MASK | RPM_FAST | TYPE_MASK)) {
			case TYPE_DD | RPM_2:
			case TYPE_HD | RPM_2:
				tracklen = 12750;
				break;
			case TYPE_DD | RPM_15:
			case TYPE_HD | RPM_15:
				tracklen = 12687;
				break;
			case TYPE_DD | RPM_1:
			case TYPE_HD | RPM_1:
				tracklen = 12625;
				break;
			case TYPE_DD | RPM_0:
			case TYPE_HD | RPM_0:
				tracklen = 12500;
				break;
			case TYPE_DD | RPM_1 | RPM_FAST:
			case TYPE_HD | RPM_1 | RPM_FAST:
				tracklen = 12376;
				break;
			case TYPE_DD | RPM_15 | RPM_FAST:
			case TYPE_HD | RPM_15 | RPM_FAST:
				tracklen = 12315;
				break;
			case TYPE_DD | RPM_2 | RPM_FAST:
			case TYPE_HD | RPM_2 | RPM_FAST:
				tracklen = 12254;
				break;

			case TYPE_ED | RPM_2:
				tracklen = 25500;
				break;
			case TYPE_ED | RPM_15:
				tracklen = 25375;
				break;
			case TYPE_ED | RPM_1:
				tracklen = 25250;
				break;
			case TYPE_ED | RPM_0:
				tracklen = 25000;
				break;
			case TYPE_ED | RPM_1 | RPM_FAST:
				tracklen = 25752;
				break;
			case TYPE_ED | RPM_15 | RPM_FAST:
				tracklen = 24630;
				break;
			case TYPE_ED | RPM_2 | RPM_FAST:
				tracklen = 24509;
				break;
		}
	}
	uint32_t trackoff;
	int trackinfolen = header.flags & EXTRA_BC ? 10 : 6;
	if(skip_odd)
		drivecyl *= 2;
	std::vector<uint8_t> trackbuf;
	std::vector<uint8_t> weakbuf;
	int track;
	for(track=0; (track < (tracklistsize / 4)) && (track < drivecyl); track++) {
		for(int side=0; side < sides; side++) {
			trackoff = tracklist[(track * imagesides) + side];
			if(!trackoff) break;
			if(!skip_odd || track%2 == 0) {
				uint8_t trackinfo[10];
				std::tie(err, actual) = read_at(io, trackoff, &trackinfo, trackinfolen); // FIXME: check for errors and premature EOF
				if(err || (actual != trackinfolen))
					return false;
				uint32_t bitcells = tracklen << 4;
				uint32_t index_cell;
				if(header.flags & EXTRA_BC)
				{
					uint32_t extra = get_u32le(trackinfo + 2);
					index_cell = get_u32le(trackinfo + 6);
					if((header.flags & TOTAL_BC) && !tracklen)
						bitcells = extra;
					else
						bitcells += (int32_t)extra;
				}
				else
					index_cell = get_u32le(trackinfo + 2);
				uint32_t fulltracklen = (bitcells >> 3) + (bitcells & 7 ? 1 : 0);
				uint8_t *weak = nullptr;
				trackbuf.resize(fulltracklen);
				std::tie(err, actual) = read_at(io, trackoff + trackinfolen, &trackbuf[0], fulltracklen);
				if(err || (actual != fulltracklen))
					return false;
				if(header.flags & SURFACE_DESC)
				{
					weakbuf.resize(fulltracklen);
					std::tie(err, actual) = read_at(io, trackoff + trackinfolen + fulltracklen, &weakbuf[0], fulltracklen);
					if(err || (actual != fulltracklen))
						return false;
					weak = &weakbuf[0];
				}
				if(skip_odd) {
					generate_track_from_bitstream_with_weak(track/2, side, &trackbuf[0], weak, index_cell, bitcells, image);
				}
				else {
					generate_track_from_bitstream_with_weak(track, side, &trackbuf[0], weak, index_cell, bitcells, image);
				}
			}
		}
		if(!trackoff) break;
	}

	if(imagesides == 2)
		image.set_variant(track > 50 ? floppy_image::DSHD : floppy_image::DSDD);
	else
		image.set_variant(floppy_image::SSDD);
	return true;
}

/*
bool _86f_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
    return true;
}
*/
const _86f_format FLOPPY_86F_FORMAT;
