// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ep64_tap.cpp

    Enterprise Sixty Four tape image format

    This is the tape image format used by ep128emu, which stores the
    sampled cassette signal rather than the decoded file data.

    The image starts with a 4096 byte header of big endian 32-bit
    words:

    0       magic (0x0275cd72)
    1       magic (0x1c445126)
    2       bits per sample (1, 2, 4 or 8)
    3       sample rate in Hz (10000 to 120000)
    4-1023  cue point sample positions, unused entries are 0xffffffff

    The remainder of the file is the sample data, packed into bytes
    with the earliest sample in the most significant bits.

*********************************************************************/

#include "ep64_tap.h"

#include "multibyte.h"

#include <algorithm>
#include <cstdint>
#include <vector>


namespace {

constexpr uint32_t EP64_TAP_MAGIC0 = 0x0275cd72;
constexpr uint32_t EP64_TAP_MAGIC1 = 0x1c445126;

constexpr unsigned EP64_TAP_HEADER_SIZE = 4096;
constexpr unsigned EP64_TAP_CUE_POINTS = 1020;

constexpr unsigned EP64_TAP_CHUNK_SAMPLES = 8192;


bool ep64_tap_parse_header(const uint8_t *header, uint32_t &bits_per_sample, uint32_t &sample_rate)
{
	if ((get_u32be(&header[0]) != EP64_TAP_MAGIC0) || (get_u32be(&header[4]) != EP64_TAP_MAGIC1))
		return false;

	bits_per_sample = get_u32be(&header[8]);
	sample_rate = get_u32be(&header[12]);

	if ((bits_per_sample != 1) && (bits_per_sample != 2) && (bits_per_sample != 4) && (bits_per_sample != 8))
		return false;

	if ((sample_rate < 10000) || (sample_rate > 120000))
		return false;

	// the last cue point slot is never used, so it doubles as a sanity check
	if (get_u32be(&header[12 + (EP64_TAP_CUE_POINTS * 4)]) != 0xffffffffU)
		return false;

	return true;
}


cassette_image::error ep64_tap_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	if (cassette->image_size() <= EP64_TAP_HEADER_SIZE)
		return cassette_image::error::INVALID_IMAGE;

	uint8_t header[EP64_TAP_HEADER_SIZE];
	cassette->image_read(header, 0, EP64_TAP_HEADER_SIZE);

	uint32_t bits_per_sample, sample_rate;

	if (!ep64_tap_parse_header(header, bits_per_sample, sample_rate))
		return cassette_image::error::INVALID_IMAGE;

	opts->channels = 1;
	opts->bits_per_sample = 16;
	opts->sample_frequency = sample_rate;

	return cassette_image::error::SUCCESS;
}


cassette_image::error ep64_tap_load(cassette_image *cassette)
{
	uint8_t header[EP64_TAP_HEADER_SIZE];
	cassette->image_read(header, 0, EP64_TAP_HEADER_SIZE);

	uint32_t bits_per_sample, sample_rate;

	if (!ep64_tap_parse_header(header, bits_per_sample, sample_rate))
		return cassette_image::error::INVALID_IMAGE;

	const uint64_t data_size = cassette->image_size() - EP64_TAP_HEADER_SIZE;
	const uint64_t total_samples = (data_size * 8) / bits_per_sample;
	const uint8_t max_value = (1 << bits_per_sample) - 1;

	std::vector<uint8_t> packed(EP64_TAP_CHUNK_SAMPLES * bits_per_sample / 8);
	std::vector<int8_t> samples(EP64_TAP_CHUNK_SAMPLES);

	for (uint64_t pos = 0; pos < total_samples; pos += EP64_TAP_CHUNK_SAMPLES)
	{
		const size_t count = std::min<uint64_t>(total_samples - pos, EP64_TAP_CHUNK_SAMPLES);

		cassette->image_read(&packed[0], EP64_TAP_HEADER_SIZE + (pos * bits_per_sample / 8), (count * bits_per_sample + 7) / 8);

		for (size_t i = 0; i < count; i++)
		{
			const unsigned bit = i * bits_per_sample;
			const uint8_t value = (packed[bit >> 3] >> (8 - bits_per_sample - (bit & 7))) & max_value;

			samples[i] = int8_t((value * 255 / max_value) - 128);
		}

		const cassette_image::error err = cassette->put_samples(0, double(pos) / sample_rate, double(count) / sample_rate,
			count, 1, &samples[0], cassette_image::WAVEFORM_8BIT);

		if (err != cassette_image::error::SUCCESS)
			return err;
	}

	return cassette_image::error::SUCCESS;
}


const cassette_image::Format ep64_tap_format =
{
	"tap",
	ep64_tap_identify,
	ep64_tap_load,
	nullptr
};

} // anonymous namespace


CASSETTE_FORMATLIST_START(ep64_cassette_formats)
	CASSETTE_FORMAT(ep64_tap_format)
CASSETTE_FORMATLIST_END
