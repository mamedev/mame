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


static constexpr int MAX_CHANNELS = 8;


static cassette_image::error flacfile_identify(cassette_image *cassette, cassette_image::Options *opts)
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


static cassette_image::error flacfile_load(cassette_image *cassette)
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


const cassette_image::Format cassette_image::flacfile_format =
{
	"flac",
	flacfile_identify,
	flacfile_load,
	nullptr
};
