// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cassimg.c

    Cassette tape image abstraction code

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "imageutl.h"
#include "cassimg.h"


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

static double map_double(double d, UINT64 low, UINT64 high, UINT64 value)
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	/* casting unsigned __int64 to double is not supported on VC6 or before */
	return d * (INT64)(value - low) / (INT64)(high - low);
#else
	return d * (value - low) / (high - low);
#endif
}



static size_t waveform_bytes_per_sample(int waveform_flags)
{
	return (size_t) (1 << ((waveform_flags & 0x06) / 2));
}



/*********************************************************************
    extrapolation and interpolation
*********************************************************************/

static INT32 extrapolate8(INT8 value)
{
	return ((INT32) value) << 24;
}

static INT32 extrapolate16(INT16 value)
{
	return ((INT32) value) << 16;
}

static INT8 interpolate8(INT32 value)
{
	return (INT8) (value >> 24);
}

static INT16 interpolate16(INT32 value)
{
	return (INT16) (value >> 16);
}



/*********************************************************************
    initialization and termination
*********************************************************************/

static cassette_image *cassette_init(const struct CassetteFormat *format, void *file, const struct io_procs *procs, int flags)
{
	cassette_image *cassette;

	cassette = global_alloc_clear(cassette_image);
	cassette->format = format;
	cassette->io.file = file;
	cassette->io.procs = procs;
	cassette->flags = flags;
	return cassette;
}



static void cassette_finishinit(casserr_t err, cassette_image *cassette, cassette_image **outcassette)
{
	if (cassette && (err || !outcassette))
	{
		cassette_close(cassette);
		cassette = nullptr;
	}
	if (outcassette)
		*outcassette = cassette;
}



static int good_format(const struct CassetteFormat *format, const char *extension, int flags)
{
	if (extension && !image_find_extension(format->extensions, extension))
		return FALSE;
	if (((flags & CASSETTE_FLAG_READONLY) == 0) && !format->save)
		return FALSE;
	return TRUE;
}



casserr_t cassette_open_choices(void *file, const struct io_procs *procs, const char *extension,
	const struct CassetteFormat *const *formats, int flags, cassette_image **outcassette)
{
	casserr_t err;
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
		err = CASSETTE_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* identify the image */
	format = nullptr;
	for (i = 0; !format && formats[i]; i++)
	{
		if (good_format(formats[i], extension, flags))
		{
			format = formats[i];
			memset(&opts, 0, sizeof(opts));
			err = format->identify(cassette, &opts);
			if (err == CASSETTE_ERROR_INVALIDIMAGE)
				format = nullptr;
			else if (err)
				goto done;
		}
	}

	/* have we found a proper format */
	if (!format)
	{
		err = CASSETTE_ERROR_INVALIDIMAGE;
		goto done;
	}
	cassette->format = format;

	/* read the options */
	cassette->channels = opts.channels;
	cassette->sample_frequency = opts.sample_frequency;

	/* load the image */
	err = format->load(cassette);
	if (err)
		goto done;

	/* success */
	cassette->flags &= ~CASSETTE_FLAG_DIRTY;
	err = CASSETTE_ERROR_SUCCESS;

done:
	cassette_finishinit(err, cassette, outcassette);
	return err;
}



casserr_t cassette_open(void *file, const struct io_procs *procs,
	const struct CassetteFormat *format, int flags, cassette_image **outcassette)
{
	const struct CassetteFormat *formats[2];
	formats[0] = format;
	formats[1] = nullptr;
	return cassette_open_choices(file, procs, nullptr, formats, flags, outcassette);
}



casserr_t cassette_create(void *file, const struct io_procs *procs, const struct CassetteFormat *format,
	const struct CassetteOptions *opts, int flags, cassette_image **outcassette)
{
	casserr_t err;
	cassette_image *cassette;
	static const struct CassetteOptions default_options = { 1, 16, 44100 };

	/* cannot create to a read only image */
	if (flags & CASSETTE_FLAG_READONLY)
		return CASSETTE_ERROR_INVALIDIMAGE;

	/* is this a good format? */
	if (!good_format(format, nullptr, flags))
		return CASSETTE_ERROR_INVALIDIMAGE;

	/* normalize arguments */
	if (!opts)
		opts = &default_options;

	/* create the cassette object */
	cassette = cassette_init(format, file, procs, flags);
	if (!cassette)
	{
		err = CASSETTE_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* read the options */
	cassette->channels = opts->channels;
	cassette->sample_frequency = opts->sample_frequency;

	err = CASSETTE_ERROR_SUCCESS;

done:
	cassette_finishinit(err, cassette, outcassette);
	return err;
}



static casserr_t cassette_perform_save(cassette_image *cassette)
{
	struct CassetteInfo info;
	cassette_get_info(cassette, &info);
	return cassette->format->save(cassette, &info);
}



casserr_t cassette_save(cassette_image *cassette)
{
	casserr_t err;

	if (!cassette->format || !cassette->format->save)
		return CASSETTE_ERROR_UNSUPPORTED;

	err = cassette_perform_save(cassette);
	if (err)
		return err;

	cassette->flags &= ~CASSETTE_FLAG_DIRTY;
	return CASSETTE_ERROR_SUCCESS;
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
			global_free(elem);
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

void cassette_image_read(cassette_image *cassette, void *buffer, UINT64 offset, size_t length)
{
	io_generic_read(&cassette->io, buffer, offset, length);
}



void cassette_image_write(cassette_image *cassette, const void *buffer, UINT64 offset, size_t length)
{
	io_generic_write(&cassette->io, buffer, offset, length);
}



UINT64 cassette_image_size(cassette_image *cassette)
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



static casserr_t compute_manipulation_ranges(cassette_image *cassette, int channel,
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

	return CASSETTE_ERROR_SUCCESS;
}



static casserr_t lookup_sample(cassette_image *cassette, int channel, size_t sample, INT32 **ptr)
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
	return CASSETTE_ERROR_SUCCESS;
}



/*********************************************************************
    waveform accesses
*********************************************************************/

casserr_t cassette_get_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_bytes,
	void *samples, int waveform_flags)
{
	casserr_t err;
	struct manipulation_ranges ranges;
	size_t sample_index;
	size_t cassette_sample_index;
	UINT8 *dest_ptr;
	const INT32 *source_ptr;
	double d;
	INT16 word;
	INT32 dword;
	INT64 sum;

	assert(cassette);

	err = compute_manipulation_ranges(cassette, channel, time_index, sample_period, &ranges);
	if (err)
		return err;

	for (sample_index = 0; sample_index < sample_count; sample_index++)
	{
		sum = 0;

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			d = map_double(ranges.sample_last + 1 - ranges.sample_first, 0, sample_count, sample_index) + ranges.sample_first;
			cassette_sample_index = (size_t) d;
			err = lookup_sample(cassette, channel, cassette_sample_index, (INT32 **) &source_ptr);
			if (err)
				return err;

			sum += *source_ptr;
		}

		/* average out the samples */
		sum /= (ranges.channel_last + 1 - ranges.channel_first);

		/* and write out the result */
		dest_ptr = (UINT8*)samples;
		dest_ptr += waveform_bytes_per_sample(waveform_flags) * sample_index * cassette->channels;
		switch(waveform_bytes_per_sample(waveform_flags))
		{
			case 1:
				*((INT8 *) dest_ptr) = interpolate8(sum);
				break;
			case 2:
				word = interpolate16(sum);
				if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
					word = FLIPENDIAN_INT16(word);
				*((INT16 *) dest_ptr) = word;
				break;
			case 4:
				dword = sum;
				if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
					dword = FLIPENDIAN_INT32(dword);
				*((INT32 *) dest_ptr) = dword;
				break;
		}
	}
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_put_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_bytes,
	const void *samples, int waveform_flags)
{
	casserr_t err;
	struct manipulation_ranges ranges;
	size_t sample_index;
	INT32 *dest_ptr;
	INT32 dest_value;
	INT16 word;
	INT32 dword;
	const UINT8 *source_ptr;
	double d;

	if (!cassette)
		return CASSETTE_ERROR_SUCCESS;

	if (sample_period == 0)
		return CASSETTE_ERROR_SUCCESS;

	err = compute_manipulation_ranges(cassette, channel, time_index, sample_period, &ranges);
	if (err)
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
		source_ptr = (const UINT8*)samples;
		source_ptr += ((size_t) d) * sample_bytes;

		/* compute the value that we are writing */
		switch(waveform_bytes_per_sample(waveform_flags)) {
		case 1:
			if (waveform_flags & CASSETTE_WAVEFORM_UNSIGNED)
				dest_value = extrapolate8((INT8)(*source_ptr - 128));
			else
				dest_value = extrapolate8(*((INT8 *) source_ptr));
			break;
		case 2:
			word = *((INT16 *) source_ptr);
			if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
				word = FLIPENDIAN_INT16(word);
			dest_value = extrapolate16(word);
			break;
		case 4:
			dword = *((INT32 *) source_ptr);
			if (waveform_flags & CASSETTE_WAVEFORM_ENDIAN_FLIP)
				dword = FLIPENDIAN_INT32(dword);
			dest_value = dword;
			break;
		default:
			return CASSETTE_ERROR_INTERNAL;
		}

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			err = lookup_sample(cassette, channel, sample_index, &dest_ptr);
			if (err)
				return err;
			*dest_ptr = dest_value;
		}
	}
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_get_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, INT32 *sample)
{
	return cassette_get_samples(cassette, channel, time_index,
			sample_period, 1, 0, sample, CASSETTE_WAVEFORM_32BIT);
}



casserr_t cassette_put_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, INT32 sample)
{
	return cassette_put_samples(cassette, channel, time_index,
			sample_period, 1, 0, &sample, CASSETTE_WAVEFORM_32BIT);
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

casserr_t cassette_read_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, UINT64 offset, int waveform_flags)
{
	casserr_t err;
	size_t chunk_sample_count;
	size_t bytes_per_sample;
	size_t sample_bytes;
	size_t samples_loaded = 0;
	double chunk_time_index;
	double chunk_sample_period;
	int channel;
	UINT8 buffer[8192];

	bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	sample_bytes = bytes_per_sample * channels;

	while(samples_loaded < sample_count)
	{
		chunk_sample_count = MIN(sizeof(buffer) / sample_bytes, (sample_count - samples_loaded));
		chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_loaded);

		cassette_image_read(cassette, buffer, offset, chunk_sample_count * sample_bytes);

		for (channel = 0; channel < channels; channel++)
		{
			err = cassette_put_samples(cassette, channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_bytes, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err)
				return err;
		}

		offset += chunk_sample_count * sample_bytes;
		samples_loaded += chunk_sample_count;
	}
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_write_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, UINT64 offset, int waveform_flags)
{
	casserr_t err;
	size_t chunk_sample_count;
	size_t bytes_per_sample;
	size_t sample_bytes;
	size_t samples_saved = 0;
	double chunk_time_index;
	double chunk_sample_period;
	int channel;
	UINT8 buffer[8192];

	bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	sample_bytes = bytes_per_sample * channels;

	while(samples_saved < sample_count)
	{
		chunk_sample_count = MIN(sizeof(buffer) / sample_bytes, (sample_count - samples_saved));
		chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_saved);

		for (channel = 0; channel < channels; channel++)
		{
			err = cassette_get_samples(cassette, channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_bytes, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err)
				return err;
		}

		cassette_image_write(cassette, buffer, offset, chunk_sample_count * sample_bytes);

		offset += chunk_sample_count * sample_bytes;
		samples_saved += chunk_sample_count;
	}
	return CASSETTE_ERROR_SUCCESS;
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

static const INT8 *choose_wave(const struct CassetteModulation *modulation, size_t *wave_bytes_length)
{
	static const INT8 square_wave[] = { -128, 127 };
	static const INT8 sine_wave[] = { 0, 48, 89, 117, 127, 117, 89, 48, 0, -48, -89, -117, -127, -117, -89, -48 };

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



casserr_t cassette_modulation_identify(cassette_image *cassette, const struct CassetteModulation *modulation,
	struct CassetteOptions *opts)
{
	size_t wave_bytes_length;
	choose_wave(modulation, &wave_bytes_length);
	opts->bits_per_sample = 8;
	opts->channels = 1;
	opts->sample_frequency = (UINT32) (MAX(modulation->zero_frequency_high, modulation->one_frequency_high) * wave_bytes_length * 2);
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_put_modulated_data(cassette_image *cassette, int channel, double time_index,
	const void *data, size_t data_length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	casserr_t err;
	const UINT8 *data_bytes = (const UINT8 *)data;
	const INT8 *wave_bytes;
	size_t wave_bytes_length;
	double total_displacement = 0.0;
	double pulse_period;
	double pulse_frequency;
	UINT8 b;
	int i;

	wave_bytes = choose_wave(modulation, &wave_bytes_length);

	while(data_length--)
	{
		b = *(data_bytes++);
		for (i = 0; i < 8; i++)
		{
			pulse_frequency = (b & (1 << i)) ? modulation->one_frequency_cannonical : modulation->zero_frequency_cannonical;
			pulse_period = 1 / pulse_frequency;
			err = cassette_put_samples(cassette, 0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, CASSETTE_WAVEFORM_8BIT);
			if (err)
				goto done;
			time_index += pulse_period;
			total_displacement += pulse_period;
		}
	}
	err = CASSETTE_ERROR_SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



casserr_t cassette_put_modulated_filler(cassette_image *cassette, int channel, double time_index,
	UINT8 filler, size_t filler_length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	casserr_t err;
	double delta;
	double total_displacement = 0.0;

	while(filler_length--)
	{
		err = cassette_put_modulated_data(cassette, channel, time_index, &filler, 1, modulation, &delta);
		if (err)
			return err;
		total_displacement += delta;
		time_index += delta;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_read_modulated_data(cassette_image *cassette, int channel, double time_index,
	UINT64 offset, UINT64 length, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	casserr_t err;
	UINT8 buffer_stack[1024];
	UINT8 *buffer;
	UINT8 *alloc_buffer = nullptr;
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
		buffer_length = MIN(length, 100000);
		alloc_buffer = (UINT8*)malloc(buffer_length);
		if (!alloc_buffer)
		{
			err = CASSETTE_ERROR_OUTOFMEMORY;
			goto done;
		}
		buffer = alloc_buffer;
	}

	while(length > 0)
	{
		this_length = (size_t) MIN(length, buffer_length);
		cassette_image_read(cassette, buffer, offset, this_length);

		err = cassette_put_modulated_data(cassette, channel, time_index, buffer, this_length, modulation, &delta);
		if (err)
			goto done;
		total_displacement += delta;
		time_index += delta;
		length -= this_length;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	err = CASSETTE_ERROR_SUCCESS;

done:
	if (alloc_buffer)
		free(alloc_buffer);
	return err;
}



casserr_t cassette_put_modulated_data_bit(cassette_image *cassette, int channel, double time_index,
	UINT8 data, const struct CassetteModulation *modulation,
	double *time_displacement)
{
	casserr_t err;
	const INT8 *wave_bytes;
	size_t wave_bytes_length;
	double total_displacement = 0.0;
	double pulse_period;
	double pulse_frequency;

	wave_bytes = choose_wave(modulation, &wave_bytes_length);

	pulse_frequency = (data) ? modulation->one_frequency_cannonical : modulation->zero_frequency_cannonical;
	pulse_period = 1 / pulse_frequency;
	err = cassette_put_samples(cassette, 0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, CASSETTE_WAVEFORM_8BIT);
	if (err)
		goto done;
	time_index += pulse_period;
	total_displacement += pulse_period;

	err = CASSETTE_ERROR_SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

casserr_t cassette_legacy_identify(cassette_image *cassette, struct CassetteOptions *opts,
	const struct CassetteLegacyWaveFiller *legacy_args)
{
	opts->channels = 1;
	opts->bits_per_sample = 16;
	opts->sample_frequency = legacy_args->sample_frequency;
	return CASSETTE_ERROR_SUCCESS;
}



casserr_t cassette_legacy_construct(cassette_image *cassette,
	const struct CassetteLegacyWaveFiller *legacy_args)
{
	casserr_t err;
	int length;
	int sample_count;
	dynamic_buffer bytes;
	dynamic_buffer chunk;
	std::vector<INT16> samples;
	int pos = 0;
	UINT64 offset = 0;
	UINT64 size;
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
	if (args.chunk_sample_calc)
	{
		if (size > 0x7FFFFFFF)
		{
			err = CASSETTE_ERROR_OUTOFMEMORY;
			goto done;
		}

		bytes.resize(size);
		cassette_image_read(cassette, &bytes[0], 0, size);
		sample_count = args.chunk_sample_calc(&bytes[0], (int)size);

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
			err = CASSETTE_ERROR_INVALIDIMAGE;
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
			err = CASSETTE_ERROR_INVALIDIMAGE;
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
			err = CASSETTE_ERROR_INVALIDIMAGE;
			goto done;
		}
		pos += length;
	}

	/* specify the wave */
	err = cassette_put_samples(cassette, 0, 0.0, ((double) pos) / args.sample_frequency,
		pos, 2, &samples[0], CASSETTE_WAVEFORM_16BIT);
	if (err)
		goto done;

	/* success! */
	err = CASSETTE_ERROR_SUCCESS;

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
