// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cassimg.cpp

    Cassette tape image abstraction code

*********************************************************************/

#include "cassimg.h"
#include "imageutl.h"

#include "corealloc.h" // make_unique_clear

#include <algorithm>
#include <cassert>
#include <cstring>


/* debugging parameters */
#define LOG_PUT_SAMPLES         0
#define DUMP_CASSETTES          0

#define SAMPLES_PER_BLOCK       0x40000
#define CASSETTE_FLAG_DIRTY     0x10000


CASSETTE_FORMATLIST_START(cassette_default_formats)
CASSETTE_FORMATLIST_END


namespace {

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



/*********************************************************************
    extrapolation and interpolation
*********************************************************************/

constexpr int32_t extrapolate8(int8_t value)
{
	return int32_t(value) << 24;
}

constexpr int32_t extrapolate16(int16_t value)
{
	return int32_t(value) << 16;
}

constexpr int8_t interpolate8(int32_t value)
{
	return int8_t(value >> 24);
}

constexpr int16_t interpolate16(int32_t value)
{
	return int16_t(value >> 16);
}



constexpr size_t my_round(double d)
{
	return size_t(d + 0.5);
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

const int8_t *choose_wave(const cassette_image::Modulation &modulation, size_t &wave_bytes_length)
{
	static const int8_t square_wave[] = { -128, 127 };
	static const int8_t sine_wave[] = { 0, 48, 89, 117, 127, 117, 89, 48, 0, -48, -89, -117, -127, -117, -89, -48 };

	if (modulation.flags & cassette_image::MODULATION_SINEWAVE)
	{
		wave_bytes_length = ARRAY_LENGTH(sine_wave);
		return sine_wave;
	}
	else
	{
		wave_bytes_length = ARRAY_LENGTH(square_wave);
		return square_wave;
	}
}

} // anonymous namespace



/*********************************************************************
    initialization and termination
*********************************************************************/

cassette_image::error cassette_image::try_identify_format(const Format &format, const std::string &extension, int flags, Options &opts)
{
	// is this the right extension?
	if (!extension.empty() && !image_find_extension(format.extensions, extension.c_str()))
		return error::INVALID_IMAGE;

	// invoke format->identify
	opts = Options();
	error err = format.identify(this, &opts);
	if (err != error::SUCCESS)
		return err;

	// is this a read only format, but the cassette was not opened read only?
	if (((flags & FLAG_READONLY) == 0) && (format.save == nullptr))
		return error::READ_WRITE_UNSUPPORTED;

	// success!
	return error::SUCCESS;
}


cassette_image::error cassette_image::perform_save()
{
	Info info = get_info();
	return m_format->save(this, &info);
}



/*********************************************************************
    waveform accesses
*********************************************************************/

struct cassette_image::manipulation_ranges
{
	int channel_first;
	int channel_last;
	size_t sample_first;
	size_t sample_last;
};



cassette_image::error cassette_image::compute_manipulation_ranges(int channel,
	double time_index, double sample_period, manipulation_ranges &ranges) const
{
	if (channel < 0)
	{
		ranges.channel_first = 0;
		ranges.channel_last = m_channels - 1;
	}
	else
	{
		ranges.channel_first = channel;
		ranges.channel_last = channel;
	}

	ranges.sample_first = my_round(time_index * m_sample_frequency);
	ranges.sample_last = my_round((time_index + sample_period) * m_sample_frequency);

	if (ranges.sample_last > ranges.sample_first)
		ranges.sample_last--;

	return error::SUCCESS;
}



cassette_image::error cassette_image::lookup_sample(int channel, size_t sample, int32_t *&ptr)
{
	ptr = nullptr;
	size_t sample_blocknum = (sample / SAMPLES_PER_BLOCK) * m_channels + channel;
	size_t sample_index = sample % SAMPLES_PER_BLOCK;

	// is this block beyond the edge of our waveform?
	if (sample_blocknum >= m_blocks.size())
		m_blocks.resize(sample_blocknum + 1);

	if (!m_blocks[sample_blocknum])
		m_blocks[sample_blocknum] = make_unique_clear<int32_t []>(SAMPLES_PER_BLOCK);

	ptr = &m_blocks[sample_blocknum][sample_index];
	return error::SUCCESS;
}



cassette_image::cassette_image(const Format *format, void *file, const io_procs *procs, int flags)
{
	m_format = format;
	m_io.file = file;
	m_io.procs = procs;
	m_flags = flags;
}



cassette_image::~cassette_image()
{
	if ((m_flags & CASSETTE_FLAG_DIRTY) && (m_flags & FLAG_SAVEONEXIT))
		save();
}



cassette_image::error cassette_image::open(void *file, const io_procs *procs,
	const Format *format, int flags, ptr &outcassette)
{
	const Format *const formats[2] = { format, nullptr };
	return open_choices(file, procs, nullptr, formats, flags, outcassette);
}



cassette_image::error cassette_image::open_choices(void *file, const io_procs *procs, const std::string &extension,
	const Format *const *formats, int flags, ptr &outcassette)
{
	// if not specified, use the dummy arguments
	if (!formats)
		formats = cassette_default_formats;

	// create the cassette object
	ptr cassette;
	try { cassette.reset(new cassette_image(nullptr, file, procs, flags)); }
	catch (std::bad_alloc const &) { return error::OUT_OF_MEMORY; }

	// identify the image
	const Format *format = nullptr;
	Options opts;
	for (int i = 0; !format && formats[i]; i++)
	{
		// try this format
		error err = cassette->try_identify_format(*formats[i], extension, flags, opts);
		if (err != error::SUCCESS && err != error::INVALID_IMAGE)
			return err;

		// did we succeed?
		if (err == error::SUCCESS)
			format = formats[i];
	}

	// have we found a proper format
	if (!format)
		return error::INVALID_IMAGE;
	cassette->m_format = format;

	// read the options
	cassette->m_channels = opts.channels;
	cassette->m_sample_frequency = opts.sample_frequency;

	// load the image
	error err = format->load(cassette.get());
	if (err != error::SUCCESS)
		return err;

	/* success */
	cassette->m_flags &= ~CASSETTE_FLAG_DIRTY;
	outcassette = std::move(cassette);
	return error::SUCCESS;
}



cassette_image::error cassette_image::create(void *file, const io_procs *procs, const Format *format,
	const Options *opts, int flags, ptr &outcassette)
{
	static const Options default_options = { 1, 16, 44100 };

	// cannot create to a read only image
	if (flags & FLAG_READONLY)
		return error::INVALID_IMAGE;

	// is this a good format?
	if (format->save == nullptr)
		return error::INVALID_IMAGE;

	// normalize arguments
	if (!opts)
		opts = &default_options;

	// create the cassette object
	ptr cassette;
	try { cassette.reset(new cassette_image(format, file, procs, flags)); }
	catch (std::bad_alloc const &) { return error::OUT_OF_MEMORY; }

	// read the options
	cassette->m_channels = opts->channels;
	cassette->m_sample_frequency = opts->sample_frequency;

	outcassette = std::move(cassette);
	return error::SUCCESS;
}



cassette_image::error cassette_image::save()
{
	if (!m_format || !m_format->save)
		return error::UNSUPPORTED;

	error err = perform_save();
	if (err != error::SUCCESS)
		return err;

	m_flags &= ~CASSETTE_FLAG_DIRTY;
	return error::SUCCESS;
}



cassette_image::Info cassette_image::get_info() const
{
	Info result;
	result.channels = m_channels;
	result.sample_count = m_sample_count;
	result.sample_frequency = m_sample_frequency;
	result.bits_per_sample = (int)waveform_bytes_per_sample(m_flags) * 8;
	return result;
}



void cassette_image::change(void *file, const io_procs *procs, const Format *format, int flags)
{
	if ((flags & FLAG_READONLY) == 0)
		flags |= CASSETTE_FLAG_DIRTY;
	m_io.file = file;
	m_io.procs = procs;
	m_format = format;
	m_flags = flags;
}



/*********************************************************************
    calls for accessing the raw cassette image
*********************************************************************/

void cassette_image::image_read(void *buffer, uint64_t offset, size_t length)
{
	io_generic_read(&m_io, buffer, offset, length);
}



uint8_t cassette_image::image_read_byte(uint64_t offset)
{
	uint8_t data;
	io_generic_read(&m_io, &data, offset, 1);
	return data;
}



void cassette_image::image_write(const void *buffer, uint64_t offset, size_t length)
{
	io_generic_write(&m_io, buffer, offset, length);
}



uint64_t cassette_image::image_size()
{
	return io_generic_size(&m_io);
}



/*********************************************************************
    waveform accesses
*********************************************************************/

// Note: In normal use, the sample_spacing is the same as the sample size (in bytes)
//       But it can be larger to help do an interleaved write to samples
//       (see cassette_write_samples)
cassette_image::error cassette_image::get_samples(int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
	void *samples, int waveform_flags)
{
	error err;
	manipulation_ranges ranges;
	int32_t *source_ptr;

	err = compute_manipulation_ranges(channel, time_index, sample_period, ranges);
	if (err != error::SUCCESS)
		return err;

	for (size_t sample_index = 0; sample_index < sample_count; sample_index++)
	{
		int64_t sum = 0;

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			double d = map_double(ranges.sample_last + 1 - ranges.sample_first, 0, sample_count, sample_index) + ranges.sample_first;
			size_t cassette_sample_index = (size_t) d;
			err = lookup_sample(channel, cassette_sample_index, source_ptr);
			if (err != error::SUCCESS)
				return err;

			sum += *source_ptr;
		}

		/* average out the samples */
		sum /= (ranges.channel_last + 1 - ranges.channel_first);

		/* and write out the result */
		uint8_t *dest_ptr = (uint8_t*)samples;
		dest_ptr += sample_index * sample_spacing;
		int16_t word;
		int32_t dword;
		switch (waveform_bytes_per_sample(waveform_flags))
		{
		case 1:
			*((int8_t *) dest_ptr) = interpolate8(sum);
			break;
		case 2:
			word = interpolate16(sum);
			if (waveform_flags & WAVEFORM_ENDIAN_FLIP)
				word = swapendian_int16(word);
			*((int16_t *) dest_ptr) = word;
			break;
		case 4:
			dword = sum;
			if (waveform_flags & WAVEFORM_ENDIAN_FLIP)
				dword = swapendian_int32(dword);
			*((int32_t *) dest_ptr) = dword;
			break;
		}
	}
	return error::SUCCESS;
}


// Note: In normal use, the sample_spacing is the same as the sample size (in bytes)
//       But it can be larger to help do an interleaved read from samples
//       (see cassette_read_samples)
cassette_image::error cassette_image::put_samples(int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
	const void *samples, int waveform_flags)
{
	error err;
	manipulation_ranges ranges;
	int32_t *dest_ptr;

	if (sample_period == 0)
		return error::SUCCESS;

	err = compute_manipulation_ranges(channel, time_index, sample_period, ranges);
	if (err != error::SUCCESS)
		return err;

	if (m_sample_count < ranges.sample_last+1)
		m_sample_count = ranges.sample_last + 1;
	m_flags |= CASSETTE_FLAG_DIRTY;

	if (LOG_PUT_SAMPLES)
	{
		LOG_FORMATS("cassette_put_samples(): Putting samples TIME=[%2.6g..%2.6g] INDEX=[%i..%i]\n",
			time_index,             time_index + sample_period,
			(int)ranges.sample_first,   (int)ranges.sample_last);
	}

	for (size_t sample_index = ranges.sample_first; sample_index <= ranges.sample_last; sample_index++)
	{
		/* figure out the source pointer */
		double d = map_double(sample_count, ranges.sample_first, ranges.sample_last + 1, sample_index);
		const uint8_t *source_ptr = (const uint8_t*)samples;
		source_ptr += ((size_t) d) * sample_spacing;

		/* compute the value that we are writing */
		int32_t dest_value;
		int16_t word;
		int32_t dword;
		switch(waveform_bytes_per_sample(waveform_flags)) {
		case 1:
			if (waveform_flags & WAVEFORM_UNSIGNED)
				dest_value = extrapolate8((int8_t)(*source_ptr - 128));
			else
				dest_value = extrapolate8(*((int8_t *) source_ptr));
			break;
		case 2:
			word = *((int16_t *) source_ptr);
			if (waveform_flags & WAVEFORM_ENDIAN_FLIP)
				word = swapendian_int16(word);
			dest_value = extrapolate16(word);
			break;
		case 4:
			dword = *((int32_t *) source_ptr);
			if (waveform_flags & WAVEFORM_ENDIAN_FLIP)
				dword = swapendian_int32(dword);
			dest_value = dword;
			break;
		default:
			return error::INTERNAL;
		}

		for (channel = ranges.channel_first; channel <= ranges.channel_last; channel++)
		{
			/* find the sample that we are putting */
			err = lookup_sample(channel, sample_index, dest_ptr);
			if (err != error::SUCCESS)
				return err;
			*dest_ptr = dest_value;
		}
	}
	return error::SUCCESS;
}



cassette_image::error cassette_image::get_sample(int channel,
	double time_index, double sample_period, int32_t *sample)
{
	return get_samples(channel, time_index,
			sample_period, 1, 0, sample, WAVEFORM_32BIT);
}



cassette_image::error cassette_image::put_sample(int channel,
	double time_index, double sample_period, int32_t sample)
{
	return put_samples(channel, time_index,
			sample_period, 1, 0, &sample, WAVEFORM_32BIT);
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

cassette_image::error cassette_image::read_samples(int channels, double time_index,
	double sample_period, size_t sample_count, uint64_t offset, int waveform_flags)
{
	error err;
	size_t samples_loaded = 0;
	uint8_t buffer[8192];

	size_t bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	size_t sample_spacing = bytes_per_sample * channels;

	while (samples_loaded < sample_count)
	{
		size_t chunk_sample_count = std::min(sizeof(buffer) / sample_spacing, (sample_count - samples_loaded));
		double chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		double chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_loaded);

		image_read(buffer, offset, chunk_sample_count * sample_spacing);

		for (int channel = 0; channel < channels; channel++)
		{
			err = put_samples(channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_spacing, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err != error::SUCCESS)
				return err;
		}

		offset += chunk_sample_count * sample_spacing;
		samples_loaded += chunk_sample_count;
	}
	return error::SUCCESS;
}



cassette_image::error cassette_image::write_samples(int channels, double time_index,
	double sample_period, size_t sample_count, uint64_t offset, int waveform_flags)
{
	error err;
	size_t samples_saved = 0;
	uint8_t buffer[8192];

	size_t bytes_per_sample = waveform_bytes_per_sample(waveform_flags);
	size_t sample_spacing = bytes_per_sample * channels;

	while (samples_saved < sample_count)
	{
		size_t chunk_sample_count = std::min(sizeof(buffer) / sample_spacing, (sample_count - samples_saved));
		double chunk_sample_period = map_double(sample_period, 0, sample_count, chunk_sample_count);
		double chunk_time_index = time_index + map_double(sample_period, 0, sample_count, samples_saved);

		for (int channel = 0; channel < channels; channel++)
		{
			err = get_samples(channel, chunk_time_index, chunk_sample_period,
				chunk_sample_count, sample_spacing, &buffer[channel * bytes_per_sample], waveform_flags);
			if (err != error::SUCCESS)
				return err;
		}

		image_write(buffer, offset, chunk_sample_count * sample_spacing);

		offset += chunk_sample_count * sample_spacing;
		samples_saved += chunk_sample_count;
	}
	return error::SUCCESS;
}



cassette_image::error cassette_image::modulation_identify(const Modulation &modulation, Options *opts)
{
	size_t wave_bytes_length;
	choose_wave(modulation, wave_bytes_length);
	opts->bits_per_sample = 8;
	opts->channels = 1;
	opts->sample_frequency = uint32_t(std::max(modulation.zero_frequency_high, modulation.one_frequency_high) * wave_bytes_length * 2);
	return error::SUCCESS;
}



cassette_image::error cassette_image::put_modulated_data(int channel, double time_index,
	const void *data, size_t data_length, const Modulation &modulation,
	double *time_displacement)
{
	error err;
	const uint8_t *data_bytes = (const uint8_t *)data;
	size_t wave_bytes_length;
	double total_displacement = 0.0;

	const int8_t *wave_bytes = choose_wave(modulation, wave_bytes_length);

	while (data_length--)
	{
		uint8_t b = *(data_bytes++);
		for (int i = 0; i < 8; i++)
		{
			double pulse_frequency = (b & (1 << i)) ? modulation.one_frequency_canonical : modulation.zero_frequency_canonical;
			double pulse_period = 1 / pulse_frequency;
			err = put_samples(0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, WAVEFORM_8BIT);
			if (err != error::SUCCESS)
				goto done;
			time_index += pulse_period;
			total_displacement += pulse_period;
		}
	}
	err = error::SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



cassette_image::error cassette_image::put_modulated_filler(int channel, double time_index,
	uint8_t filler, size_t filler_length, const Modulation &modulation,
	double *time_displacement)
{
	error err;
	double delta;
	double total_displacement = 0.0;

	while (filler_length--)
	{
		err = put_modulated_data(channel, time_index, &filler, 1, modulation, &delta);
		if (err != error::SUCCESS)
			return err;
		total_displacement += delta;
		time_index += delta;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	return error::SUCCESS;
}



cassette_image::error cassette_image::read_modulated_data(int channel, double time_index,
	uint64_t offset, uint64_t length, const Modulation &modulation,
	double *time_displacement)
{
	uint8_t *buffer;
	uint8_t buffer_stack[1024];
	std::unique_ptr<uint8_t []> alloc_buffer;
	size_t buffer_length;
	if (length <= sizeof(buffer_stack))
	{
		buffer = buffer_stack;
		buffer_length = sizeof(buffer_stack);
	}
	else
	{
		buffer_length = std::min<uint64_t>(length, 100000);
		try { alloc_buffer = std::make_unique<uint8_t []>(buffer_length); }
		catch (std::bad_alloc const &) { return error::OUT_OF_MEMORY; }
		buffer = alloc_buffer.get();
	}

	double delta;
	double total_displacement = 0.0;
	while (length > 0)
	{
		size_t this_length = std::min<uint64_t>(length, buffer_length);
		image_read(buffer, offset, this_length);

		error err = put_modulated_data(channel, time_index, buffer, this_length, modulation, &delta);
		if (err != error::SUCCESS)
			return err;
		total_displacement += delta;
		time_index += delta;
		length -= this_length;
	}

	if (time_displacement)
		*time_displacement = total_displacement;
	return error::SUCCESS;
}



cassette_image::error cassette_image::put_modulated_data_bit(int channel, double time_index,
	uint8_t data, const Modulation &modulation,
	double *time_displacement)
{
	error err;
	size_t wave_bytes_length;
	double total_displacement = 0.0;

	const int8_t *wave_bytes = choose_wave(modulation, wave_bytes_length);

	double pulse_frequency = (data) ? modulation.one_frequency_canonical : modulation.zero_frequency_canonical;
	double pulse_period = 1 / pulse_frequency;
	err = put_samples(0, time_index, pulse_period, wave_bytes_length, 1, wave_bytes, WAVEFORM_8BIT);
	if (err != error::SUCCESS)
		goto done;
	time_index += pulse_period;
	total_displacement += pulse_period;

	err = error::SUCCESS;

done:
	if (time_displacement)
		*time_displacement = total_displacement;
	return err;
}



/*********************************************************************
    waveform accesses to/from the raw image
*********************************************************************/

cassette_image::error cassette_image::legacy_identify(Options *opts,
	const LegacyWaveFiller *legacy_args)
{
	opts->channels = 1;
	opts->bits_per_sample = 16;
	opts->sample_frequency = legacy_args->sample_frequency;
	return error::SUCCESS;
}



cassette_image::error cassette_image::legacy_construct(const LegacyWaveFiller *legacy_args)
{
	error err;
	int length;
	int sample_count;
	std::vector<uint8_t> bytes;
	std::vector<int16_t> samples;
	int pos = 0;
	uint64_t offset = 0;

	/* sanity check the args */
	assert(legacy_args->header_samples >= -1);
	assert(legacy_args->trailer_samples >= 0);
	assert(legacy_args->fill_wave);

	uint64_t size = image_size();

	/* normalize the args */
	LegacyWaveFiller args = *legacy_args;
	if (args.chunk_size == 0)
		args.chunk_size = 1;
	else if (args.chunk_size < 0)
		args.chunk_size = image_size();
	if (args.sample_frequency == 0)
		args.sample_frequency = 11025;

	/* allocate a buffer for the binary data */
	std::vector<uint8_t> chunk(args.chunk_size);

	/* determine number of samples */
	if (args.chunk_sample_calc != nullptr)
	{
		if (size > 0x7FFFFFFF)
		{
			err = error::OUT_OF_MEMORY;
			goto done;
		}

		bytes.resize(size);
		image_read(&bytes[0], 0, size);
		sample_count = args.chunk_sample_calc(&bytes[0], (int)size);

		// chunk_sample_calc functions report errors by returning negative numbers
		if (sample_count < 0)
		{
			err = error::INVALID_IMAGE;
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
			err = error::INVALID_IMAGE;
			goto done;
		}
		pos += length;
	}

	/* convert the file data to samples */
	while ((pos < sample_count) && (offset < size))
	{
		image_read(&chunk[0], offset, args.chunk_size);
		offset += args.chunk_size;

		length = args.fill_wave(&samples[pos], sample_count - pos, &chunk[0]);
		if (length < 0)
		{
			err = error::INVALID_IMAGE;
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
			err = error::INVALID_IMAGE;
			goto done;
		}
		pos += length;
	}

	/* specify the wave */
	err = put_samples(0, 0.0, ((double) pos) / args.sample_frequency,
		pos, 2, &samples[0], WAVEFORM_16BIT);
	if (err != error::SUCCESS)
		goto done;

	/* success! */
	err = error::SUCCESS;

#if DUMP_CASSETTES
	dump("C:\\TEMP\\CASDUMP.WAV");
#endif

done:
	return err;
}



/*********************************************************************
    cassette_dump

    A debugging call to dump a cassette image to a disk based wave file
*********************************************************************/

void cassette_image::dump(const char *filename)
{
	FILE *f = fopen(filename, "wb");
	if (!f)
		return;

	io_generic saved_io = m_io;
	const Format *saved_format = m_format;

	m_io.file = f;
	m_io.procs = &stdio_ioprocs_noclose;
	m_format = &wavfile_format;
	perform_save();

	m_io = saved_io;
	m_format = saved_format;

	fclose(f);
}
