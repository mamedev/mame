// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "hxcmfm_dsk.h"

#include "ioprocs.h"

#include <cstring>


#define MFM_FORMAT_HEADER   "HXCMFM"

#pragma pack(1)

struct MFMIMG
{
	uint8_t headername[7];

	uint16_t number_of_track;
	uint8_t number_of_side;

	uint16_t floppyRPM;
	uint16_t floppyBitRate;
	uint8_t floppyiftype;

	uint32_t mfmtracklistoffset;
};

struct MFMTRACKIMG
{
	uint16_t track_number;
	uint8_t side_number;
	uint32_t mfmtracksize;
	uint32_t mfmtrackoffset;
};

#pragma pack()

mfm_format::mfm_format() : floppy_image_format_t()
{
}

const char *mfm_format::name() const
{
	return "mfm";
}

const char *mfm_format::description() const
{
	return "HxCFloppyEmulator floppy disk image";
}

const char *mfm_format::extensions() const
{
	return "mfm";
}

bool mfm_format::supports_save() const
{
	return true;
}

int mfm_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t header[7];

	size_t actual;
	io.read_at(0, &header, sizeof(header), actual);
	if ( memcmp( header, MFM_FORMAT_HEADER, 6 ) ==0) {
		return 100;
	}
	return 0;
}

bool mfm_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	size_t actual;
	MFMIMG header;
	MFMTRACKIMG trackdesc;

	// read header
	io.read_at(0, &header, sizeof(header), actual);

	int drivecyl, driveheads;
	image->get_maximal_geometry(drivecyl, driveheads);
	bool skip_odd = (drivecyl < 50 && header.number_of_track >= 80);

	int counter = 0;
	std::vector<uint8_t> trackbuf;
	for(int track=0; track < header.number_of_track; track++) {
		for(int side=0; side < header.number_of_side; side++) {
			if (!skip_odd || track%2 == 0) {
				// read location of
				io.read_at((header.mfmtracklistoffset)+( counter *sizeof(trackdesc)), &trackdesc, sizeof(trackdesc), actual);

				trackbuf.resize(trackdesc.mfmtracksize);

				// actual data read
				io.read_at(trackdesc.mfmtrackoffset, &trackbuf[0], trackdesc.mfmtracksize, actual);

				if (skip_odd) {
					generate_track_from_bitstream(track/2, side, &trackbuf[0], trackdesc.mfmtracksize*8, image);
				}
				else {
					generate_track_from_bitstream(track, side, &trackbuf[0], trackdesc.mfmtracksize*8, image);
				}
			}

			counter++;
		}
	}

	image->set_variant(floppy_image::DSDD);
	return true;
}

bool mfm_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	// TODO: HD support
	size_t actual;
	MFMIMG header;
	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	memcpy(&header.headername, MFM_FORMAT_HEADER, 7);
	header.number_of_track = track_count;
	header.number_of_side = head_count;
	header.floppyRPM = 0;
	header.floppyBitRate = 250;
	header.floppyiftype = 4;
	header.mfmtracklistoffset = sizeof(MFMIMG);

	io.write_at(0, &header, sizeof(MFMIMG), actual);

	int tpos = sizeof(MFMIMG);
	int dpos = tpos + track_count*head_count*sizeof(MFMTRACKIMG);

	for(int track=0; track < track_count; track++) {
		for(int side=0; side < head_count; side++) {
			auto trackbuf = generate_bitstream_from_track(track, side, 2000, image);
			std::vector<uint8_t> packed((trackbuf.size() + 7) >> 3, 0);
			for(uint32_t i = 0; i != trackbuf.size(); i++)
				if(trackbuf[i])
					packed[i >> 3] |= 0x80 >> (i & 7);

			MFMTRACKIMG trackdesc;
			trackdesc.track_number = track;
			trackdesc.side_number = side;
			trackdesc.mfmtracksize = packed.size();
			trackdesc.mfmtrackoffset = dpos;

			io.write_at(tpos, &trackdesc, sizeof(MFMTRACKIMG), actual);
			io.write_at(dpos, packed.data(), packed.size(), actual);

			tpos += sizeof(MFMTRACKIMG);
			dpos += packed.size();
		}
	}

	return true;
}

const floppy_format_type FLOPPY_MFM_FORMAT = &floppy_image_format_creator<mfm_format>;
