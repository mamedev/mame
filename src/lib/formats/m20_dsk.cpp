// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m20_dsk.cpp

    Olivetti M20 floppy-disk images

    Track 0/head 0 is FM, 128 byte sectors. The rest is MFM,
    256 byte sectors.
    In image files the sectors of track 0/sector 0 are 256 bytes
    long to simplify access. Only the first half of these sectors
    contain image data.

*********************************************************************/

#include "m20_dsk.h"

#include "ioprocs.h"


m20_format::m20_format()
{
}

const char *m20_format::name() const noexcept
{
	return "m20";
}

const char *m20_format::description() const noexcept
{
	return "M20 disk image";
}

const char *m20_format::extensions() const noexcept
{
	return "img";
}

bool m20_format::supports_save() const noexcept
{
	return true;
}

int m20_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == 286720)
		return FIFID_SIZE;

	return 0;
}

bool m20_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	for (int track = 0; track < 35; track++)
		for (int head = 0; head < 2; head ++) {
			bool mfm = track || head;
			desc_pc_sector sects[16];
			uint8_t sectdata[16*256];
			/*auto const [err, actual] =*/ read_at(io, 16*256*(track*2+head), sectdata, 16*256); // FIXME: check for errors and premature EOF
			for (int i = 0; i < 16; i++) {
				int j = i/2 + (i & 1 ? 0 : 8);
				sects[i].track = track;
				sects[i].head = head;
				sects[i].sector = j+1;
				sects[i].size = mfm ? 1 : 0;
				sects[i].actual_size = mfm ? 256 : 128;
				sects[i].data = sectdata + 256*j;
				sects[i].deleted = false;
				sects[i].bad_crc = false;
			}

			if(mfm)
				build_wd_track_mfm(track, head, image, 100000, 16, sects, 50, 32, 22);
			else
				build_wd_track_fm(track, head, image, 50000, 16, sects, 24, 16, 11);
		}

	return true;
}

bool m20_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	uint64_t file_offset = 0;

	int track_count, head_count;
	track_count = 35; head_count = 2;  //FIXME: use image.get_actual_geometry(track_count, head_count) instead

	// initial FM track
	auto bitstream = generate_bitstream_from_track(0, 0, 4000, image);
	auto sectors = extract_sectors_from_bitstream_fm_pc(bitstream);

	for (int i = 0; i < 16; i++) {
		/*auto const [err, actual] =*/ write_at(io, file_offset, sectors[i + 1].data(), 128); // FIXME: check for errors
		file_offset += 256; //128;
	}

	// rest are MFM tracks
	for (int track = 0; track < track_count; track++) {
		for (int head = 0; head < head_count; head++) {
			// skip track 0, head 0
			if (track == 0) {
				if (head_count == 1) break;
				else head++;
			}

			bitstream = generate_bitstream_from_track(track, head, 2000, image);
			sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);

			for (int i = 0; i < 16; i++) {
				/*auto const [err, actual] =*/ write_at(io, file_offset, sectors[i + 1].data(), 256); // FIXME: check for errors
				file_offset += 256;
			}
		}
	}

	return true;
}

const m20_format FLOPPY_M20_FORMAT;
