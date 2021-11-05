// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    wavfile.c

    Format code for wave (*.wav) files

*********************************************************************/

#include "wavfile.h"

#include "osdcomm.h" // little_endianize_int16

#include <cassert>
#include <cstdio>
#include <cstring>

static const char magic1[4] = { 'R', 'I', 'F', 'F' };
static const char magic2[4] = { 'W', 'A', 'V', 'E' };
static const char format_tag_id[4] = { 'f', 'm', 't', ' ' };
static const char data_tag_id[4] = { 'd', 'a', 't', 'a' };

#define WAV_FORMAT_PCM      1



static uint32_t get_leuint32(const void *ptr)
{
	uint32_t value;
	memcpy(&value, ptr, sizeof(value));
	return little_endianize_int32(value);
}



static uint16_t get_leuint16(const void *ptr)
{
	uint16_t value;
	memcpy(&value, ptr, sizeof(value));
	return little_endianize_int16(value);
}



static void put_leuint32(void *ptr, uint32_t value)
{
	value = little_endianize_int32(value);
	memcpy(ptr, &value, sizeof(value));
}



static void put_leuint16(void *ptr, uint16_t value)
{
	value = little_endianize_int16(value);
	memcpy(ptr, &value, sizeof(value));
}



static cassette_image::error wavfile_process(cassette_image *cassette, cassette_image::Options *opts,
	bool read_waveform)
{
	uint8_t file_header[12];
	uint8_t tag_header[8];
	uint8_t format_tag[16];
	uint32_t stated_size;
	uint64_t file_size;
	uint32_t tag_size;
	uint32_t tag_samples;
	uint64_t offset;
	bool format_specified = false;

	uint16_t format_type = 0;
	uint32_t bytes_per_second = 0;
//  uint16_t block_align = 0;
	int waveform_flags = 0;

	/* read header */
	cassette->image_read(file_header, 0, sizeof(file_header));
	offset = sizeof(file_header);

	/* check magic numbers */
	if (memcmp(&file_header[0], magic1, 4))
		return cassette_image::error::INVALID_IMAGE;
	if (memcmp(&file_header[8], magic2, 4))
		return cassette_image::error::INVALID_IMAGE;

	/* read and sanity check size */
	stated_size = get_leuint32(&file_header[4]) + 8;
	file_size = cassette->image_size();
	if (stated_size > file_size)
		stated_size = (uint32_t) file_size;

	while(offset < stated_size)
	{
		cassette->image_read(tag_header, offset, sizeof(tag_header));
		tag_size = get_leuint32(&tag_header[4]);
		offset += sizeof(tag_header);

		if (!memcmp(tag_header, format_tag_id, 4))
		{
			/* format tag */
			if (format_specified || (tag_size < sizeof(format_tag)))
				return cassette_image::error::INVALID_IMAGE;
			format_specified = true;

			cassette->image_read(format_tag, offset, sizeof(format_tag));

			format_type             = get_leuint16(&format_tag[0]);
			opts->channels          = get_leuint16(&format_tag[2]);
			opts->sample_frequency  = get_leuint32(&format_tag[4]);
			bytes_per_second        = get_leuint32(&format_tag[8]);
//          block_align             = get_leuint16(&format_tag[12]);
			opts->bits_per_sample   = get_leuint16(&format_tag[14]);

			if (format_type != WAV_FORMAT_PCM)
				return cassette_image::error::INVALID_IMAGE;
			if (opts->sample_frequency * opts->bits_per_sample * opts->channels / 8 != bytes_per_second)
				return cassette_image::error::INVALID_IMAGE;

			switch(opts->bits_per_sample)
			{
				case 8:
					waveform_flags = cassette_image::WAVEFORM_8BIT | cassette_image::WAVEFORM_UNSIGNED;   // 8-bits wav are stored unsigned
					break;
				case 16:
					waveform_flags = cassette_image::WAVEFORM_16BITLE;
					break;
				case 32:
					waveform_flags = cassette_image::WAVEFORM_32BITLE;
					break;
				default:
					return cassette_image::error::INVALID_IMAGE;
			}
		}
		else if (!memcmp(tag_header, data_tag_id, 4))
		{
			/* data tag */
			if (!format_specified)
				return cassette_image::error::INVALID_IMAGE;

			if (read_waveform)
			{
				tag_samples = tag_size / (opts->bits_per_sample / 8) / opts->channels;
				cassette->read_samples(opts->channels, 0.0, tag_samples / ((double) opts->sample_frequency),
					tag_samples, offset, waveform_flags);
			}
		}
		else
		{
			/* ignore other tags */
		}
		offset += tag_size;
	}

	return cassette_image::error::SUCCESS;
}



static cassette_image::error wavfile_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return wavfile_process(cassette, opts, false);
}



static cassette_image::error wavfile_load(cassette_image *cassette)
{
	cassette_image::Options opts;
	memset(&opts, 0, sizeof(opts));
	return wavfile_process(cassette, &opts, true);
}



static cassette_image::error wavfile_save(cassette_image *cassette, const cassette_image::Info *info)
{
	cassette_image::error err;
	uint8_t consolidated_header[12 + 8 + 16 + 8];
	uint8_t *header               = &consolidated_header[0];
	uint8_t *format_tag_header    = &consolidated_header[12];
	uint8_t *format_tag_data      = &consolidated_header[12 + 8];
	uint8_t *data_tag_header      = &consolidated_header[12 + 8 + 16];
	uint32_t file_size;
	uint32_t bytes_per_second;
	uint16_t bits_per_sample;
	uint32_t data_size;
	size_t bytes_per_sample = 2;
	int waveform_flags = cassette_image::WAVEFORM_16BITLE;
	uint16_t block_align;

	bits_per_sample = (uint16_t) (bytes_per_sample * 8);
	bytes_per_second = info->sample_frequency * bytes_per_sample * info->channels;
	data_size = (uint32_t) (info->sample_count * bytes_per_sample * info->channels);
	file_size = data_size + sizeof(consolidated_header) - 8;
	block_align = (uint16_t) (bytes_per_sample * info->channels);

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
	cassette->image_write(consolidated_header, 0, sizeof(consolidated_header));

	/* write out the actual data */
	err = cassette->write_samples(info->channels, 0.0, info->sample_count
		/ (double) info->sample_frequency, info->sample_count, sizeof(consolidated_header),
		waveform_flags);
	if (err != cassette_image::error::SUCCESS)
		return err;

	return cassette_image::error::SUCCESS;
}



const cassette_image::Format cassette_image::wavfile_format =
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
	int32_t cassamp;
	int16_t wavsamp;

	f = fopen(fname, "rb");
	if (!f)
		return;

	if (cassette_open(f, &stdio_ioprocs, &wavfile_format, cassette_image::FLAG_READONLY, &cassette))
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
		assert(cassamp == (((uint32_t) wavsamp) << 16));
	}

	cassette_close(cassette);

	fclose(f);
}
#endif
