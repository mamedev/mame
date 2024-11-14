// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*********************************************************************

    formats/nabupc_dsk.cpp

    NABU PC floppy-disk images

    Disk Layouts:
      200k - 1 head , 40 tracks, 5 1024 byte sectors per track
      400k - 2 heads, 40 tracks, 5 1024 byte sectors per track
      800k - 2 heads, 80 tracks, 5 1024 byte sectors per track
*********************************************************************/

#include "nabupc_dsk.h"

#include "imageutl.h"
#include "ioprocs.h"
#include "strformat.h"


const nabupc_format::format nabupc_format::formats[] =
{
	{ // 200k 40 track single sided double density (nabu)
		floppy_image::FF_525, floppy_image::SSDD,
		40, 1, 32, 22, 80,
		{0x00, 0x28, 0x00, 0x03, 0x07, 0x00, 0xc2, 0x00, 0x5f, 0x00, 0xe0, 0x00, 0x00, 0x18, 0x01, 0x00, 0x03, 0x07}
	},
	{ // 400k 40 track double sided double density (nabu)
		floppy_image::FF_525, floppy_image::DSDD,
		40, 2, 32, 22, 80,
		{0x01, 0x28, 0x00, 0x04, 0x0f, 0x01, 0xc4, 0x00, 0xbf, 0x00, 0xe0, 0x00, 0x00, 0x30, 0x01, 0x00, 0x03, 0x07}
	},
	{ // 800k 80 track double sided double density (nabu)
		floppy_image::FF_35, floppy_image::DSDD,
		80, 2, 32, 22, 80,
		{0x02, 0x28, 0x00, 0x04, 0x0f, 0x00, 0x8c, 0x01, 0x7f, 0x01, 0xfc, 0x00, 0x00, 0x61, 0x01, 0x00, 0x03, 0x07}
	},
	{}
};

nabupc_format::nabupc_format()
{
}

int nabupc_format::find_format(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size)) {
		return -1;
	}

	for (int i = 0; formats[i].form_factor; i++) {
		const format &f = formats[i];

		if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor) {
			continue;
		}
		if (!variants.empty() && !has_variant(variants, f.variant)) {
			continue;
		}
		if (size == (uint64_t)sector_size * sector_count * f.track_count * f.head_count) {
			LOG_FORMATS("nabupc: Found nabupc disk: %d\n", i);
			return i;
		}
		LOG_FORMATS("nabupc: no match\n");
	}
	return -1;
}

int nabupc_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{

	int const format = find_format(io, form_factor, variants);

	if (format == -1) {
		return 0;
	}

	return FIFID_SIZE;
}

void nabupc_format::build_nabu_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2, const uint8_t *dpb)
{
	std::vector<uint32_t> track_data;

	if (dpb[1] != 0 && track == 0 && head == 0) {
		for (int i = 0; i < 2; i++) raw_w(track_data, 16, 0x4489);
		raw_w(track_data, 16, 0x1054);  // 4e
		for (int i = 0; i < 11; i++) mfm_w(track_data, 8, 0x4e);
		mfm_w(track_data, 8, 0x19);
		mfm_w(track_data, 8, 0x0F);
		mfm_w(track_data, 8, 0x2D);
		mfm_w(track_data, 8, 0x1A);
		for (int i = 0; i < 18; i++) mfm_w(track_data, 8, dpb[i]);
	}

	for (int i = 0; i < gap_1; i++) mfm_w(track_data, 8, 0x4e);

	int total_size = 0;
	for (int i = 0; i < sector_count; i++) total_size += sects[i].actual_size;

	int etpos = int(track_data.size()) + (sector_count * (12 + 3 + 5 + 2 + gap_2 + 12 + 3 + 1+ 2) + total_size) * 16;

	if (etpos > cell_count) {
		throw std::invalid_argument(util::string_format("Incorrect layout on track %d head %d, expected_size=%d, current_size=%d", track, head, cell_count, etpos));
	}

	if (etpos + gap_3 * 16 * (sector_count - 1) > cell_count) {
		gap_3 = (cell_count - etpos) / 16 / (sector_count - 1);
	}

	// Build the track
	for (int i = 0; i < sector_count; i++) {
		uint16_t crc;
		// sync and IDAM and gap 2
		for (int j = 0; j < 12; j++) mfm_w(track_data, 8, 0x00);
		unsigned int cpos = track_data.size();
		for (int j = 0; j < 3; j++) raw_w(track_data, 16, 0x4489);
		mfm_w(track_data, 8, 0xfe);
		mfm_w(track_data, 8, sects[i].track);
		mfm_w(track_data, 8, sects[i].head);
		mfm_w(track_data, 8, sects[i].sector);
		mfm_w(track_data, 8, sects[i].size);
		crc = calc_crc_ccitt(track_data, cpos, track_data.size());
		mfm_w(track_data, 16, crc);
		for (int j = 0; j < gap_2; j++) mfm_w(track_data, 8, 0x4e);

		if (!sects[i].data) {
			for (int j = 0; j < 12 + 4 + sects[i].actual_size + 2 + (i != sector_count - 1 ? gap_3 : 0); j++) mfm_w(track_data, 8, 0x4e);
		} else {
			// sync, DAM, data and gap 3
			for (int j = 0; j < 12; j++) mfm_w(track_data, 8, 0x00);
			cpos = track_data.size();
			for (int j = 0; j < 3; j++) raw_w(track_data, 16, 0x4489);
			mfm_w(track_data, 8, sects[i].deleted ? 0xf8 : 0xfb);
			for (int j = 0; j < sects[i].actual_size; j++) mfm_w(track_data, 8, sects[i].data[j]);
			crc = calc_crc_ccitt(track_data, cpos, track_data.size());
			if (sects[i].bad_crc) {
				crc = 0xffff^crc;
			}
			mfm_w(track_data, 16, crc);
			if (i != sector_count - 1) {
				for (int j = 0; j < gap_3; j++) mfm_w(track_data, 8, 0x4e);
			}
		}
	}

	// Gap 4b

	while (int(track_data.size()) < cell_count - 15) mfm_w(track_data, 8, 0x4e);
	raw_w(track_data, cell_count - int(track_data.size()), 0x9254 >> (16 + int(track_data.size()) - cell_count));

	generate_track_from_levels(track, head, track_data, 0, image);
}

bool nabupc_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	int const type = find_format(io, form_factor, variants);

	if (type == -1) {
		return false;
	}

	const format &f = formats[type];

	for (int track = 0; track < f.track_count; track++) {
		for (int head = 0; head < f.head_count; head++) {
			desc_pc_sector sects[sector_count];
			uint8_t sectdata[sector_count * sector_size];
			/*auto const [err, actual] =*/ read_at(io, (track * f.head_count + head) * sector_count * sector_size, sectdata, sector_count * sector_size); // FIXME: check for errors and premature EOF
			for (int i = 0; i < sector_count; i++) {
				sects[i].track = track;
				sects[i].head = head;
				sects[i].sector = i + 1;
				sects[i].size = 3;
				sects[i].actual_size = sector_size;
				sects[i].data = sectdata + sector_size * i;
				sects[i].deleted = false;
				sects[i].bad_crc = false;
			}
			build_nabu_track_mfm(track, head, image, 100000, sector_count, sects, f.gap_3, f.gap_1, f.gap_2, f.dpb);
		}
	}
	return true;
}

bool nabupc_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{

	int heads, tracks;
	image.get_actual_geometry(tracks, heads);
	for (int track = 0; track < tracks; track++) {
		for (int head = 0; head < heads; head++) {
			uint64_t file_offset = (track * heads + head) * sector_count * sector_size;
			auto bitstream = generate_bitstream_from_track(track, head, 2000, image);
			auto sectors = extract_sectors_from_bitstream_mfm_pc(bitstream);
			for (int i = 0; i < sector_count; i++) {
				/*auto const [err, actual] =*/ write_at(io, file_offset, sectors[i + 1].data(), sector_size); // FIXME: check for errors
				file_offset += sector_size;
			}
		}
	}

	return true;
}

const char *nabupc_format::name() const noexcept
{
	return "nabupc";
}

const char *nabupc_format::description() const noexcept
{
	return "NABU PC CP/M Disk Image";
}

const char *nabupc_format::extensions() const noexcept
{
	return "img,dsk";
}

bool nabupc_format::supports_save() const noexcept
{
	return true;
}

const nabupc_format FLOPPY_NABUPC_FORMAT;
