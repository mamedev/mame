// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cassimg.h

    Cassette tape image abstraction code

*********************************************************************/

#ifndef CASSIMG_H
#define CASSIMG_H

#include "osdcore.h"
#include "coretmpl.h"
#include "ioprocs.h"

#ifndef LOG_FORMATS
#define LOG_FORMATS if (0) printf
#endif

/***************************************************************************

    Constants

***************************************************************************/

#define CASSETTE_FLAG_READWRITE         0
#define CASSETTE_FLAG_READONLY          1
#define CASSETTE_FLAG_NOSAVEONEXIT      0
#define CASSETTE_FLAG_SAVEONEXIT        2

#define CASSETTE_WAVEFORM_8BIT          0
#define CASSETTE_WAVEFORM_16BIT         2
#define CASSETTE_WAVEFORM_16BIT_FLIP    3
#define CASSETTE_WAVEFORM_32BIT         4
#define CASSETTE_WAVEFORM_32BIT_FLIP    5
#define CASSETTE_WAVEFORM_ENDIAN_FLIP   1
#define CASSETTE_WAVEFORM_UNSIGNED      8

#define CASSETTE_MODULATION_SQUAREWAVE  0
#define CASSETTE_MODULATION_SINEWAVE    1


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

typedef enum
{
	CASSETTE_ERROR_SUCCESS,         /* no error */
	CASSETTE_ERROR_INTERNAL,        /* fatal internal error */
	CASSETTE_ERROR_UNSUPPORTED,     /* this operation is unsupported */
	CASSETTE_ERROR_OUTOFMEMORY,     /* ran out of memory */
	CASSETTE_ERROR_INVALIDIMAGE     /* invalid image */
}
casserr_t;

typedef std::vector<INT32> sample_block;

struct CassetteOptions
{
	int channels;
	int bits_per_sample;
	UINT32 sample_frequency;
};

struct CassetteInfo
{
	int channels;
	int bits_per_sample;
	UINT32 sample_frequency;
	size_t sample_count;
};

struct cassette_image
{
	const struct CassetteFormat *format;
	struct io_generic io;

	int channels;
	int flags;
	UINT32 sample_frequency;

	std::vector<sample_block *> blocks;
	size_t sample_count;
};

struct CassetteFormat
{
	const char *extensions;
	casserr_t (*identify)(cassette_image *cassette, struct CassetteOptions *opts);
	casserr_t (*load)(cassette_image *cassette);
	casserr_t (*save)(cassette_image *cassette, const struct CassetteInfo *info);
};

/* used for the core modulation code */
struct CassetteModulation
{
	int flags;
	double zero_frequency_low;
	double zero_frequency_cannonical;
	double zero_frequency_high;
	double one_frequency_low;
	double one_frequency_cannonical;
	double one_frequency_high;
};

/* code to adapt existing legacy fill_wave functions */
struct CassetteLegacyWaveFiller
{
	int (*fill_wave)(INT16 *, int, UINT8 *);
	int chunk_size;
	int chunk_samples;
	int (*chunk_sample_calc)(const UINT8 *bytes, int length);
	UINT32 sample_frequency;
	int header_samples;
	int trailer_samples;
};

/* builtin formats */
extern const struct CassetteFormat wavfile_format;

/* macros for specifying format lists */
#define CASSETTE_FORMATLIST_EXTERN(name)    \
	extern const struct CassetteFormat *const name[]

#define CASSETTE_FORMATLIST_START(name)     \
	const struct CassetteFormat *const name[] = \
	{                                       \
		&wavfile_format,
#define CASSETTE_FORMAT(name)               \
		&(name),
#define CASSETTE_FORMATLIST_END             \
		nullptr                                \
	};

CASSETTE_FORMATLIST_EXTERN(cassette_default_formats);

/***************************************************************************

    Prototypes

***************************************************************************/

casserr_t cassette_open(void *file, const struct io_procs *procs,
	const struct CassetteFormat *format, int flags, cassette_image **outcassette);
casserr_t cassette_open_choices(void *file, const struct io_procs *procs, const char *extension,
	const struct CassetteFormat *const *formats, int flags, cassette_image **outcassette);
casserr_t cassette_create(void *file, const struct io_procs *procs, const struct CassetteFormat *format,
	const struct CassetteOptions *opts, int flags, cassette_image **outcassette);
casserr_t cassette_save(cassette_image *cassette);
void cassette_close(cassette_image *cassette);
void cassette_change(cassette_image *cassette, void *file, const struct io_procs *procs, const struct CassetteFormat *format, int flags);
void cassette_get_info(cassette_image *cassette, struct CassetteInfo *info);

/* calls for accessing the raw cassette image */
void cassette_image_read(cassette_image *cassette, void *buffer, UINT64 offset, size_t length);
void cassette_image_write(cassette_image *cassette, const void *buffer, UINT64 offset, size_t length);
UINT64 cassette_image_size(cassette_image *cassette);

/* waveform accesses */
casserr_t cassette_get_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_bytes,
	void *samples, int waveform_flags);
casserr_t cassette_put_samples(cassette_image *cassette, int channel,
	double time_index, double sample_period, size_t sample_count, size_t sample_bytes,
	const void *samples, int waveform_flags);
casserr_t cassette_get_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, INT32 *sample);
casserr_t cassette_put_sample(cassette_image *cassette, int channel,
	double time_index, double sample_period, INT32 sample);

/* waveform accesses to/from the raw image */
casserr_t cassette_read_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, UINT64 offset, int waveform_flags);
casserr_t cassette_write_samples(cassette_image *cassette, int channels, double time_index,
	double sample_period, size_t sample_count, UINT64 offset, int waveform_flags);

/* modulation support */
casserr_t cassette_modulation_identify(cassette_image *cassette, const struct CassetteModulation *modulation,
	struct CassetteOptions *opts);
casserr_t cassette_put_modulated_data(cassette_image *cassette, int channel, double time_index,
	const void *data, size_t data_length, const struct CassetteModulation *modulation,
	double *time_displacement);
casserr_t cassette_put_modulated_filler(cassette_image *cassette, int channel, double time_index,
	UINT8 filler, size_t filler_length, const struct CassetteModulation *modulation,
	double *time_displacement);
casserr_t cassette_read_modulated_data(cassette_image *cassette, int channel, double time_index,
	UINT64 offset, UINT64 length, const struct CassetteModulation *modulation,
	double *time_displacement);
casserr_t cassette_put_modulated_data_bit(cassette_image *cassette, int channel, double time_index,
	UINT8 data, const struct CassetteModulation *modulation,
	double *time_displacement);

/* debug calls */
void cassette_dump(cassette_image *image, const char *filename);

/* legacy code support */
#define CODE_HEADER     ((UINT8*)-1)
#define CODE_TRAILER    ((UINT8*)-2)
casserr_t cassette_legacy_identify(cassette_image *cassette, struct CassetteOptions *opts,
	const struct CassetteLegacyWaveFiller *legacy_args);
casserr_t cassette_legacy_construct(cassette_image *cassette,
	const struct CassetteLegacyWaveFiller *legacy_args);

#endif /* CASSIMG_H */
