// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    aiffile.cpp

    Format code for cassette images in Apple Computer's AIFF
    (Audio Interchange Format File) format.

*********************************************************************/

#include "aiffile.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "eminline.h"

#include <cstring>


namespace {

constexpr unsigned COMM_OFFSET = 0x0c;
constexpr unsigned SSND_OFFSET = 0x26;
constexpr unsigned SOUND_DATA_OFFSET = 0x36;

constexpr int MAX_CHANNELS = 8;


int encode_bits_per_sample(std::uint16_t bits_per_sample)
{
	if (bits_per_sample == 32)
		return cassette_image::WAVEFORM_32BITBE;
	else if (bits_per_sample == 16)
		return cassette_image::WAVEFORM_16BITBE;
	else
		return cassette_image::WAVEFORM_8BIT;
}


cassette_image::error aiffile_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	std::uint64_t length;
	if (cassette->get_raw_cassette_image()->length(length))
		return cassette_image::error::INVALID_IMAGE; // FIXME: return the actual error here

	const auto [err, chunk_header, header_length] = util::read_at(*cassette->get_raw_cassette_image(), 0, SOUND_DATA_OFFSET);
	if (err || header_length != SOUND_DATA_OFFSET)
		return cassette_image::error::INVALID_IMAGE; // FIXME: return the actual error here

	// Check for the presence of IDs at the usual offsets
	if (std::memcmp(&chunk_header[0], "FORM", 4) ||
		std::memcmp(&chunk_header[8], "AIFF", 4) ||
		std::memcmp(&chunk_header[COMM_OFFSET], "COMM", 4) ||
		std::memcmp(&chunk_header[SSND_OFFSET], "SSND", 4))
		return cassette_image::error::INVALID_IMAGE;

	// Verify the chunk lengths
	if (get_u32be(&chunk_header[4]) != length - 8 ||
		get_u32be(&chunk_header[COMM_OFFSET + 4]) != SSND_OFFSET - 8 - COMM_OFFSET ||
		get_u32be(&chunk_header[SSND_OFFSET + 4]) != length - 8 - SSND_OFFSET)
		return cassette_image::error::INVALID_IMAGE;

	// Prepare to convert 80-bit extended floating point the cheap and simple way
	const std::uint16_t sample_rate_exp = get_u16be(&chunk_header[COMM_OFFSET + 16]);
	if (sample_rate_exp < 0x4000 || sample_rate_exp > 0x401e)
		return cassette_image::error::UNSUPPORTED;

	opts->channels = get_u16be(&chunk_header[COMM_OFFSET + 8]);
	opts->bits_per_sample = get_u16be(&chunk_header[COMM_OFFSET + 14]);
	opts->sample_frequency = get_u32be(&chunk_header[COMM_OFFSET + 18]) >> (0x401e - sample_rate_exp);

	const std::uint32_t sample_frames = get_u32be(&chunk_header[COMM_OFFSET + 10]);
	const std::uint32_t offset = get_u32be(&chunk_header[SSND_OFFSET + 8]);
	const std::uint32_t block_size = get_u32be(&chunk_header[SSND_OFFSET + 12]);
	if (opts->channels > MAX_CHANNELS)
		return cassette_image::error::INVALID_IMAGE;
	else if (opts->bits_per_sample != 8 && opts->bits_per_sample != 16 && opts->bits_per_sample != 32)
		return cassette_image::error::UNSUPPORTED;
	else if (offset != 0 || block_size != 0)
		return cassette_image::error::UNSUPPORTED; // we don't support block-aligning here
	else if (opts->channels > 0 && opts->sample_frequency > 0 && sample_frames > 0)
		return cassette_image::error::SUCCESS;
	else
		return cassette_image::error::INVALID_IMAGE;
}


cassette_image::error aiffile_load(cassette_image *cassette)
{
	const auto [err, comm_info, info_length] = util::read_at(*cassette->get_raw_cassette_image(), COMM_OFFSET + 8, 14);
	if (err || info_length != 14)
		return cassette_image::error::INVALID_IMAGE; // FIXME: return the actual error here

	const std::uint16_t channels = get_u16be(&comm_info[0]);
	const std::uint32_t sample_frames = get_u32be(&comm_info[2]);
	const std::uint16_t bits_per_sample = get_u16be(&comm_info[6]);
	const std::uint16_t sample_rate_exp = get_u16be(&comm_info[8]);
	const std::uint32_t sample_rate = get_u32be(&comm_info[10]) >> (0x401e - sample_rate_exp);

	const cassette_image::error caserr = cassette->read_samples(
			channels,
			0.0,
			double(sample_frames) / sample_rate,
			sample_frames,
			SOUND_DATA_OFFSET,
			encode_bits_per_sample(bits_per_sample));
	if (caserr != cassette_image::error::SUCCESS)
		return caserr;

	return cassette_image::error::SUCCESS;
}


cassette_image::error aiffile_save(cassette_image *cassette, const cassette_image::Info *info)
{
	if (info->channels > MAX_CHANNELS || info->sample_frequency == 0)
		return cassette_image::error::UNSUPPORTED;
	if (info->bits_per_sample != 8 && info->bits_per_sample != 16 && info->bits_per_sample != 32)
		return cassette_image::error::UNSUPPORTED;

	uint8_t chunk_header[SOUND_DATA_OFFSET];
	std::memcpy(&chunk_header[0], "FORM", 4);
	std::memcpy(&chunk_header[8], "AIFF", 4);
	std::memcpy(&chunk_header[COMM_OFFSET], "COMM", 4);
	std::memcpy(&chunk_header[SSND_OFFSET], "SSND", 4);

	put_u16be(&chunk_header[COMM_OFFSET + 8], info->channels);
	put_u32be(&chunk_header[COMM_OFFSET + 10], info->sample_count);
	put_u16be(&chunk_header[COMM_OFFSET + 14], info->bits_per_sample);

	const unsigned bytes_per_sample = info->bits_per_sample / 8;
	const std::uint32_t container_length = SOUND_DATA_OFFSET + std::uint64_t(info->sample_count) * info->channels * bytes_per_sample;
	put_u32be(&chunk_header[4], container_length - 8);
	put_u32be(&chunk_header[COMM_OFFSET + 4], SSND_OFFSET - 8 - COMM_OFFSET);
	put_u32be(&chunk_header[SSND_OFFSET + 4], container_length - 8 - SSND_OFFSET);

	const std::uint32_t sample_rate = std::uint32_t(info->sample_frequency);
	const std::uint16_t sample_rate_exp = 0x401e - count_leading_zeros_32(sample_rate);
	put_u16be(&chunk_header[COMM_OFFSET + 16], sample_rate_exp);
	put_u32be(&chunk_header[COMM_OFFSET + 18], sample_rate << (0x401e - sample_rate_exp));
	std::memset(&chunk_header[COMM_OFFSET + 22], 0, 4);

	std::memset(&chunk_header[SSND_OFFSET + 8], 0, 8);

	auto [err, length] = util::write_at(*cassette->get_raw_cassette_image(), 0, &chunk_header[0], SOUND_DATA_OFFSET);
	if (err || length != SOUND_DATA_OFFSET)
		return cassette_image::error::INTERNAL; // FIXME: return actual error

	const cassette_image::error caserr = cassette->write_samples(
			info->channels,
			0.0,
			double(info->sample_count) / sample_rate,
			info->sample_count,
			SOUND_DATA_OFFSET,
			encode_bits_per_sample(info->bits_per_sample));
	if (caserr != cassette_image::error::SUCCESS)
		return caserr;

	return cassette_image::error::SUCCESS;
}

} // anonymous namespace


const cassette_image::Format cassette_image::aiffile_format =
{
	"aif,aiff",
	aiffile_identify,
	aiffile_load,
	aiffile_save
};
