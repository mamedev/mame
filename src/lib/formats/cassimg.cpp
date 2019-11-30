// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cassimg.cpp

    Cassette tape image abstraction code

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "imageutl.h"
#include "cassimg.h"
#include <algorithm>


/* debugging parameters */
#define LOG_PUT_SAMPLES         0
#define DUMP_CASSETTES          0

#define SAMPLES_PER_BLOCK       0x40000
#define CASSETTE_FLAG_DIRTY     0x10000


CASSETTE_FORMATLIST_START(cassette_default_formats)
CASSETTE_FORMATLIST_END



/*********************************************************************
    helper code
*********************************************************************/

static double map_double(double d, uint64_t low, uint64_t high, uint64_t value)
{
	return d * (value - low) / (high - low);
}



static size_t waveform_bytes_per_sample(int waveform_flags)
{
	return (size_t) (1 << ((waveform_flags & 0x06) / 2));
}



/*********************************************************************
    extrapolation and interpolation
*********************************************************************/

static int32_t extrapolate8(int8_t value)
{
	return ((int32_t) value) << 24;
}

static int32_t extrapolate16(int16_t value)
{
	return ((int32_t) value) << 16;
}

static int8_t interpolate8(int32_t value)
{
	return (int8_t) (value >> 24);
}

static int16_t interpolate16(int32_t value)
{
	return (int16_t) (value >> 16);
}



/*********************************************************************
    initialization and termination
*********************************************************************/

static cassette_image *cassette_init(const struct CassetteFormat *format, void *file, const struct io_procs *procs, int flags)
{
	cassette_image *cassette;

	cassette = global_alloc_clear<cassette_image>();
	cassette->format = format;
	cassette->io.file = file;
	cassette->io.procs = procs;
	cassette->flags = flags;
	return cassette;
}



static void cassette_finishinit(cassette_image::error err, cassette_image *cassette, cassette_image **outcassette)
{
	if (cassette && ((err != cassette_image::error::SUCCESS) || !outcassette))
	{
		cassette_close(cassette);
		cassette = nullptr;
	}
	if (outcassette)
		*outcassette = cassette;
}



static cassette_image::error try_identify_format(const struct CassetteFormat &format, cassette_image *image, const std::string &extension, int flags, struct CassetteOptions &opts)
{
	// is this the right extension?
	if (!extension.empty() && !image_find_extension(format.extensions, extension.c_str()))
		return cassette_image::error::INVALID_IMAGE;

	// invoke format->identify
	memset(&opts, 0, sizeof(opts));
	cassette_image::error err = format.identify(image, &opts);
	if (err != cassette_image::error::SUCCESS)
		return err;

	// is this a read only format, but the cassette was not opened read only?
	if (((flags & CASSETTE_FLAG_READONLY) == 0) && (format.save == nullptr))
		return cassette_image::error::READ_WRITE_UNSUPPORTED;

	// success!
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_open_choices(void *file, const struct io_procs *procs, const std::string &extension,
	const struct CassetteFormat *const *formats, int flags, cassette_image **outcassette)
{
	cassette_image::error err;
	cassette_image *cassette;
	const struct CassetteFormat *format;
	struct CassetteOptions opts = {0, };
	int i;

	/* if not specified, use the dummy arguments */
	if (!formats)
		formats = cassette_default_formats;

	/* create the cassette object */
	cassette = cassette_init(nullptr, file, procs, flags);
	if (!cassette)
	{
		err = cassette_image::error::OUT_OF_MEMORY;
		goto done;
	}

	/* identify the image */
	format = nullptr;
	for (i = 0; !format && formats[i]; i++)
	{
		// try this format
		err = try_identify_format(*formats[i], cassette, extension, flags, opts);
		if (err != cassette_image::error::SUCCESS && err != cassette_image::error::INVALID_IMAGE)
			goto done;

		// did we succeed?
		if (err == cassette_image::error::SUCCESS)
			format = formats[i];
	}

	/* have we found a proper format */
	if (!format)
	{
		err = cassette_image::error::INVALID_IMAGE;
		goto done;
	}
	cassette->format = format;

	/* read the options */
	cassette->channels = opts.channels;
	cassette->sample_frequency = opts.sample_frequency;

	/* load the image */
	err = format->load(cassette);
	if (err != cassette_image::error::SUCCESS)
		goto done;

	/* success */
	cassette->flags &= ~CASSETTE_FLAG_DIRTY;
	err = cassette_image::error::SUCCESS;

done:
	cassette_finishinit(err, cassette, outcassette);
	return err;
}



cassette_image::error cassette_open(void *file, const struct io_procs *procs,
	const struct CassetteFormat *format, int flags, cassette_image **outcassette)
{
	const struct CassetteFormat *formats[2];
	formats[0] = format;
	formats[1] = nullptr;
	return cassette_open_choices(file, procs, nullptr, formats, flags, outcassette);
}



cassette_image::error cassette_create(void *file, const struct io_procs *procs, const struct CassetteFormat *format,
	const struct CassetteOptions *opts, int flags, cassette_image **outcassette)
{
	cassette_image::error err;
	cassette_image *cassette;
	static const struct CassetteOptions default_options = { 1, 16, 44100 };

	/* cannot create to a read only image */
	if (flags & CASSETTE_FLAG_READONLY)
		return cassette_image::error::INVALID_IMAGE;

	/* is this a good format? */
	if (format->save == nullptr)
		return cassette_image::error::INVALID_IMAGE;

	/* normalize arguments */
	if (!opts)
		opts = &default_options;

	/* create the cassette object */
	cassette = cassette_init(format, file, procs, flags);
	if (!cassette)
	{
		err = cassette_image::error::OUT_OF_MEMORY;
		goto done;
	}

	/* read the options */
	cassette->channels = opts->channels;
	cassette->sample_frequency = opts->sample_frequency;

	err = cassette_image::error::SUCCESS;

done:
	cassette_finishinit(err, cassette, outcassette);
	return err;
}



static cassette_image::error cassette_perform_save(cassette_image *cassette)
{
	struct CassetteInfo info;
	cassette_get_info(cassette, &info);
	return cassette->format->save(cassette, &info);
}



cassette_image::error cassette_save(cassette_image *cassette)
{
	cassette_image::error err;

	if (!cassette->format || !cassette->format->save)
		return cassette_image::error::UNSUPPORTED;

	err = cassette_perform_save(cassette);
	if (err != cassette_image::error::SUCCESS)
		return err;

	cassette->flags &= ~CASSETTE_FLAG_DIRTY;
	return cassette_image::error::SUCCESS;
}



void cassette_get_info(cassette_image *cassette, struct CassetteInfo *info)
{
	memset(info, 0, sizeof(*info));
	info->channels = cassette->channels;
	info->sample_count = cassette->sample_count;
	info->sample_frequency = cassette->sample_frequency;
	info->bits_per_sample = (int) waveform_bytes_per_sample(cassette->flags) * 8;
}



void cassette_close(cassette_image *cassette)
{
	if (cassette)
	{
		if ((cassette->flags & CASSETTE_FLAG_DIRTY) && (cassette->flags & CASSETTE_FLAG_SAVEONEXIT))
			cassette_save(cassette);
		for (auto & elem : cassette->blocks)
		{
			global_free(elem);
		}
		global_free(cassette);
	}
}



void cassette_change(cassette_image *cassette, void *file, const struct io_procs *procs, const struct CassetteFormat *format, int flags)
{
	if ((flags & CASSETTE_FLAG_READONLY) == 0)
		flags |= CASSETTE_FLAG_DIRTY;
	cassette->io.file = file;
	cassette->io.procs = procs;
	cassette->format = format;
	cassette->flags = flags;
}



/*********************************************************************
    calls for accessing the raw cassette image
*********************************************************************/

void cassette_image_read(cassette_image *cassette, void *buffer, uint64_t offset, size_t length)
{
	io_generic_read(&cassette->io, buffer, offset, length);
}



void cassette_image_write(cassette_image *cassette, const void *buffer, uint64_t offset, size_t length)
{
	io_generic_write(&cassette->io, buffer, offset, length);
}



uint64_t cassette_image_size(cassette_image *cassette)
{
	return io_generic_size(&cassette->io);
}



/*********************************************************************
    waveform accesses
*********************************************************************/

struct manipulation_ranges
{
	int channel_first;
	int channel_last;
	size_t sample_first;
	size_t sample_last;
};



static size_t my_round(double d)
{
	size_t result;
	d += 0.5;
	result = (size_t) d;
	return result;
}



static cassette_image::error compute_manipulation_ranges(cassette_image *cassette, int channel,
	double time_index, double sample_period, struct manipulation_ranges *ranges)
{
	if (channel < 0)
	{
		ranges->channel_first = 0;
		ranges->channel_last = cassette->channels - 1;
	}
	else
	{
		ranges->channel_first = channel;
		ranges->channel_last = channel;
	}

	ranges->sample_first = my_round(time_index * cassette->sample_frequency);
	ranges->sample_last = my_round((time_index + sample_period) * cassette->sample_frequency);

	if (ranges->sample_last > ranges->sample_first)
		ranges->sample_last--;

	return cassette_image::error::SUCCESS;
}



static cassette_image::error lookup_sample(cassette_image *cassette, int channel, size_t sample, int32_t **ptr)
{
	*ptr = nullptr;
	size_t sample_blocknum = (sample / SAMPLES_PER_BLOCK) * cassette->channels + channel;
	size_t sample_index = sample % SAMPLES_PER_BLOCK;

	/* is this block beyond the edge of our waveform? */
	if (sample_blocknum >= cassette->blocks.size()) {
		size_t osize = cassette->blocks.size();
		cassette->blocks.resize(sample_blocknum + 1);
		memset(&cassette->blocks[osize], 0, (cassette->blocks.size()-osize)*sizeof(cassette->blocks[0]));
	}

	if (cassette->blocks[sample_blocknum] == nullptr)
		cassette->blocks[sample_blocknum] = global_alloc(sample_block);

	sample_block &block = *cassette->blocks[sample_blocknum];

	/* is this sample access off the current block? */
	if (sample_index >= block.size()) {
		size_t osize = block.size();
		block.resize(SAMPLES_PER_BLOCK);
		memset(&block[osize], 0, (SAMPLES_PER_BLOCK-osize)*sizeof(block[0]));
	}

	*ptr = &block[sample_index];
	return cassette_image::error::SUCCESS;
}



/*********************************************************************
    waveform accesses
*********************************************************************/

// Note: In normal use, the sample_spacing is the same as the sample size (in bytes)
//       But it can be larger to help do an interleaved write to samples
//       (see cassette_write_samples)
cassette_image::error cassette_get_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
	void *samples, int waveform_flags)
{
	cassette_image::error err;
	struct manipulation_ranges ranges;
	size_t sample_index;
	size_t cassette_sample_index;
	uint8_t *dest_ptr;
	const int32_t *source_ptr;
	double d;
	int16_t word;
	int32_t dword;
	int64_t sum;

	assert(cassette);

	err = compute_manipulation_ranges(cassette, channel, time_index, sample_period, &ranges);
	if (err != cassette_image::error::SUCCESS)
		return err;

	for (sample_index = 0; sample_index < sample_count; sample_index++)
	{
		sum = 0;

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			d = map_double(ranges.sample_last + 1 - ranges.sample_first, 0, sample_count, sample_index) + ranges.sample_first;
			cassette_sample_index = (size_t) d;
			err = lookup_sample(cassette, channel, cassette_sample_index, (int32_t **) &source_ptr);
			if (err != cassette_image::error::SUCCESS)
				return err;

			sum += *source_ptr;
		}

		/* average out the samples */
		sum /= (ranges.channel_last + 1 - ranges.channel_first);

		/* and write out the result */
		dest_ptr = (uint8_t*)samples;
		dest_ptr += sample_index * sample_spacing;
		switch(waveform_bytes_per_sample(waveform_flags))
		{
			case 1:
				*((int8_t *) dest_ptr) = interpolate8(sum);
				break;
			case 2:
				word = interpolate16(sum);
				if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
					word = swapendian_int16(word);
				*((int16_t *) dest_ptr) = word;
				break;
			case 4:
				dword = sum;
				if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
					dword = swapendian_int32(dword);
				*((int32_t *) dest_ptr) = dword;
				break;
		}
	}
	return cassette_image::error::SUCCESS;
}


// Note: In normal use, the sample_spacing is the same as the sample size (in bytes)
//       But it can be larger to help do an interleaved read from samples
//       (see cassette_read_samples)
cassette_image::error cassette_put_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
	const void *samples, int waveform_flags)
{
	cassette_image::error err;
	struct manipulation_ranges ranges;
	size_t sample_index;
	int32_t *dest_ptr;
	int32_t dest_value;
	int16_t word;
	int32_t dword;
	const uint8_t *source_ptr;
	double d;

	if (!cassette)
		return cassette_image::error::SUCCESS;

	if (sample_period == 0)
		return cassette_image::error::SUCCESS;

	err = compute_manipulation_ranges(cassette, channel, time_index, sample_period, &ranges);
	if (err != cassette_image::error::SUCCESS)
		return err;

	if (cassette->sample_count < ranges.sample_last+1)
		cassette->sample_count = ranges.sample_last + 1;
	cassette->flags |= CASSETTE_FLAG_DIRTY;

	if (LOG_PUT_SAMPLES)
	{
		LOG_FORMATS("cassette_put_samples(): Putting samples TIME=[%2.6g..%2.6g] INDEX=[%i..%i]\n",
			time_index,             time_index + sample_period,
			(int)ranges.sample_first,   (int)ranges.sample_last);
	}

	for (sample_index = ranges.sample_first; sample_index <= ranges.sample_last; sample_index++)
	{
		/* figure out the source pointer */
		d = map_double(sample_count, ranges.sample_first, ranges.sample_last + 1, sample_index);
		source_ptr = (const uint8_t*)samples;
		source_ptr += ((size_t) d) * sample_spacing;

		/* compute the value that we are writing */
		switch(waveform_bytes_per_sample(waveform_flags)) {
		case 1:
			if (waveform_flags & CASSETTE_WAVEFORM_UNSIGNED)
				dest_value = extrapolate8((int8_t)(*source_ptr - 128));
			else
				dest_value = extrapolate8(*((int8_t *) source_ptr));
			break;
		case 2:
			word = *((int16_t *) source_ptr);
			if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
				word = swapendian_int16(word);
			dest_value = extrapolate16(word);
			break;
		case 4:
			dword = *((int32_t *) source_ptr);
			if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
				dword = swapendian_int32(dword);
			dest_value = dword;
			break;
		default:
			return cassette_image::error::INTERNAL;
		}

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			err = lookup_sample(cassette, channel, sample_index, &dest_ptr);
			if (err != cassette_image::error::SUCCESS)
				return err;
			*dest_ptr = dest_value;
		}
	}
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_get_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, int32_t *sample)
{
	return cassette_get_samples(cassette, channel, time_index,
			sample_period, 1, 0, sample, CASSETTE_WAVEFORM_32BIT);
}



cassette_image::error cassette_put_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, int32_t sample)
{
	return cassette_put_samples(cassette, channel, time_index,
			sample_period, 1, 0, &sample, CASSETTE_WAVEFORM_32BIT);
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

cassette_image::error cassette_read_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, uint64_t offset, int waveform_flags)
{
	cassette_image::error err;
	size_t chunk_sample_count;
	size_t bytes_per_sample;
	size_t sample_spacing;
	size_t samples_loaded = 0;
	double chunk_time_index;
	double chunk_sample_period;
	int channel;
	uint8_t buffer[8192];

	bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	sample_spacing = bytes_per_sample * channels;

	while(samples_loaded < sample_count)
	{
		chunk_sample_count = std::min(sizeof(buffer) / sample_spacing, (sample_count - samples_loaded));
		chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_loaded);

		cassette_image_read(cassette, buffer, offset, chunk_sample_count * sample_spacing);

		for (channel = 0; channel < channels; channel++)
		{
			err = cassette_put_samples(cassette, channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_spacing, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err != cassette_image::error::SUCCESS)
				return err;
		}

		offset += chunk_sample_count * sample_spacing;
		samples_loaded += chunk_sample_count;
	}
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_write_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, uint64_t offset, int waveform_flags)
{
	cassette_image::error err;
	size_t chunk_sample_count;
	size_t bytes_per_sample;
	size_t sample_spacing;
	size_t samples_saved = 0;
	double chunk_time_index;
	double chunk_sample_period;
	int channel;
	uint8_t buffer[8192];

	bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	sample_spacing = bytes_per_sample * channels;

	while(samples_saved < sample_count)
	{
		chunk_sample_count = std::min(sizeof(buffer) / sample_spacing, (sample_count - samples_saved));
		chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_saved);

		for (channel = 0; channel < channels; channel++)
		{
			err = cassette_get_samples(cassette, channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_spacing, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err != cassette_image::error::SUCCESS)
				return err;
		}

		cassette_image_write(cassette, buffer, offset, chunk_sample_count * sample_spacing);

		offset += chunk_sample_count * sample_spacing;
		samples_saved += chunk_sample_count;
	}
	return cassette_image::error::SUCCESS;
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

static const int8_t *choose_wave(const struct CassetteModulation *modulation, size_t *wave_bytes_length)
{
	static const int8_t square_wave[] = { -128, 127 };
	static const int8_t sine_wave[] = { 0, 48, 89, 117, 127, 117, 89, 48, 0, -48, -89, -117, -127, -117, -89, -48 };

	if (modulation->flags & CASSETTE_MODULATION_SINEWAVE)
	{
		*wave_bytes_length = ARRAY_LENGTH(sine_wave);
		return sine_wave;
	}
	else
	{
		*wave_bytes_length = ARRAY_LENGTH(square_wave);
		return square_wave;
	}
}



cassette_image::error cassette_modulation_identify(cassette_image *cassette, const struct CassetteModulation *modulation,
	struct CassetteOptions *opts)
{
	size_t wave_bytes_length;
	choose_wave(modulation, &wave_bytes_length);
	opts->bits_per_sample = 8;
	opts->channels = 1;
	opts->sample_frequency = (uint32_t) (std::max(modulation->zero_frequency_high, modulation->one_frequency_high) * wave_bytes_length * 2);
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_put_modulated_data(cassette_image *cassette, int channel, double time_index,
	const void *data, size_t data_length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	cassette_image::error err;
	const uint8_t *data_bytes = (const uint8_t *)data;
	const int8_t *wave_bytes;
	size_t wave_bytes_length;
	double total_displacement = 0.0;
	double pulse_period;
	double pulse_frequency;
	uint8_t b;
	int i;

	wave_bytes = choose_wave(modulation, &wave_bytes_length);

	while(data_length--)
	{
		b = *(data_bytes++);
		for (i = 0; i < 8; i++)
		{
			pulse_frequency = (b & (1 << i)) ? modulation->one_frequency_canonical : modulation->zero_frequency_canonical;
			pulse_period = 1 / pulse_frequency;
			err = cassette_put_samples(cassette, 0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, CASSETTE_WAVEFORM_8BIT);
			if (err != cassette_image::error::SUCCESS)
				goto done;
			time_index += pulse_period;
			total_displacement += pulse_period;
		}
	}
	err = cassette_image::error::SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



cassette_image::error cassette_put_modulated_filler(cassette_image *cassette, int channel, double time_index,
	uint8_t filler, size_t filler_length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	cassette_image::error err;
	double delta;
	double total_displacement = 0.0;

	while(filler_length--)
	{
		err = cassette_put_modulated_data(cassette, channel, time_index, &filler, 1, modulation, &delta);
		if (err != cassette_image::error::SUCCESS)
			return err;
		total_displacement += delta;
		time_index += delta;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_read_modulated_data(cassette_image *cassette, int channel, double time_index,
	uint64_t offset, uint64_t length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	cassette_image::error err;
	uint8_t buffer_stack[1024];
	uint8_t *buffer;
	uint8_t *alloc_buffer = nullptr;
	double delta;
	double total_displacement = 0.0;
	size_t this_length;
	size_t buffer_length;

	if (length <= sizeof(buffer_stack))
	{
		buffer = buffer_stack;
		buffer_length = sizeof(buffer_stack);
	}
	else
	{
		buffer_length = std::min<uint64_t>(length, 100000);
		alloc_buffer = (uint8_t*)malloc(buffer_length);
		if (!alloc_buffer)
		{
			err = cassette_image::error::OUT_OF_MEMORY;
			goto done;
		}
		buffer = alloc_buffer;
	}

	while(length > 0)
	{
		this_length = (std::min<uint64_t>)(length, buffer_length);
		cassette_image_read(cassette, buffer, offset, this_length);

		err = cassette_put_modulated_data(cassette, channel, time_index, buffer, this_length, modulation, &delta);
		if (err != cassette_image::error::SUCCESS)
			goto done;
		total_displacement += delta;
		time_index += delta;
		length -= this_length;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	err = cassette_image::error::SUCCESS;

done:
	if (alloc_buffer)
		free(alloc_buffer);
	return err;
}



cassette_image::error cassette_put_modulated_data_bit(cassette_image *cassette, int channel, double time_index,
	uint8_t data, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	cassette_image::error err;
	const int8_t *wave_bytes;
	size_t wave_bytes_length;
	double total_displacement = 0.0;
	double pulse_period;
	double pulse_frequency;

	wave_bytes = choose_wave(modulation, &wave_bytes_length);

	pulse_frequency = (data) ? modulation->one_frequency_canonical : modulation->zero_frequency_canonical;
	pulse_period = 1 / pulse_frequency;
	err = cassette_put_samples(cassette, 0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, CASSETTE_WAVEFORM_8BIT);
	if (err != cassette_image::error::SUCCESS)
		goto done;
	time_index += pulse_period;
	total_displacement += pulse_period;

	err = cassette_image::error::SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

cassette_image::error cassette_legacy_identify(cassette_image *cassette, struct CassetteOptions *opts,
	const struct CassetteLegacyWaveFiller *legacy_args)
{
	opts->channels = 1;
	opts->bits_per_sample = 16;
	opts->sample_frequency = legacy_args->sample_frequency;
	return cassette_image::error::SUCCESS;
}



cassette_image::error cassette_legacy_construct(cassette_image *cassette,
	const struct CassetteLegacyWaveFiller *legacy_args)
{
	cassette_image::error err;
	int length;
	int sample_count;
	std::vector<uint8_t> bytes;
	std::vector<uint8_t> chunk;
	std::vector<int16_t> samples;
	int pos = 0;
	uint64_t offset = 0;
	uint64_t size;
	struct CassetteLegacyWaveFiller args;

	/* sanity check the args */
	assert(legacy_args->header_samples >= -1);
	assert(legacy_args->trailer_samples >= 0);
	assert(legacy_args->fill_wave);

	size = cassette_image_size(cassette);

	/* normalize the args */
	args = *legacy_args;
	if (args.chunk_size == 0)
		args.chunk_size = 1;
	else if (args.chunk_size < 0)
		args.chunk_size = cassette_image_size(cassette);
	if (args.sample_frequency == 0)
		args.sample_frequency = 11025;

	/* allocate a buffer for the binary data */
	chunk.resize(args.chunk_size);

	/* determine number of samples */
	if (args.chunk_sample_calc != nullptr)
	{
		if (size > 0x7FFFFFFF)
		{
			err = cassette_image::error::OUT_OF_MEMORY;
			goto done;
		}

		bytes.resize(size);
		cassette_image_read(cassette, &bytes[0], 0, size);
		sample_count = args.chunk_sample_calc(&bytes[0], (int)size);

		// chunk_sample_calc functions report errors by returning negative numbers
		if (sample_count < 0)
		{
			err = cassette_image::error::INVALID_IMAGE;
			goto done;
		}

		if (args.header_samples < 0)
			args.header_samples = sample_count;
	}
	else
	{
		sample_count = ((size + args.chunk_size - 1) / args.chunk_size)
			* args.chunk_samples;
	}
	sample_count += args.header_samples + args.trailer_samples;

	/* allocate a buffer for the completed samples */
	samples.resize(sample_count);

	/* if there has to be a header */
	if (args.header_samples > 0)
	{
		length = args.fill_wave(&samples[pos], sample_count - pos, CODE_HEADER);
		if (length < 0)
		{
			err = cassette_image::error::INVALID_IMAGE;
			goto done;
		}
		pos += length;
	}

	/* convert the file data to samples */
	while((pos < sample_count) && (offset < size))
	{
		cassette_image_read(cassette, &chunk[0], offset, args.chunk_size);
		offset += args.chunk_size;

		length = args.fill_wave(&samples[pos], sample_count - pos, &chunk[0]);
		if (length < 0)
		{
			err = cassette_image::error::INVALID_IMAGE;
			goto done;
		}
		pos += length;
		if (length == 0)
			break;
	}

	/* if there has to be a trailer */
	if (args.trailer_samples > 0)
	{
		length = args.fill_wave(&samples[pos], sample_count - pos, CODE_TRAILER);
		if (length < 0)
		{
			err = cassette_image::error::INVALID_IMAGE;
			goto done;
		}
		pos += length;
	}

	/* specify the wave */
	err = cassette_put_samples(cassette, 0, 0.0, ((double) pos) / args.sample_frequency,
		pos, 2, &samples[0], CASSETTE_WAVEFORM_16BIT);
	if (err != cassette_image::error::SUCCESS)
		goto done;

	/* success! */
	err = cassette_image::error::SUCCESS;

#if DUMP_CASSETTES
	cassette_dump(cassette, "C:\\TEMP\\CASDUMP.WAV");
#endif

done:
	return err;
}



/*********************************************************************
    cassette_dump

    A debugging call to dump a cassette image to a disk based wave file
*********************************************************************/

void cassette_dump(cassette_image *image, const char *filename)
{
	FILE *f;
	struct io_generic saved_io;
	const struct CassetteFormat *saved_format;

	f = fopen(filename, "wb");
	if (!f)
		return;

	memcpy(&saved_io, &image->io, sizeof(saved_io));
	saved_format = image->format;

	image->io.file = f;
	image->io.procs = &stdio_ioprocs_noclose;
	image->format = &wavfile_format;
	cassette_perform_save(image);

	memcpy(&image->io, &saved_io, sizeof(saved_io));
	image->format = saved_format;

	fclose(f);
}
