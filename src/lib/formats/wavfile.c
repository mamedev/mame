// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    wavfile.c

    Format code for wave (*.wav) files

*********************************************************************/

#include <stdio.h>
#include <assert.h>

#include "wavfile.h"
#include "cassimg.h"

static const char magic1[4] = { 'R', 'I', 'F', 'F' };
static const char magic2[4] = { 'W', 'A', 'V', 'E' };
static const char format_tag_id[4] = { 'f', 'm', 't', ' ' };
static const char data_tag_id[4] = { 'd', 'a', 't', 'a' };

#define WAV_FORMAT_PCM      1



static UINT32 get_leuint32(const void *ptr)
{
	UINT32 value;
	memcpy(&value, ptr, sizeof(value));
	return LITTLE_ENDIANIZE_INT32(value);
}



static UINT16 get_leuint16(const void *ptr)
{
	UINT16 value;
	memcpy(&value, ptr, sizeof(value));
	return LITTLE_ENDIANIZE_INT16(value);
}



static void put_leuint32(void *ptr, UINT32 value)
{
	value = LITTLE_ENDIANIZE_INT32(value);
	memcpy(ptr, &value, sizeof(value));
}



static void put_leuint16(void *ptr, UINT16 value)
{
	value = LITTLE_ENDIANIZE_INT16(value);
	memcpy(ptr, &value, sizeof(value));
}



static casserr_t wavfile_process(cassette_image *cassette, struct CassetteOptions *opts,
	int read_waveform)
{
	UINT8 file_header[12];
	UINT8 tag_header[8];
	UINT8 format_tag[16];
	UINT32 stated_size;
	UINT64 file_size;
	UINT32 tag_size;
	UINT32 tag_samples;
	UINT64 offset;
	int format_specified = FALSE;

	UINT16 format_type = 0;
	UINT32 bytes_per_second = 0;
//  UINT16 block_align = 0;
	int waveform_flags = 0;

	/* read header */
	cassette_image_read(cassette, file_header, 0, sizeof(file_header));
	offset = sizeof(file_header);

	/* check magic numbers */
	if (memcmp(&file_header[0], magic1, 4))
		return CASSETTE_ERROR_INVALIDIMAGE;
	if (memcmp(&file_header[8], magic2, 4))
		return CASSETTE_ERROR_INVALIDIMAGE;

	/* read and sanity check size */
	stated_size = get_leuint32(&file_header[4]) + 8;
	file_size = cassette_image_size(cassette);
	if (stated_size > file_size)
		stated_size = (UINT32) file_size;

	while(offset < stated_size)
	{
		cassette_image_read(cassette, tag_header, offset, sizeof(tag_header));
		tag_size = get_leuint32(&tag_header[4]);
		offset += sizeof(tag_header);

		if (!memcmp(tag_header, format_tag_id, 4))
		{
			/* format tag */
			if (format_specified || (tag_size < sizeof(format_tag)))
				return CASSETTE_ERROR_INVALIDIMAGE;
			format_specified = TRUE;

			cassette_image_read(cassette, format_tag, offset, sizeof(format_tag));

			format_type             = get_leuint16(&format_tag[0]);
			opts->channels          = get_leuint16(&format_tag[2]);
			opts->sample_frequency  = get_leuint32(&format_tag[4]);
			bytes_per_second        = get_leuint32(&format_tag[8]);
//          block_align             = get_leuint16(&format_tag[12]);
			opts->bits_per_sample   = get_leuint16(&format_tag[14]);

			if (format_type != WAV_FORMAT_PCM)
				return CASSETTE_ERROR_INVALIDIMAGE;
			if (opts->sample_frequency * opts->bits_per_sample * opts->channels / 8 != bytes_per_second)
				return CASSETTE_ERROR_INVALIDIMAGE;

			switch(opts->bits_per_sample)
			{
				case 8:
					waveform_flags = CASSETTE_WAVEFORM_8BIT | CASSETTE_WAVEFORM_UNSIGNED;   // 8-bits wav are stored unsigned
					break;
				case 16:
					waveform_flags = CASSETTE_WAVEFORM_16BITLE;
					break;
				case 32:
					waveform_flags = CASSETTE_WAVEFORM_32BITLE;
					break;
				default:
					return CASSETTE_ERROR_INVALIDIMAGE;
			}
		}
		else if (!memcmp(tag_header, data_tag_id, 4))
		{
			/* data tag */
			if (!format_specified)
				return CASSETTE_ERROR_INVALIDIMAGE;

			if (read_waveform)
			{
				tag_samples = tag_size / (opts->bits_per_sample / 8) / opts->channels;
				cassette_read_samples(cassette, opts->channels, 0.0, tag_samples / ((double) opts->sample_frequency),
					tag_samples, offset, waveform_flags);
			}
		}
		else
		{
			/* ignore other tags */
		}
		offset += tag_size;
	}

	return CASSETTE_ERROR_SUCCESS;
}



static casserr_t wavfile_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return wavfile_process(cassette, opts, FALSE);
}



static casserr_t wavfile_load(cassette_image *cassette)
{
	struct CassetteOptions opts;
	memset(&opts, 0, sizeof(opts));
	return wavfile_process(cassette, &opts, TRUE);
}



static casserr_t wavfile_save(cassette_image *cassette, const struct CassetteInfo *info)
{
	casserr_t err;
	UINT8 consolidated_header[12 + 8 + 16 + 8];
	UINT8 *header               = &consolidated_header[0];
	UINT8 *format_tag_header    = &consolidated_header[12];
	UINT8 *format_tag_data      = &consolidated_header[12 + 8];
	UINT8 *data_tag_header      = &consolidated_header[12 + 8 + 16];
	UINT32 file_size;
	UINT32 bytes_per_second;
	UINT16 bits_per_sample;
	UINT32 data_size;
	size_t bytes_per_sample = 2;
	int waveform_flags = CASSETTE_WAVEFORM_16BITLE;
	UINT16 block_align;

	bits_per_sample = (UINT16) (bytes_per_sample * 8);
	bytes_per_second = info->sample_frequency * bytes_per_sample * info->channels;
	data_size = (UINT32) (info->sample_count * bytes_per_sample * info->channels);
	file_size = data_size + sizeof(consolidated_header) - 8;
	block_align = (UINT16) (bytes_per_sample * info->channels);

	/* set up header */
	memcpy(&header[0],                  magic1, 4);
	memcpy(&header[8],                  magic2, 4);
	put_leuint32(&header[4],            file_size);

	/* set up format tag */
	memcpy(&format_tag_header[0],       format_tag_id, 4);
	put_leuint32(&format_tag_header[4], 16);
	put_leuint16(&format_tag_data[0],   WAV_FORMAT_PCM);
	put_leuint16(&format_tag_data[2],   info->channels);
	put_leuint32(&format_tag_data[4],   info->sample_frequency);
	put_leuint32(&format_tag_data[8],   bytes_per_second);
	put_leuint16(&format_tag_data[12],  block_align);
	put_leuint16(&format_tag_data[14],  bits_per_sample);

	/* set up data tag */
	memcpy(&data_tag_header[0],         data_tag_id, 4);
	put_leuint32(&data_tag_header[4],   data_size);

	/* write consolidated header */
	cassette_image_write(cassette, consolidated_header, 0, sizeof(consolidated_header));

	/* write out the actual data */
	err = cassette_write_samples(cassette, info->channels, 0.0, info->sample_count
		/ (double) info->sample_frequency, info->sample_count, sizeof(consolidated_header),
		waveform_flags);
	if (err)
		return err;

	return CASSETTE_ERROR_SUCCESS;
}



const struct CassetteFormat wavfile_format =
{
	"wav",
	wavfile_identify,
	wavfile_load,
	wavfile_save
};



/*********************************************************************
    wavfile_testload()

    This is a hokey function used to test the cassette wave loading
    system, specifically to test that when one loads a WAV file image
    that the resulting info queried will be the same data in the WAV.

    This code has already identified some rounding errors
*********************************************************************/

#ifdef UNUSED_FUNCTION
void wavfile_testload(const char *fname)
{
	cassette_image *cassette;
	FILE *f;
	long offset;
	int freq, samples, i;
	INT32 cassamp;
	INT16 wavsamp;

	f = fopen(fname, "rb");
	if (!f)
		return;

	if (cassette_open(f, &stdio_ioprocs, &wavfile_format, CASSETTE_FLAG_READONLY, &cassette))
	{
		fclose(f);
		return;
	}

	offset = 44;
	freq = 44100;
	samples = 5667062;

	for (i = 0; i < samples; i++)
	{
		cassette_get_sample(cassette, 0, i / (double) freq, 0.0, &cassamp);

		fseek(f, offset + i * 2, SEEK_SET);
		fread(&wavsamp, 1, 2, f);
		assert(cassamp == (((UINT32) wavsamp) << 16));
	}

	cassette_close(cassette);

	fclose(f);
}
#endif
