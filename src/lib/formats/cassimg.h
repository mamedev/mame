// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cassimg.h

    Cassette tape image abstraction code

*********************************************************************/
#ifndef MAME_FORMATS_CASSIMG_H
#define MAME_FORMATS_CASSIMG_H

#pragma once

#include "ioprocs.h"

#include "coretmpl.h"
#include "osdcore.h"

#include <memory>
#include <string>
#include <vector>

#ifndef LOG_FORMATS
#define LOG_FORMATS if (0) printf
#endif

// hack to get around rogues that define this
#ifdef UNSUPPORTED
#undef UNSUPPORTED
#endif

/***************************************************************************

    Constants

***************************************************************************/

enum : int
{
	CASSETTE_FLAG_READWRITE         = 0,
	CASSETTE_FLAG_READONLY          = 1,
	CASSETTE_FLAG_NOSAVEONEXIT      = 0,
	CASSETTE_FLAG_SAVEONEXIT        = 2
};

enum : int
{
	CASSETTE_WAVEFORM_8BIT          = 0,
	CASSETTE_WAVEFORM_16BIT         = 2,
	CASSETTE_WAVEFORM_16BIT_FLIP    = 3,
	CASSETTE_WAVEFORM_32BIT         = 4,
	CASSETTE_WAVEFORM_32BIT_FLIP    = 5,
	CASSETTE_WAVEFORM_ENDIAN_FLIP   = 1,
	CASSETTE_WAVEFORM_UNSIGNED      = 8
};

enum : int
{
	CASSETTE_MODULATION_SQUAREWAVE  = 0,
	CASSETTE_MODULATION_SINEWAVE    = 1
};


#ifdef LSB_FIRST
#define CASSETTE_WAVEFORM_16BITBE       CASSETTE_WAVEFORM_16BIT_FLIP
#define CASSETTE_WAVEFORM_16BITLE       CASSETTE_WAVEFORM_16BIT
#define CASSETTE_WAVEFORM_32BITBE       CASSETTE_WAVEFORM_32BIT_FLIP
#define CASSETTE_WAVEFORM_32BITLE       CASSETTE_WAVEFORM_32BIT
#else
#define CASSETTE_WAVEFORM_16BITBE       CASSETTE_WAVEFORM_16BIT
#define CASSETTE_WAVEFORM_16BITLE       CASSETTE_WAVEFORM_16BIT_FLIP
#define CASSETTE_WAVEFORM_32BITBE       CASSETTE_WAVEFORM_32BIT
#define CASSETTE_WAVEFORM_32BITLE       CASSETTE_WAVEFORM_32BIT_FLIP
#endif


/***************************************************************************

    Type definitions

***************************************************************************/

class cassette_image
{
public:
	enum class error
	{
		SUCCESS,                // no error
		INTERNAL,               // fatal internal error
		UNSUPPORTED,            // this operation is unsupported
		OUT_OF_MEMORY,          // ran out of memory
		INVALID_IMAGE,          // invalid image
		READ_WRITE_UNSUPPORTED  // read/write is not supported by this image format
	};

	using ptr = std::unique_ptr<cassette_image>;

	struct Options
	{
		int channels = 0;
		int bits_per_sample = 0;
		uint32_t sample_frequency = 0;
	};

	struct Info
	{
		int channels = 0;
		int bits_per_sample = 0;
		uint32_t sample_frequency = 0;
		size_t sample_count = 0;
	};

	struct Format
	{
		const char *extensions = nullptr;
		error (*identify)(cassette_image *cassette, Options *opts) = nullptr;
		error (*load)(cassette_image *cassette) = nullptr;
		error (*save)(cassette_image *cassette, const Info *info) = nullptr;
	};

	/* used for the core modulation code */
	struct Modulation
	{
		int flags = 0;
		double zero_frequency_low = 0;
		double zero_frequency_canonical = 0;
		double zero_frequency_high = 0;
		double one_frequency_low = 0;
		double one_frequency_canonical = 0;
		double one_frequency_high = 0;
	};

	/* code to adapt existing legacy fill_wave functions */
	struct LegacyWaveFiller
	{
		int (*fill_wave)(int16_t *, int, uint8_t *) = nullptr;
		int chunk_size = 0;
		int chunk_samples = 0;
		int (*chunk_sample_calc)(const uint8_t *bytes, int length) = nullptr;
		uint32_t sample_frequency = 0;
		int header_samples = 0;
		int trailer_samples = 0;
	};

	~cassette_image();

	// calls for accessing the raw cassette image
	void image_read(void *buffer, uint64_t offset, size_t length);
	void image_write(const void *buffer, uint64_t offset, size_t length);
	uint64_t image_size();

	// waveform accesses
	error get_samples(int channel,
		double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
		void *samples, int waveform_flags);
	error put_samples(int channel,
		double time_index, double sample_period, size_t sample_count, size_t sample_spacing,
		const void *samples, int waveform_flags);
	error get_sample(int channel, double time_index, double sample_period, int32_t *sample);
	error put_sample(int channel, double time_index, double sample_period, int32_t sample);

	// waveform accesses to/from the raw image - these are only used by lib\formats\wavfile.cpp
	error read_samples(int channels, double time_index,
		double sample_period, size_t sample_count, uint64_t offset, int waveform_flags);
	error write_samples(int channels, double time_index,
		double sample_period, size_t sample_count, uint64_t offset, int waveform_flags);

	// modulation support
	error modulation_identify(const Modulation *modulation,
		Options *opts);
	error put_modulated_data(int channel, double time_index,
		const void *data, size_t data_length, const Modulation *modulation,
		double *time_displacement);
	error put_modulated_filler(int channel, double time_index,
		uint8_t filler, size_t filler_length, const Modulation *modulation,
		double *time_displacement);
	error read_modulated_data(int channel, double time_index,
		uint64_t offset, uint64_t length, const Modulation *modulation,
		double *time_displacement);
	error put_modulated_data_bit(int channel, double time_index,
		uint8_t data, const Modulation *modulation,
		double *time_displacement);

	/* legacy code support */
#define CODE_HEADER     ((uint8_t*)-1)
#define CODE_TRAILER    ((uint8_t*)-2)
	error legacy_identify(Options *opts, const LegacyWaveFiller *legacy_args);
	error legacy_construct(const LegacyWaveFiller *legacy_args);

	/* debug calls */
	void dump(const char *filename);

	error save();
	void change(void *file, const io_procs *procs, const Format *format, int flags);
	Info get_info() const;

	static error open(void *file, const io_procs *procs,
		const Format *format, int flags, ptr &outcassette);
	static error open_choices(void *file, const io_procs *procs, const std::string &extension,
		const Format *const *formats, int flags, ptr &outcassette);
	static error create(void *file, const io_procs *procs, const cassette_image::Format *format,
		const cassette_image::Options *opts, int flags, ptr &outcassette);

	const Format *format = nullptr;
	io_generic io;

	int channels = 0;
	int flags = 0;
	uint32_t sample_frequency = 0;

	std::vector<std::unique_ptr<int32_t []> > blocks;
	size_t sample_count = 0;

	// builtin formats
	static const Format wavfile_format;

private:
	cassette_image(const Format *format, void *file, const io_procs *procs, int flags);
	cassette_image(const cassette_image &) = delete;
	cassette_image(cassette_image &&) = delete;
	cassette_image &operator=(const cassette_image &) = delete;
	cassette_image &operator=(cassette_image &&) = delete;
};

/* macros for specifying format lists */
#define CASSETTE_FORMATLIST_EXTERN(name)    \
	extern const cassette_image::Format *const name[]

#define CASSETTE_FORMATLIST_START(name)     \
	const cassette_image::Format *const name[] =    \
	{                                       \
		&cassette_image::wavfile_format,
#define CASSETTE_FORMAT(name)               \
		&(name),
#define CASSETTE_FORMATLIST_END             \
		nullptr                             \
	};

CASSETTE_FORMATLIST_EXTERN(cassette_default_formats);

#endif // MAME_FORMATS_CASSIMG_H
