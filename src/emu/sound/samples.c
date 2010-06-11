#include "emu.h"
#include "emuopts.h"
#include "streams.h"
#include "samples.h"


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
	running_device *device;
	int			numchannels;	/* how many channels */
	sample_channel *channel;/* array of channels */
	loaded_samples *samples;/* array of samples */
};


INLINE samples_info *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_SAMPLES);
	return (samples_info *)downcast<legacy_device_base *>(device)->token();
}



#define FRAC_BITS		24
#define FRAC_ONE		(1 << FRAC_BITS)
#define FRAC_MASK		(FRAC_ONE - 1)
#define MAX_CHANNELS    100


/*-------------------------------------------------
    read_wav_sample - read a WAV file as a sample
-------------------------------------------------*/

static int read_wav_sample(running_machine *machine, mame_file *f, loaded_sample *sample)
{
	unsigned long offset = 0;
	UINT32 length, rate, filesize;
	UINT16 bits, temp16;
	char buf[32];
	UINT32 sindex;

	/* read the core header and make sure it's a WAVE file */
	offset += mame_fread(f, buf, 4);
	if (offset < 4)
		return 0;
	if (memcmp(&buf[0], "RIFF", 4) != 0)
		return 0;

	/* get the total size */
	offset += mame_fread(f, &filesize, 4);
	if (offset < 8)
		return 0;
	filesize = LITTLE_ENDIANIZE_INT32(filesize);

	/* read the RIFF file type and make sure it's a WAVE file */
	offset += mame_fread(f, buf, 4);
	if (offset < 12)
		return 0;
	if (memcmp(&buf[0], "WAVE", 4) != 0)
		return 0;

	/* seek until we find a format tag */
	while (1)
	{
		offset += mame_fread(f, buf, 4);
		offset += mame_fread(f, &length, 4);
		length = LITTLE_ENDIANIZE_INT32(length);
		if (memcmp(&buf[0], "fmt ", 4) == 0)
			break;

		/* seek to the next block */
		mame_fseek(f, length, SEEK_CUR);
		offset += length;
		if (offset >= filesize)
			return 0;
	}

	/* read the format -- make sure it is PCM */
	offset += mame_fread(f, &temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
		return 0;

	/* number of channels -- only mono is supported */
	offset += mame_fread(f, &temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
		return 0;

	/* sample rate */
	offset += mame_fread(f, &rate, 4);
	rate = LITTLE_ENDIANIZE_INT32(rate);

	/* bytes/second and block alignment are ignored */
	offset += mame_fread(f, buf, 6);

	/* bits/sample */
	offset += mame_fread(f, &bits, 2);
	bits = LITTLE_ENDIANIZE_INT16(bits);
	if (bits != 8 && bits != 16)
		return 0;

	/* seek past any extra data */
	mame_fseek(f, length - 16, SEEK_CUR);
	offset += length - 16;

	/* seek until we find a data tag */
	while (1)
	{
		offset += mame_fread(f, buf, 4);
		offset += mame_fread(f, &length, 4);
		length = LITTLE_ENDIANIZE_INT32(length);
		if (memcmp(&buf[0], "data", 4) == 0)
			break;

		/* seek to the next block */
		mame_fseek(f, length, SEEK_CUR);
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
		mame_fread(f, sample->data, length);

		/* convert 8-bit data to signed samples */
		tempptr = (unsigned char *)sample->data;
		for (sindex = length - 1; sindex >= 0; sindex--)
			sample->data[sindex] = (INT8)(tempptr[sindex] ^ 0x80) * 256;
	}
	else
	{
		/* 16-bit data is fine as-is */
		sample->data = auto_alloc_array(machine, INT16, length/2);
		mame_fread(f, sample->data, length);
		sample->length /= 2;
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			for (sindex = 0; sindex < sample->length; sindex++)
				sample->data[sindex] = LITTLE_ENDIANIZE_INT16(sample->data[sindex]);
	}
	return 1;
}


/*-------------------------------------------------
    readsamples - load all samples
-------------------------------------------------*/

loaded_samples *readsamples(running_machine *machine, const char *const *samplenames, const char *basename)
{
	loaded_samples *samples;
	int skipfirst = 0;
	int i;

	/* if the user doesn't want to use samples, bail */
	if (!options_get_bool(mame_options(), OPTION_SAMPLES))
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
			file_error filerr;
			mame_file *f;

			astring fname(basename, PATH_SEPARATOR, samplenames[i+skipfirst]);
			filerr = mame_fopen(SEARCHPATH_SAMPLE, fname, OPEN_FLAG_READ, &f);

			if (filerr != FILERR_NONE && skipfirst)
			{
				astring fname(samplenames[0] + 1, PATH_SEPARATOR, samplenames[i+skipfirst]);
				filerr = mame_fopen(SEARCHPATH_SAMPLE, fname, OPEN_FLAG_READ, &f);
			}
			if (filerr == FILERR_NONE)
			{
				read_wav_sample(machine, f, &samples->sample[i]);
				mame_fclose(f);
			}
		}

	return samples;
}




/* Start one of the samples loaded from disk. Note: channel must be in the range */
/* 0 .. Samplesinterface->channels-1. It is NOT the discrete channel to pass to */
/* mixer_play_sample() */
void sample_start(running_device *device,int channel,int samplenum,int loop)
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
	stream_update(chan->stream);

	/* update the parameters */
	sample = &info->samples->sample[samplenum];
	chan->source = sample->data;
	chan->source_length = sample->length;
	chan->source_num = sample->data ? samplenum : -1;
	chan->pos = 0;
	chan->frac = 0;
	chan->basefreq = sample->frequency;
	chan->step = ((INT64)chan->basefreq << FRAC_BITS) / info->device->machine->sample_rate;
	chan->loop = loop;
}


void sample_start_raw(running_device *device,int channel,const INT16 *sampledata,int samples,int frequency,int loop)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	stream_update(chan->stream);

	/* update the parameters */
	chan->source = sampledata;
	chan->source_length = samples;
	chan->source_num = -1;
	chan->pos = 0;
	chan->frac = 0;
	chan->basefreq = frequency;
	chan->step = ((INT64)chan->basefreq << FRAC_BITS) / info->device->machine->sample_rate;
	chan->loop = loop;
}


void sample_set_freq(running_device *device,int channel,int freq)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	stream_update(chan->stream);

	chan->step = ((INT64)freq << FRAC_BITS) / info->device->machine->sample_rate;
}


void sample_set_volume(running_device *device,int channel,float volume)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	stream_set_output_gain(chan->stream, 0, volume);
}


void sample_set_pause(running_device *device,int channel,int pause)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	stream_update(chan->stream);

	chan->paused = pause;
}


void sample_stop(running_device *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

    /* force an update before we start */
    stream_update(chan->stream);
    chan->source = NULL;
    chan->source_num = -1;
}


int sample_get_base_freq(running_device *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	stream_update(chan->stream);
	return chan->basefreq;
}


int sample_playing(running_device *device,int channel)
{
    samples_info *info = get_safe_token(device);
    sample_channel *chan;

    assert( channel < info->numchannels );

    chan = &info->channel[channel];

	/* force an update before we start */
	stream_update(chan->stream);
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


static STATE_POSTLOAD( samples_postload )
{
	samples_info *info = (samples_info *)param;
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
	const samples_interface *intf = (const samples_interface *)device->baseconfig().static_config();
	samples_info *info = get_safe_token(device);

	info->device = device;

	/* read audio samples */
	if (intf->samplenames)
		info->samples = readsamples(device->machine, intf->samplenames,device->machine->gamedrv->name);

	/* allocate channels */
	info->numchannels = intf->channels;
	assert(info->numchannels < MAX_CHANNELS);
	info->channel = auto_alloc_array(device->machine, sample_channel, info->numchannels);
	for (i = 0; i < info->numchannels; i++)
	{
	    info->channel[i].stream = stream_create(device, 0, 1, device->machine->sample_rate, &info->channel[i], sample_update_sound);

		info->channel[i].source = NULL;
		info->channel[i].source_num = -1;
		info->channel[i].step = 0;
		info->channel[i].loop = 0;
		info->channel[i].paused = 0;

		/* register with the save state system */
        state_save_register_device_item(device, i, info->channel[i].source_length);
        state_save_register_device_item(device, i, info->channel[i].source_num);
        state_save_register_device_item(device, i, info->channel[i].pos);
        state_save_register_device_item(device, i, info->channel[i].frac);
        state_save_register_device_item(device, i, info->channel[i].step);
        state_save_register_device_item(device, i, info->channel[i].loop);
        state_save_register_device_item(device, i, info->channel[i].paused);
	}
	state_save_register_postload(device->machine, samples_postload, info);

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
