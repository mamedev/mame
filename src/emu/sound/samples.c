/* samples.c
 
 Playback of pre-recorded samples. Used for high-level simulation of discrete sound circuits
 where proper low-level simulation isn't available.  Also used for tape loops and similar.

 Current limitations
  - Only supports single channel samples!

 Considerations
  - Maybe this should be part of the presentation layer (artwork etc.) with samples specified
    in .lay files instead of in drivers?

*/


#include "emu.h"
#include "emuopts.h"
#include "samples.h"
#include "../../lib/libflac/include/flac/all.h"

typedef struct _sample_channel sample_channel;
struct _sample_channel
{
	sound_stream *stream;
	const INT16 *source;
	INT32		source_length;
	INT32		source_num;
	UINT32		pos;
	UINT32		frac;
	UINT32		step;
	UINT32		basefreq;
	UINT8		loop;
	UINT8		paused;
};


typedef struct _samples_info samples_info;
struct _samples_info
{
	device_t *device;
	int			numchannels;	/* how many channels */
	sample_channel *channel;/* array of channels */
	loaded_samples *samples;/* array of samples */
};


INLINE samples_info *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SAMPLES);
	return (samples_info *)downcast<legacy_device_base *>(device)->token();
}



#define FRAC_BITS		24
#define FRAC_ONE		(1 << FRAC_BITS)
#define FRAC_MASK		(FRAC_ONE - 1)
#define MAX_CHANNELS    100

struct flac_reader
{
	UINT8* rawdata;
	INT16* write_data;
	int position;
	int length;
	int decoded_size;
	int sample_rate;
	int channels;
	int bits_per_sample;
	int total_samples;
	int write_position;
} flacread;

static flac_reader* flacreadptr;

void my_error_callback(const FLAC__StreamDecoder * decoder, FLAC__StreamDecoderErrorStatus status, void * client_data)
{
	fatalerror("FLAC error Callback\n");
}

void my_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{

	flac_reader* flacrd =  ((flac_reader*)client_data);

	if (metadata->type==0)
	{
		const FLAC__StreamMetadata_StreamInfo *streaminfo = &(metadata->data.stream_info);

		flacrd->sample_rate = streaminfo->sample_rate;
		flacrd->channels = streaminfo->channels;
		flacrd->bits_per_sample = streaminfo->bits_per_sample;
		flacrd->total_samples = streaminfo->total_samples;
	}
}




FLAC__StreamDecoderReadStatus my_read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	flac_reader* flacrd =  ((flac_reader*)client_data);

	if(*bytes > 0)
	{
		if (*bytes <=  flacrd->length)
		{
			memcpy(buffer, flacrd->rawdata+flacrd->position, *bytes);
			flacrd->position+=*bytes;
			flacrd->length-=*bytes;
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
		else
		{
			memcpy(buffer, flacrd->rawdata+flacrd->position,  flacrd->length);
		    flacrd->position+= flacrd->length;
			flacrd->length = 0;

			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
	}
	else
	{
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}

	if ( flacrd->length==0)
	{
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}

	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;

}

FLAC__StreamDecoderWriteStatus my_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	flac_reader* flacrd =  ((flac_reader*)client_data);

	flacrd->decoded_size += frame->header.blocksize;

	for (int i=0;i<frame->header.blocksize;i++)
	{
		flacrd->write_data[i+flacrd->write_position] = buffer[0][i];
	}

	flacrd->write_position +=  frame->header.blocksize;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}






/*-------------------------------------------------
    read_wav_sample - read a WAV file as a sample
-------------------------------------------------*/

static int read_wav_sample(running_machine &machine, emu_file &file, loaded_sample *sample, char* filename)
{
	unsigned long offset = 0;
	UINT32 length, rate, filesize;
	UINT16 bits, temp16;
	char buf[32];
	UINT32 sindex;
	int type = 0;

	/* read the core header and make sure it's a WAVE file */
	offset += file.read(buf, 4);
	if (offset < 4)
	{
		mame_printf_warning("unable to read %s, 0-byte file?\n", filename);
		return 0;
	}
	if (memcmp(&buf[0], "RIFF", 4) == 0)
		type = 1;
	else if (memcmp(&buf[0], "fLaC", 4) == 0)
		type = 2;
	else
	{
		mame_printf_warning("unable to read %s, corrupt file?\n", filename);
		return 0;
	}

	if (type==1)
	{
		/* get the total size */
		offset += file.read(&filesize, 4);
		if (offset < 8)
			return 0;
		filesize = LITTLE_ENDIANIZE_INT32(filesize);

		/* read the RIFF file type and make sure it's a WAVE file */
		offset += file.read(buf, 4);
		if (offset < 12)
			return 0;
		if (memcmp(&buf[0], "WAVE", 4) != 0)
			return 0;

		/* seek until we find a format tag */
		while (1)
		{
			offset += file.read(buf, 4);
			offset += file.read(&length, 4);
			length = LITTLE_ENDIANIZE_INT32(length);
			if (memcmp(&buf[0], "fmt ", 4) == 0)
				break;

			/* seek to the next block */
			file.seek(length, SEEK_CUR);
			offset += length;
			if (offset >= filesize)
				return 0;
		}

		/* read the format -- make sure it is PCM */
		offset += file.read(&temp16, 2);
		temp16 = LITTLE_ENDIANIZE_INT16(temp16);
		if (temp16 != 1)
			return 0;

		/* number of channels -- only mono is supported */
		offset += file.read(&temp16, 2);
		temp16 = LITTLE_ENDIANIZE_INT16(temp16);
		if (temp16 != 1)
			return 0;

		/* sample rate */
		offset += file.read(&rate, 4);
		rate = LITTLE_ENDIANIZE_INT32(rate);

		/* bytes/second and block alignment are ignored */
		offset += file.read(buf, 6);

		/* bits/sample */
		offset += file.read(&bits, 2);
		bits = LITTLE_ENDIANIZE_INT16(bits);
		if (bits != 8 && bits != 16)
			return 0;

		/* seek past any extra data */
		file.seek(length - 16, SEEK_CUR);
		offset += length - 16;

		/* seek until we find a data tag */
		while (1)
		{
			offset += file.read(buf, 4);
			offset += file.read(&length, 4);
			length = LITTLE_ENDIANIZE_INT32(length);
			if (memcmp(&buf[0], "data", 4) == 0)
				break;

			/* seek to the next block */
			file.seek(length, SEEK_CUR);
			offset += length;
			if (offset >= filesize)
				return 0;
		}

		/* if there was a 0 length data block, we're done */
		if (length == 0)
			return 0;

		/* fill in the sample data */
		sample->length = length;
		sample->frequency = rate;

		/* read the data in */
		if (bits == 8)
		{
			unsigned char *tempptr;
			int sindex;

			sample->data = auto_alloc_array(machine, INT16, length);
			file.read(sample->data, length);

			/* convert 8-bit data to signed samples */
			tempptr = (unsigned char *)sample->data;
			for (sindex = length - 1; sindex >= 0; sindex--)
				sample->data[sindex] = (INT8)(tempptr[sindex] ^ 0x80) * 256;

		}
		else
		{
			/* 16-bit data is fine as-is */
			sample->data = auto_alloc_array(machine, INT16, length/2);
			file.read(sample->data, length);

				sample->length /= 2;
			if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
				for (sindex = 0; sindex < sample->length; sindex++)
					sample->data[sindex] = LITTLE_ENDIANIZE_INT16(sample->data[sindex]);
		}
	}
	else
	{
		int length;

		file.seek(0, SEEK_END);
		length = file.tell();
		file.seek(0, 0);

		flacread.rawdata = auto_alloc_array(machine, UINT8, length);
		flacread.length = length;
		flacread.position = 0;
		flacread.decoded_size = 0;

		flacreadptr = &flacread;

		file.read(flacread.rawdata, length);

		FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();

		if (!decoder)
			fatalerror("Fail FLAC__stream_decoder_new\n");

		if(FLAC__stream_decoder_init_stream(
			decoder,
			my_read_callback,
			NULL, //my_seek_callback,      // or NULL
			NULL, //my_tell_callback,      // or NULL
			NULL, //my_length_callback,    // or NULL
			NULL, //my_eof_callback,       // or NULL
			my_write_callback,
			my_metadata_callback, //my_metadata_callback,  // or NULL
			my_error_callback,
			(void*)flacreadptr /*my_client_data*/ ) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
			fatalerror("Fail FLAC__stream_decoder_init_stream\n");

		if (FLAC__stream_decoder_process_until_end_of_metadata(decoder) != FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM)
			fatalerror("Fail FLAC__stream_decoder_process_until_end_of_metadata\n");

		if (flacread.channels != 1) // only Mono supported
			fatalerror("Only MONO samples are supported\n");


		sample->data = auto_alloc_array(machine, INT16, flacread.total_samples*2);
		flacread.write_position = 0;
		flacread.write_data = sample->data;

		if (FLAC__stream_decoder_process_until_end_of_stream (decoder) != FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM)
		{
			fatalerror("Fail FLAC__stream_decoder_process_until_end_of_stream\n");
		}

		if (FLAC__stream_decoder_finish (decoder) != true)
			fatalerror("Fail FLAC__stream_decoder_finish\n");

		FLAC__stream_decoder_delete(decoder);

		/* fill in the sample data */

		sample->frequency = flacread.sample_rate;
		sample->length = flacread.total_samples * (flacread.bits_per_sample/8);

		if (flacread.bits_per_sample == 8)
		{
			for (sindex = 0; sindex <= sample->length; sindex++)
				sample->data[sindex] = ((sample->data[sindex])&0xff)*256;
		}
		else // don't need to process 16-bit samples?
		{
			sample->length = sample->length /2; //??
		}
	}

	return 1;
}


/*-------------------------------------------------
    readsamples - load all samples
-------------------------------------------------*/

loaded_samples *readsamples(running_machine &machine, const char *const *samplenames, const char *basename)
{
	loaded_samples *samples;
	int skipfirst = 0;
	int i;

	/* if the user doesn't want to use samples, bail */
	if (!machine.options().samples())
		return NULL;
	if (samplenames == 0 || samplenames[0] == 0)
		return NULL;

	/* if a name begins with '*', we will also look under that as an alternate basename */
	if (samplenames[0][0] == '*')
		skipfirst = 1;

	/* count the samples */
	for (i = 0; samplenames[i+skipfirst] != 0; i++) ;
	if (i == 0)
		return NULL;

	/* allocate the array */
	samples = (loaded_samples *)auto_alloc_array_clear(machine, UINT8, sizeof(loaded_samples) + (i-1) * sizeof(loaded_sample));
	samples->total = i;

	/* load the samples */
	for (i = 0; i < samples->total; i++)
		if (samplenames[i+skipfirst][0])
		{
			emu_file file(machine.options().sample_path(), OPEN_FLAG_READ);
			file_error filerr = FILERR_NOT_FOUND;

			char filename[512];

			if (filerr != FILERR_NONE)
			{
				// first try opening samples as .flac
				sprintf(filename, "%s.flac", samplenames[i+skipfirst]);

				filerr = file.open(basename, PATH_SEPARATOR, filename);
				// try parent sample set
				if (filerr != FILERR_NONE && skipfirst)
					filerr = file.open(samplenames[0] + 1, PATH_SEPARATOR, filename);
			}

			if (filerr != FILERR_NONE)
			{
				// .wav fallback
				sprintf(filename, "%s.wav", samplenames[i+skipfirst]);

				filerr = file.open(basename, PATH_SEPARATOR, filename);
				// try parent sample set
				if (filerr != FILERR_NONE && skipfirst)
					filerr = file.open(samplenames[0] + 1, PATH_SEPARATOR, filename);
			}


			if (filerr == FILERR_NONE)
				read_wav_sample(machine, file, &samples->sample[i], filename);

			if (filerr == FILERR_NOT_FOUND)
				mame_printf_warning("sample '%s' NOT FOUND\n", samplenames[i+skipfirst]);

		}

	return samples;
}




/* Start one of the samples loaded from disk. Note: channel must be in the range */
/* 0 .. Samplesinterface->channels-1. It is NOT the discrete channel to pass to */
/* mixer_play_sample() */
void sample_start(device_t *device,int channel,int samplenum,int loop)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;
    loaded_sample *sample;

	/* if samples are disabled, just return quietly */
	if (info->samples == NULL)
		return;

    assert( samplenum < info->samples->total );
    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();

	/* update the parameters */
	sample = &info->samples->sample[samplenum];
	chan->source = sample->data;
	chan->source_length = sample->length;
	chan->source_num = sample->data ? samplenum : -1;
	chan->pos = 0;
	chan->frac = 0;
	chan->basefreq = sample->frequency;
	chan->step = ((INT64)chan->basefreq << FRAC_BITS) / info->device->machine().sample_rate();
	chan->loop = loop;
}


void sample_start_raw(device_t *device,int channel,const INT16 *sampledata,int samples,int frequency,int loop)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();

	/* update the parameters */
	chan->source = sampledata;
	chan->source_length = samples;
	chan->source_num = -1;
	chan->pos = 0;
	chan->frac = 0;
	chan->basefreq = frequency;
	chan->step = ((INT64)chan->basefreq << FRAC_BITS) / info->device->machine().sample_rate();
	chan->loop = loop;
}


void sample_set_freq(device_t *device,int channel,int freq)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();

	chan->step = ((INT64)freq << FRAC_BITS) / info->device->machine().sample_rate();
}


void sample_set_volume(device_t *device,int channel,float volume)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	chan->stream->set_output_gain(0, volume);
}


void sample_set_pause(device_t *device,int channel,int pause)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();

	chan->paused = pause;
}


void sample_stop(device_t *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

    /* force an update before we start */
    chan->stream->update();
    chan->source = NULL;
    chan->source_num = -1;
}


int sample_get_base_freq(device_t *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();
	return chan->basefreq;
}


int sample_playing(device_t *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	chan->stream->update();
	return (chan->source != NULL);
}


static STREAM_UPDATE( sample_update_sound )
{
	sample_channel *chan = (sample_channel *)param;
	stream_sample_t *buffer = outputs[0];

	if (chan->source && !chan->paused)
	{
		/* load some info locally */
		UINT32 pos = chan->pos;
		UINT32 frac = chan->frac;
		UINT32 step = chan->step;
		const INT16 *sample = chan->source;
		UINT32 sample_length = chan->source_length;

		while (samples--)
		{
			/* do a linear interp on the sample */
			INT32 sample1 = sample[pos];
			INT32 sample2 = sample[(pos + 1) % sample_length];
			INT32 fracmult = frac >> (FRAC_BITS - 14);
			*buffer++ = ((0x4000 - fracmult) * sample1 + fracmult * sample2) >> 14;

			/* advance */
			frac += step;
			pos += frac >> FRAC_BITS;
			frac = frac & ((1 << FRAC_BITS) - 1);

			/* handle looping/ending */
			if (pos >= sample_length)
			{
				if (chan->loop)
					pos %= sample_length;
				else
				{
					chan->source = NULL;
					chan->source_num = -1;
					if (samples > 0)
						memset(buffer, 0, samples * sizeof(*buffer));
					break;
				}
			}
		}

		/* push position back out */
		chan->pos = pos;
		chan->frac = frac;
	}
	else
		memset(buffer, 0, samples * sizeof(*buffer));
}


static void samples_postload(samples_info *info)
{
	int i;

	/* loop over channels */
	for (i = 0; i < info->numchannels; i++)
	{
		sample_channel *chan = &info->channel[i];

		/* attach any samples that were loaded and playing */
		if (chan->source_num >= 0 && chan->source_num < info->samples->total)
		{
			loaded_sample *sample = &info->samples->sample[chan->source_num];
			chan->source = sample->data;
			chan->source_length = sample->length;
			if (!sample->data)
				chan->source_num = -1;
		}

		/* validate the position against the length in case the sample is smaller */
		if (chan->source && chan->pos >= chan->source_length)
		{
			if (chan->loop)
				chan->pos %= chan->source_length;
			else
			{
				chan->source = NULL;
				chan->source_num = -1;
			}
		}
	}
}


static DEVICE_START( samples )
{
	int i;
	const samples_interface *intf = (const samples_interface *)device->static_config();
	samples_info *info = get_safe_token(device);

	info->device = device;

	/* read audio samples */
	if (intf->samplenames)
		info->samples = readsamples(device->machine(), intf->samplenames,device->machine().system().name);

	/* allocate channels */
	info->numchannels = intf->channels;
	assert(info->numchannels < MAX_CHANNELS);
	info->channel = auto_alloc_array(device->machine(), sample_channel, info->numchannels);
	for (i = 0; i < info->numchannels; i++)
	{
	    info->channel[i].stream = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), &info->channel[i], sample_update_sound);

		info->channel[i].source = NULL;
		info->channel[i].source_num = -1;
		info->channel[i].step = 0;
		info->channel[i].loop = 0;
		info->channel[i].paused = 0;

		/* register with the save state system */
        device->save_item(NAME(info->channel[i].source_length), i);
        device->save_item(NAME(info->channel[i].source_num), i);
        device->save_item(NAME(info->channel[i].pos), i);
        device->save_item(NAME(info->channel[i].frac), i);
        device->save_item(NAME(info->channel[i].step), i);
        device->save_item(NAME(info->channel[i].loop), i);
        device->save_item(NAME(info->channel[i].paused), i);
	}
	device->machine().save().register_postload(save_prepost_delegate(FUNC(samples_postload), info));

	/* initialize any custom handlers */
	if (intf->start)
		(*intf->start)(device);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( samples )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(samples_info);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
        case DEVINFO_FCT_START:                         info->start = DEVICE_START_NAME( samples );		break;
        case DEVINFO_FCT_STOP:                          /* Nothing */                           		break;
        case DEVINFO_FCT_RESET:                         /* Nothing */                           		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
        case DEVINFO_STR_NAME:                          strcpy(info->s, "Samples");                 	break;
        case DEVINFO_STR_FAMILY:                   strcpy(info->s, "Big Hack");                 	break;
        case DEVINFO_STR_VERSION:                  strcpy(info->s, "1.1");                      	break;
        case DEVINFO_STR_SOURCE_FILE:                     strcpy(info->s, __FILE__);                    		break;
        case DEVINFO_STR_CREDITS:                  strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(SAMPLES, samples);
