// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss
/***************************************************************************

    Brother LW-30 Disk image format
    see https://github.com/BartmanAbyss/brother-diskconv for disk conversion tool

***************************************************************************/
#include "lw30_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <array>
#include <cassert>
#include <cstring>


namespace {
	constexpr uint16_t sync_table[]{
		0xdaef, 0xb7ad, 0xfbbe, 0xeadf, 0xbffa, 0xaeb6, 0xf5d7, 0xdbee, 0xbaab, 0xfdbd,
		0xebde, 0xd5f7, 0xafb5, 0xf6d6, 0xdded, 0xbbaa, 0xedbb, 0xd6dd, 0xb5f6, 0xf7af,
		0xded5, 0xbdeb, 0xabfd, 0xeeba, 0xd7db, 0xb6f5, 0xfaae, 0xdfbf, 0xbeea, 0xadfb,
		0xefb7, 0xdada, 0xb7ef, 0xfbad, 0xeabe, 0xbfdf, 0xaefa, 0xf5b6, 0xdbd7, 0xbaee,
		0xfdab, 0xebbd, 0xd5de, 0xaff7, 0xf6b5, 0xddd6, 0xbbed, 0xaadd, 0xedf6, 0xd6af,
		0xb5d5, 0xf7eb, 0xdefd, 0xbdba, 0xabdb, 0xeef5, 0xd7ae, 0xb6bf, 0xfaea, 0xdffb,
		0xbeb7, 0xadda, 0xefef, 0xdaad, 0xb7be, 0xfbdf, 0xeafa, 0xbfb6, 0xaed7, 0xf5ee,
		0xdbab, 0xbabd, 0xfdde, 0xebf7, 0xd5b5, 0xafd6, 0xf6ed, 0xddaa, 0xd6bb, 0xb5dd
	};

	constexpr uint8_t gcr_table[]{
		0xaa, 0xab, 0xad, 0xae, 0xaf, 0xb5, 0xb6, 0xb7,
		0xba, 0xbb, 0xbd, 0xbe, 0xbf, 0xd5, 0xd6, 0xd7,
		0xda, 0xdb, 0xdd, 0xde, 0xdf, 0xea, 0xeb, 0xed,
		0xee, 0xef, 0xf5, 0xf6, 0xf7, 0xfa, 0xfb, 0xfd,
		0xfe, 0xff // FE, FF are reserved
	};

	// format
	constexpr uint8_t sector_prefix[]{
		0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xab
	};

	// write
	constexpr uint8_t sector_header[]{
		0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xed
	};

	// write
	constexpr uint8_t sector_footer[]{
		0xf5, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd
	};

	constexpr uint8_t sector_interleave1[]{ // 1-based
		1, 6, 11, 4, 9, 2, 7, 12, 5, 10, 3, 8
	};

	constexpr uint8_t sector_interleave2[]{ // 1-based
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
	};

	constexpr uint8_t sector_interleave3[]{ // 1-based
		1, 4, 7, 10, 6, 9, 12, 3, 11, 2, 5, 8
	};

	void gcr_encode_5_to_8(const uint8_t *input, uint8_t *output)
	{
		// input:
		// 76543210
		// --------
		// 00000111 0
		// 11222223 1
		// 33334444 2
		// 45555566 3
		// 66677777 4

		output[0] = gcr_table[(input[0] >> 3) & 0x1f];
		output[1] = gcr_table[((input[0] << 2) & 0x1f) | ((input[1] >> 6) & 0b00000011)];
		output[2] = gcr_table[(input[1] >> 1) & 0x1f];
		output[3] = gcr_table[((input[1] << 4) & 0x1f) | ((input[2] >> 4) & 0b00001111)];
		output[4] = gcr_table[((input[2] << 1) & 0x1f) | ((input[3] >> 7) & 0b00000001)];
		output[5] = gcr_table[(input[3] >> 2) & 0x1f];
		output[6] = gcr_table[((input[3] << 3) & 0x1f) | ((input[4] >> 5) & 0b00000111)];
		output[7] = gcr_table[input[4] & 0x1f];
	}

	std::array<uint8_t, 3> checksum_256_bytes(const uint8_t *input)
	{
		size_t i = 0;
		uint8_t a = 0;
		uint8_t c = input[i++];
		uint8_t d = input[i++];
		uint8_t e = input[i++];
		for(size_t b = 0; b < 253; b++) {
			a = d;
			if(c & 0b10000000)
				a ^= 1;
			d = c;
			c = a;
			a = (d << 1) ^ e;
			e = d;
			d = a;
			e ^= input[i++];
		}

		return { c, d, e };
	}

	std::array<uint8_t, 416> gcr_encode_and_checksum(const uint8_t *input /* 256 bytes */) {
		std::array<uint8_t, 416> output;
		for(int i = 0; i < 51; i++)
			gcr_encode_5_to_8(&input[i * 5], &output[i * 8]);

		auto checksum = checksum_256_bytes(input);
		std::array<uint8_t, 5> end_and_checksum{ input[255], checksum[0], checksum[1], checksum[2], 0x58 };
		gcr_encode_5_to_8(&end_and_checksum[0], &output[408]);

		return output;
	}
}

static constexpr int TRACKS_PER_DISK = 78;
static constexpr int SECTORS_PER_TRACK = 12;
static constexpr int SECTOR_SIZE = 256;

static constexpr int RPM = 300;
static constexpr int CELLS_PER_REV = 250'000 / (RPM / 60);

// format track: 0xaa (2), 0xaa (48), 12*sector
// format sector: sector_prefix (8), track_sync (2), sector_sync (2), predata (19), payload=0xaa (414), postdata (13), 0xaa (42), should come out to ~4,000 bits
// write sector: (after sector_sync, 0xdd) sector_header (2+14), payload (416), sector_footer (11)

// from write_format, write_sector_data_header_data_footer
static constexpr int raw_sector_size = 8/*sector_prefix*/ + 2/*track_sync*/ + 2/*sector_sync*/ + 1/*0xdd*/ + 16/*sector_header*/ + 416/*payload*/ + 11/*sector_footer*/ + 42/*0xaa*/;
static constexpr int raw_track_size = 2/*0xaa*/ + 48/*0xaa*/ + SECTORS_PER_TRACK * raw_sector_size;

int lw30_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size = 0;
	if(io.length(size))
		return 0;

	if(size == TRACKS_PER_DISK * SECTORS_PER_TRACK * SECTOR_SIZE)
		return FIFID_SIZE; // identified by size

	return 0;
}

bool lw30_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t trackdata[SECTORS_PER_TRACK * SECTOR_SIZE], rawdata[CELLS_PER_REV / 8];
	memset(rawdata, 0xaa, sizeof(rawdata));
	for(int track = 0; track < TRACKS_PER_DISK; track++) {
		auto const [err, actual] = read_at(io, track * SECTORS_PER_TRACK * SECTOR_SIZE, trackdata, SECTORS_PER_TRACK * SECTOR_SIZE);
		if(err || (actual != SECTORS_PER_TRACK * SECTOR_SIZE))
			return false;
		size_t i = 0;
		for(int x = 0; x < 2 + 48; x++)
			rawdata[i++] = 0xaa;
		auto interleave_offset = (track % 4) * 4;
		for(size_t s = interleave_offset; s < interleave_offset + SECTORS_PER_TRACK; s++) {
			auto sector = sector_interleave1[s % SECTORS_PER_TRACK] - 1;
			// according to check_track_and_sector
			for(const auto& d : sector_prefix) // 8 bytes
				rawdata[i++] = d;
			put_u16le(&rawdata[i], sync_table[track]);
			i += 2;
			put_u16le(&rawdata[i], sync_table[sector]);
			i += 2;
			rawdata[i++] = 0xdd;
			for(const auto& d : sector_header) // 16 bytes
				rawdata[i++] = d;
			auto payload = gcr_encode_and_checksum(trackdata + sector * SECTOR_SIZE); // 256 -> 416 bytes
			for(const auto &d : payload)
				rawdata[i++] = d;
			for(const auto &d : sector_footer) // 11 bytes
				rawdata[i++] = d;
			for(int x = 0; x < 42; x++)
				rawdata[i++] = 0xaa;
		}
		assert(i == raw_track_size);
		assert(i <= CELLS_PER_REV / 8);
		generate_track_from_bitstream(track, 0, rawdata, CELLS_PER_REV, image);
	}

	image.set_variant(floppy_image::SSDD);

	return true;
}

const char *lw30_format::name() const noexcept
{
	return "lw30";
}

const char *lw30_format::description() const noexcept
{
	return "Brother LW-30 floppy disk image";
}

const char *lw30_format::extensions() const noexcept
{
	return "img";
}

bool lw30_format::supports_save() const noexcept
{
	// TODO
	return false;
}

const lw30_format FLOPPY_LW30_FORMAT;
