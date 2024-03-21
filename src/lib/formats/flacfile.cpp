// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    flacfile.cpp

    Format code for flac files.

*********************************************************************/

#include "flacfile.h"

#include "flac.h"

#include <memory>
#include <new>


namespace {

constexpr int MAX_CHANNELS = 8;


// Copied from cassimg.cpp; put somewhere central?
/*********************************************************************
    helper code
*********************************************************************/
constexpr double map_double(double d, uint64_t low, uint64_t high, uint64_t value)
{
	return d * (value - low) / (high - low);
}

constexpr size_t waveform_bytes_per_sample(int waveform_flags)
{
	return size_t(1 << ((waveform_flags & 0x06) / 2));
}


cassette_image::error flacfile_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	cassette->get_raw_cassette_image()->seek(0, SEEK_SET);
	flac_decoder decoder(*cassette->get_raw_cassette_image());
	const int channels = decoder.channels();
	const int sample_rate = decoder.sample_rate();
	const int bits_per_sample = decoder.bits_per_sample();
	const int total_samples = decoder.total_samples();
	decoder.finish();

	opts->channels = channels;
	opts->sample_frequency = sample_rate;
	opts->bits_per_sample = bits_per_sample;

	if (channels > MAX_CHANNELS)
		return cassette_image::error::INVALID_IMAGE;
	else if (channels > 0 && sample_rate > 0 && total_samples > 0)
		return cassette_image::error::SUCCESS;
	else
		return cassette_image::error::INVALID_IMAGE;
}


cassette_image::error flacfile_load(cassette_image *cassette)
{
	cassette->get_raw_cassette_image()->seek(0, SEEK_SET);
	flac_decoder decoder(*cassette->get_raw_cassette_image());
	const int channels = decoder.channels();
	const int total_samples = decoder.total_samples();

	std::unique_ptr<int16_t []> samples[MAX_CHANNELS];
	int16_t *channel_samples[MAX_CHANNELS];
	for (int channel = 0; channel < channels; channel++)
	{
		samples[channel].reset(new (std::nothrow) int16_t [total_samples]);
		if (!samples[channel])
			return cassette_image::error::OUT_OF_MEMORY;
		channel_samples[channel] = samples[channel].get();
	}
	if (!decoder.decode(channel_samples, decoder.total_samples()))
		return cassette_image::error::INVALID_IMAGE;
	for (int channel = 0; channel < channels; channel++)
	{
		const cassette_image::error err = cassette->put_samples(
				channel, 0.0, (double)total_samples/decoder.sample_rate(), total_samples,
				2, channel_samples[channel], cassette_image::WAVEFORM_16BIT);
		if (err != cassette_image::error::SUCCESS)
			return err;
	}

	return cassette_image::error::SUCCESS;
}


cassette_image::error flacfile_save(cassette_image *cassette, const cassette_image::Info *info)
{
	if (info->channels > MAX_CHANNELS)
		return cassette_image::error::INVALID_IMAGE;

	cassette->get_raw_cassette_image()->seek(0, SEEK_SET);
	flac_encoder encoder;
	encoder.set_num_channels(info->channels);
	encoder.set_sample_rate(info->sample_frequency);
	if (!encoder.reset(*cassette->get_raw_cassette_image()))
		return cassette_image::error::INTERNAL;

	size_t samples_saved = 0;
	int16_t buffer[MAX_CHANNELS][4096];
	int16_t *buffer_ptr[MAX_CHANNELS];
	const size_t bytes_per_sample = waveform_bytes_per_sample(cassette_image::WAVEFORM_16BIT);
	const size_t sample_spacing = bytes_per_sample * info->channels;
	const double sample_period = info->sample_count / (double) info->sample_frequency;

	for (int channel = 0; channel < MAX_CHANNELS; channel++)
	{
		buffer_ptr[channel] = &buffer[channel][0];
	}

	while (samples_saved < info->sample_count)
	{
		size_t chunk_sample_count = std::min(sizeof(buffer) / sample_spacing, (info->sample_count - samples_saved));
		double chunk_sample_period = map_double(sample_period, 0, info->sample_count, chunk_sample_count);
		double chunk_time_index = map_double(sample_period, 0, info->sample_count, samples_saved);

		for (int channel = 0; channel < info->channels; channel++)
		{
			cassette_image::error err = cassette->get_samples(channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_spacing, &buffer[channel * bytes_per_sample], cassette_image::WAVEFORM_16BIT);

			if (err != cassette_image::error::SUCCESS)
				return err;
		}

		if (!encoder.encode(buffer_ptr, chunk_sample_count))
			return cassette_image::error::INTERNAL;

		samples_saved += chunk_sample_count;
	}

	encoder.finish();

	return cassette_image::error::SUCCESS;
}

} // anonymous namespace


const cassette_image::Format cassette_image::flacfile_format =
{
	"flac",
	flacfile_identify,
	flacfile_load,
	flacfile_save
};
