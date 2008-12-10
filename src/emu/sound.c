/***************************************************************************

    sound.c

    Core sound functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "osdepend.h"
#include "streams.h"
#include "config.h"
#include "profiler.h"
#include "sound/wavwrite.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE			(0)

#define VPRINTF(x)	do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_MIXER_CHANNELS		100
#define SOUND_UPDATE_FREQUENCY	ATTOTIME_IN_HZ(50)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sound_output sound_output;
struct _sound_output
{
	sound_stream *	stream;					/* associated stream */
	int				output;					/* output number */
};


typedef struct _sound_info sound_info;
struct _sound_info
{
	const sound_config *sound;				/* pointer to the sound info */
	int				outputs;				/* number of outputs from this instance */
	sound_output *	output;					/* array of output information */
};


typedef struct _speaker_input speaker_input;
struct _speaker_input
{
	float			gain;					/* current gain */
	float			default_gain;			/* default gain */
	char *			name;					/* name of this input */
};


typedef struct _speaker_info speaker_info;
struct _speaker_info
{
	const speaker_config *speaker;			/* pointer to the speaker info */
	const char *	tag;					/* speaker tag */
	sound_stream *	mixer_stream;			/* mixing stream */
	int				inputs;					/* number of input streams */
	speaker_input *	input;					/* array of input information */
#ifdef MAME_DEBUG
	INT32			max_sample;				/* largest sample value we've seen */
	INT32			clipped_samples;		/* total number of clipped samples */
	INT32			total_samples;			/* total number of samples */
#endif
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static emu_timer *sound_update_timer;

static int totalsnd;
static sound_info sound[MAX_SOUND];

static INT16 *finalmix;
static UINT32 finalmix_leftover;
static INT32 *leftmix, *rightmix;

static int sound_muted;
static int sound_attenuation;
static int global_sound_enabled;
static int nosound_mode;

static wav_file *wavfile;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void sound_reset(running_machine *machine);
static void sound_exit(running_machine *machine);
static void sound_pause(running_machine *machine, int pause);
static void sound_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void sound_save(running_machine *machine, int config_type, xml_data_node *parentnode);
static TIMER_CALLBACK( sound_update );
static void start_sound_chips(running_machine *machine);
static void route_sound(running_machine *machine);
static void mixer_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length);
static STATE_POSTLOAD( mixer_postload );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a timer
-------------------------------------------------*/

INLINE speaker_info *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SPEAKER_OUTPUT);

	return (speaker_info *)device->token;
}


/*-------------------------------------------------
    find_speaker_by_tag - find a tagged speaker
-------------------------------------------------*/

INLINE speaker_info *find_speaker_by_tag(running_machine *machine, const char *tag)
{
	const device_config *speaker = device_list_find_by_tag(machine->config->devicelist, SPEAKER_OUTPUT, tag);
	return (speaker == NULL) ? NULL : speaker->token;
}


/*-------------------------------------------------
    find_sound_by_tag - find a tagged sound chip
-------------------------------------------------*/

INLINE sound_info *find_sound_by_tag(const char *tag)
{
	int sndnum;

	/* attempt to find the speaker in our list */
	for (sndnum = 0; sndnum < totalsnd; sndnum++)
		if (sound[sndnum].sound->tag && !strcmp(sound[sndnum].sound->tag, tag))
			return &sound[sndnum];
	return NULL;
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    sound_init - start up the sound system
-------------------------------------------------*/

void sound_init(running_machine *machine)
{
	attotime update_frequency = SOUND_UPDATE_FREQUENCY;
	const char *filename;

	/* handle -nosound */
	nosound_mode = !options_get_bool(mame_options(), OPTION_SOUND);
	if (nosound_mode)
		machine->sample_rate = 11025;

	/* count the speakers */
	VPRINTF(("total speakers = %d\n", speaker_output_count(machine->config)));

	/* allocate memory for mix buffers */
	leftmix = auto_malloc(machine->sample_rate * sizeof(*leftmix));
	rightmix = auto_malloc(machine->sample_rate * sizeof(*rightmix));
	finalmix = auto_malloc(machine->sample_rate * sizeof(*finalmix));

	/* allocate a global timer for sound timing */
	sound_update_timer = timer_alloc(machine, sound_update, NULL);
	timer_adjust_periodic(sound_update_timer, update_frequency, 0, update_frequency);

	/* initialize the streams engine */
	VPRINTF(("streams_init\n"));
	streams_init(machine, update_frequency.attoseconds);

	/* now start up the sound chips and tag their streams */
	VPRINTF(("start_sound_chips\n"));
	start_sound_chips(machine);

	/* finally, do all the routing */
	VPRINTF(("route_sound\n"));
	route_sound(machine);

	/* open the output WAV file if specified */
	filename = options_get_string(mame_options(), OPTION_WAVWRITE);
	if (filename[0] != 0)
		wavfile = wav_open(filename, machine->sample_rate, 2);

	/* enable sound by default */
	global_sound_enabled = TRUE;
	sound_muted = FALSE;
	sound_set_attenuation(options_get_int(mame_options(), OPTION_VOLUME));

	/* register callbacks */
	config_register(machine, "mixer", sound_load, sound_save);
	add_pause_callback(machine, sound_pause);
	add_reset_callback(machine, sound_reset);
	add_exit_callback(machine, sound_exit);
}


/*-------------------------------------------------
    sound_exit - clean up after ourselves
-------------------------------------------------*/

static void sound_exit(running_machine *machine)
{
	int sndnum;

	/* close any open WAV file */
	if (wavfile != NULL)
		wav_close(wavfile);

	/* stop all the sound chips */
	for (sndnum = 0; sndnum < MAX_SOUND; sndnum++)
		if (machine->config->sound[sndnum].type != SOUND_DUMMY)
			sndintrf_exit_sound(sndnum);

	/* reset variables */
	totalsnd = 0;
	memset(sound, 0, sizeof(sound));
}



/***************************************************************************
    INITIALIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    start_sound_chips - loop over all sound chips
    and initialize them
-------------------------------------------------*/

static void start_sound_chips(running_machine *machine)
{
	int sndnum;

	/* reset the sound array */
	memset(sound, 0, sizeof(sound));

	/* start up all the sound chips */
	for (sndnum = 0; sndnum < MAX_SOUND; sndnum++)
	{
		const sound_config *msound = &machine->config->sound[sndnum];
		sound_info *info;
		int num_regs;
		int index;

		/* stop when we hit an empty entry */
		if (msound->type == SOUND_DUMMY)
			break;
		totalsnd++;

		/* zap all the info */
		info = &sound[sndnum];
		memset(info, 0, sizeof(*info));

		/* copy in all the relevant info */
		info->sound = msound;

		/* start the chip, tagging all its streams */
		VPRINTF(("sndnum = %d -- sound_type = %d\n", sndnum, msound->type));
		num_regs = state_save_get_reg_count(machine);
		streams_set_tag(machine, info);
		if (sndintrf_init_sound(machine, sndnum, msound->tag, msound->type, msound->clock, msound->config) != 0)
			fatalerror("Sound chip #%d (%s) failed to initialize!", sndnum, sndnum_get_name(sndnum));

		/* if no state registered for saving, we can't save */
		num_regs = state_save_get_reg_count(machine) - num_regs;
		if (num_regs == 0)
		{
			logerror("Sound chip #%d (%s) did not register any state to save!\n", sndnum, sndnum_get_name(sndnum));
			if (machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
				fatalerror("Sound chip #%d (%s) did not register any state to save!", sndnum, sndnum_get_name(sndnum));
		}

		/* now count the outputs */
		VPRINTF(("Counting outputs\n"));
		for (index = 0; ; index++)
		{
			sound_stream *stream = stream_find_by_tag(machine, info, index);
			if (!stream)
				break;
			info->outputs += stream_get_outputs(stream);
			VPRINTF(("  stream %p, %d outputs\n", stream, stream_get_outputs(stream)));
		}

		/* if we have outputs, examine them */
		if (info->outputs)
		{
			/* allocate an array to hold them */
			info->output = auto_malloc(info->outputs * sizeof(*info->output));
			VPRINTF(("  %d outputs total\n", info->outputs));

			/* now fill the array */
			info->outputs = 0;
			for (index = 0; ; index++)
			{
				sound_stream *stream = stream_find_by_tag(machine, info, index);
				int outputs, outputnum;

				if (!stream)
					break;
				outputs = stream_get_outputs(stream);

				/* fill in an entry for each output */
				for (outputnum = 0; outputnum < outputs; outputnum++)
				{
					info->output[info->outputs].stream = stream;
					info->output[info->outputs].output = outputnum;
					info->outputs++;
				}
			}
		}
	}
}


/*-------------------------------------------------
    route_sound - route sound outputs to target
    inputs
-------------------------------------------------*/

static void route_sound(running_machine *machine)
{
	int sndnum, routenum, outputnum;
	const device_config *curspeak;

	/* iterate over all the sound chips */
	for (sndnum = 0; sndnum < totalsnd; sndnum++)
	{
		sound_info *info = &sound[sndnum];

		/* iterate over all routes */
		for (routenum = 0; routenum < info->sound->routes; routenum++)
		{
			const sound_route *mroute = &info->sound->route[routenum];
			speaker_info *speaker;
			sound_info *sound;

			/* find the target */
			speaker = find_speaker_by_tag(machine, mroute->target);
			sound = find_sound_by_tag(mroute->target);

			/* if neither found, it's fatal */
			if (speaker == NULL && sound == NULL)
				fatalerror("Sound route \"%s\" not found!\n", mroute->target);

			/* if we got a speaker, bump its input count */
			if (speaker != NULL)
			{
				if (mroute->output >= 0 && mroute->output < info->outputs)
					speaker->inputs++;
				else if (mroute->output == ALL_OUTPUTS)
					speaker->inputs += info->outputs;
			}
		}
	}

	/* now allocate the mixers and input data */
	streams_set_tag(machine, NULL);
	for (curspeak = speaker_output_first(machine->config); curspeak != NULL; curspeak = speaker_output_next(curspeak))
	{
		speaker_info *info = curspeak->token;
		if (info->inputs != 0)
		{
			info->mixer_stream = stream_create(curspeak, info->inputs, 1, machine->sample_rate, info, mixer_update);
			state_save_register_postload(machine, mixer_postload, info->mixer_stream);
			info->input = auto_malloc(info->inputs * sizeof(*info->input));
			info->inputs = 0;
		}
		else
			logerror("Warning: speaker \"%s\" has no inputs\n", info->tag);
	}

	/* iterate again over all the sound chips */
	for (sndnum = 0; sndnum < totalsnd; sndnum++)
	{
		sound_info *info = &sound[sndnum];

		/* iterate over all routes */
		for (routenum = 0; routenum < info->sound->routes; routenum++)
		{
			const sound_route *mroute = &info->sound->route[routenum];
			speaker_info *speaker;
			sound_info *sound;

			/* find the target */
			speaker = find_speaker_by_tag(machine, mroute->target);
			sound = find_sound_by_tag(mroute->target);

			/* if it's a speaker, set the input */
			if (speaker != NULL)
			{
				for (outputnum = 0; outputnum < info->outputs; outputnum++)
					if (mroute->output == outputnum || mroute->output == ALL_OUTPUTS)
					{
						char namebuf[256];
						int index;

						sound_type sndtype = sndnum_to_sndti(sndnum, &index);

						/* built the display name */
                        namebuf[0] = '\0';

                        /* speaker name, if more than one speaker */
						if (speaker_output_count(machine->config) > 1)
							sprintf(namebuf, "%sSpeaker '%s': ", namebuf, speaker->tag);

                        /* device name */
						sprintf(namebuf, "%s%s ", namebuf, sndnum_get_name(sndnum));

						/* device index, if more than one of this type */
						if (sndtype_count(sndtype) > 1)
							sprintf(namebuf, "%s#%d ", namebuf, index);

						/* channel number, if more than channel for this device */
						if (info->outputs > 1)
							sprintf(namebuf, "%sCh.%d", namebuf, outputnum);

						/* remove final space */
						if (namebuf[strlen(namebuf) - 1] == ' ')
	                        namebuf[strlen(namebuf) - 1] = '\0';

						/* fill in the input data on this speaker */
						speaker->input[speaker->inputs].gain = mroute->gain;
						speaker->input[speaker->inputs].default_gain = mroute->gain;
						speaker->input[speaker->inputs].name = auto_strdup(namebuf);

						/* connect the output to the input */
						stream_set_input(speaker->mixer_stream, speaker->inputs++, info->output[outputnum].stream, info->output[outputnum].output, mroute->gain);
					}
			}

			/* if it's a sound chip, set the input */
			else
			{
				if (mroute->input < 0)
				{
					for (outputnum = 0; outputnum < info->outputs; outputnum++)
						if (mroute->output == outputnum || mroute->output == ALL_OUTPUTS)
							stream_set_input(sound->output[0].stream, 0, info->output[outputnum].stream, info->output[outputnum].output, mroute->gain);
				}
				else
				{
					assert(mroute->output != ALL_OUTPUTS);
					for (outputnum = 0; outputnum < info->outputs; outputnum++)
						if (mroute->output == outputnum)
							stream_set_input(sound->output[0].stream, mroute->input, info->output[outputnum].stream, info->output[outputnum].output, mroute->gain);
				}

			}
		}
	}
}



/***************************************************************************
    GLOBAL STATE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    sound_reset - reset all sound chips
-------------------------------------------------*/

static void sound_reset(running_machine *machine)
{
	int sndnum;

	/* reset all the sound chips */
	for (sndnum = 0; sndnum < MAX_SOUND; sndnum++)
		if (machine->config->sound[sndnum].type != SOUND_DUMMY)
			sndnum_reset(sndnum);
}


/*-------------------------------------------------
    sound_pause - pause sound output
-------------------------------------------------*/

static void sound_pause(running_machine *machine, int pause)
{
	if (pause)
		sound_muted |= 0x02;
	else
		sound_muted &= ~0x02;
	osd_set_mastervolume(sound_muted ? -32 : sound_attenuation);
}


/*-------------------------------------------------
    sound_pause - pause sound output
-------------------------------------------------*/

void sound_mute(int mute)
{
	if (mute)
		sound_muted |= 0x01;
	else
		sound_muted &= ~0x01;
	osd_set_mastervolume(sound_muted ? -32 : sound_attenuation);
}


/*-------------------------------------------------
    sound_set_attenuation - set the global volume
-------------------------------------------------*/

void sound_set_attenuation(int attenuation)
{
	sound_attenuation = attenuation;
	osd_set_mastervolume(sound_muted ? -32 : sound_attenuation);
}


/*-------------------------------------------------
    sound_get_attenuation - return the global
    volume
-------------------------------------------------*/

int sound_get_attenuation(void)
{
	return sound_attenuation;
}


/*-------------------------------------------------
    sound_global_enable - enable/disable sound
    globally
-------------------------------------------------*/

void sound_global_enable(int enable)
{
	global_sound_enabled = enable;
}



/***************************************************************************
    SOUND SAVE/LOAD
***************************************************************************/

/*-------------------------------------------------
    sound_load - read and apply data from the
    configuration file
-------------------------------------------------*/

static void sound_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *channelnode;
	int mixernum;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* iterate over channel nodes */
	for (channelnode = xml_get_sibling(parentnode->child, "channel"); channelnode; channelnode = xml_get_sibling(channelnode->next, "channel"))
	{
		mixernum = xml_get_attribute_int(channelnode, "index", -1);
		if (mixernum >= 0 && mixernum < MAX_MIXER_CHANNELS)
		{
			float defvol = xml_get_attribute_float(channelnode, "defvol", -1000.0);
			float newvol = xml_get_attribute_float(channelnode, "newvol", -1000.0);
			if (fabs(defvol - sound_get_default_gain(machine, mixernum)) < 1e-6 && newvol != -1000.0)
				sound_set_user_gain(machine, mixernum, newvol);
		}
	}
}


/*-------------------------------------------------
    sound_save - save data to the configuration
    file
-------------------------------------------------*/

static void sound_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	int mixernum;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* iterate over mixer channels */
	if (parentnode != NULL)
		for (mixernum = 0; mixernum < MAX_MIXER_CHANNELS; mixernum++)
		{
			float defvol = sound_get_default_gain(machine, mixernum);
			float newvol = sound_get_user_gain(machine, mixernum);

			if (defvol != newvol)
			{
				xml_data_node *channelnode = xml_add_child(parentnode, "channel", NULL);
				if (channelnode != NULL)
				{
					xml_set_attribute_int(channelnode, "index", mixernum);
					xml_set_attribute_float(channelnode, "defvol", defvol);
					xml_set_attribute_float(channelnode, "newvol", newvol);
				}
			}
		}
}



/***************************************************************************
    MIXING STAGE
***************************************************************************/

/*-------------------------------------------------
    sound_update - mix everything down to
    its final form and send it to the OSD layer
-------------------------------------------------*/

static TIMER_CALLBACK( sound_update )
{
	UINT32 finalmix_step, finalmix_offset;
	const device_config *curspeak;
	int samples_this_update = 0;
	int sample;

	VPRINTF(("sound_update\n"));

	profiler_mark(PROFILER_SOUND);

	/* force all the speaker streams to generate the proper number of samples */
	for (curspeak = speaker_output_first(machine->config); curspeak != NULL; curspeak = speaker_output_next(curspeak))
	{
		speaker_info *spk = curspeak->token;
		const stream_sample_t *stream_buf;

		/* get the output buffer */
		if (spk->mixer_stream != NULL)
		{
			int numsamples;

			/* update the stream, getting the start/end pointers around the operation */
			stream_buf = stream_get_output_since_last_update(spk->mixer_stream, 0, &numsamples);

			/* set or assert that all streams have the same count */
			if (samples_this_update == 0)
			{
				samples_this_update = numsamples;

				/* reset the mixing streams */
				memset(leftmix, 0, samples_this_update * sizeof(*leftmix));
				memset(rightmix, 0, samples_this_update * sizeof(*rightmix));
			}
			assert(samples_this_update == numsamples);

#ifdef MAME_DEBUG
			/* debug version: keep track of the maximum sample */
			for (sample = 0; sample < samples_this_update; sample++)
			{
				if (stream_buf[sample] > spk->max_sample)
					spk->max_sample = stream_buf[sample];
				else if (-stream_buf[sample] > spk->max_sample)
					spk->max_sample = -stream_buf[sample];
				if (stream_buf[sample] > 32767 || stream_buf[sample] < -32768)
					spk->clipped_samples++;
				spk->total_samples++;
			}
#endif

			/* mix if sound is enabled */
			if (global_sound_enabled && !nosound_mode)
			{
				/* if the speaker is centered, send to both left and right */
				if (spk->speaker->x == 0)
					for (sample = 0; sample < samples_this_update; sample++)
					{
						leftmix[sample] += stream_buf[sample];
						rightmix[sample] += stream_buf[sample];
					}

				/* if the speaker is to the left, send only to the left */
				else if (spk->speaker->x < 0)
					for (sample = 0; sample < samples_this_update; sample++)
						leftmix[sample] += stream_buf[sample];

				/* if the speaker is to the right, send only to the right */
				else
					for (sample = 0; sample < samples_this_update; sample++)
						rightmix[sample] += stream_buf[sample];
			}
		}
	}

	/* now downmix the final result */
	finalmix_step = video_get_speed_factor();
	finalmix_offset = 0;
	for (sample = finalmix_leftover; sample < samples_this_update * 100; sample += finalmix_step)
	{
		int sampindex = sample / 100;
		INT32 samp;

		/* clamp the left side */
		samp = leftmix[sampindex];
		if (samp < -32768)
			samp = -32768;
		else if (samp > 32767)
			samp = 32767;
		finalmix[finalmix_offset++] = samp;

		/* clamp the right side */
		samp = rightmix[sampindex];
		if (samp < -32768)
			samp = -32768;
		else if (samp > 32767)
			samp = 32767;
		finalmix[finalmix_offset++] = samp;
	}
	finalmix_leftover = sample - samples_this_update * 100;

	/* play the result */
	if (finalmix_offset > 0)
	{
		osd_update_audio_stream(machine, finalmix, finalmix_offset / 2);
		video_avi_add_sound(machine, finalmix, finalmix_offset / 2);
		if (wavfile != NULL)
			wav_add_data_16(wavfile, finalmix, finalmix_offset);
	}

	/* update the streamer */
	streams_update(machine);

	profiler_mark(PROFILER_END);
}


/*-------------------------------------------------
    mixer_update - mix all inputs to one output
-------------------------------------------------*/

static void mixer_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	speaker_info *speaker = param;
	int numinputs = speaker->inputs;
	int pos;

	VPRINTF(("Mixer_update(%d)\n", length));

	/* loop over samples */
	for (pos = 0; pos < length; pos++)
	{
		INT32 sample = inputs[0][pos];
		int inp;

		/* add up all the inputs */
		for (inp = 1; inp < numinputs; inp++)
			sample += inputs[inp][pos];
		buffer[0][pos] = sample;
	}
}


/*-------------------------------------------------
    mixer_postload - postload function to reset
    the mixer stream to the proper sample rate
-------------------------------------------------*/

static STATE_POSTLOAD( mixer_postload )
{
	sound_stream *stream = param;
	stream_set_sample_rate(stream, machine->sample_rate);
}



/***************************************************************************
    SPEAKER OUTPUT DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    speaker_output_start - device start callback
    for a speaker
-------------------------------------------------*/

static DEVICE_START( speaker_output )
{
	speaker_info *info = device->token;

	/* copy in all the relevant info */
	info->speaker = device->inline_config;
	info->tag = device->tag;

	return DEVICE_START_OK;
}


/*-------------------------------------------------
    speaker_output_stop - device stop callback
    for a speaker
-------------------------------------------------*/

static DEVICE_STOP( speaker_output )
{
#ifdef MAME_DEBUG
	speaker_info *info = device->token;

	/* log the maximum sample values for all speakers */
	if (info->max_sample > 0)
		mame_printf_debug("Speaker \"%s\" - max = %d (gain *= %f) - %d%% samples clipped\n", info->tag, info->max_sample, 32767.0 / (info->max_sample ? info->max_sample : 1), (int)((double)info->clipped_samples * 100.0 / info->total_samples));
#endif /* MAME_DEBUG */
}


/*-------------------------------------------------
    speaker_output_set_info - device set info
    callback
-------------------------------------------------*/

static DEVICE_SET_INFO( speaker_output )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


/*-------------------------------------------------
    speaker_output_get_info - device get info
    callback
-------------------------------------------------*/

DEVICE_GET_INFO( speaker_output )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(speaker_info);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(speaker_config);		break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_AUDIO;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = DEVICE_SET_INFO_NAME(speaker_output); break;
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(speaker_output); break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME(speaker_output); break;
		case DEVINFO_FCT_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "Speaker";					break;
		case DEVINFO_STR_FAMILY:						info->s = "Sound";						break;
		case DEVINFO_STR_VERSION:						info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}



/***************************************************************************
    MISCELLANEOUS HELPERS
***************************************************************************/

/*-------------------------------------------------
    sndti_set_output_gain - set the gain of a
    particular output
-------------------------------------------------*/

void sndti_set_output_gain(sound_type type, int index, int output, float gain)
{
	int sndnum = sndti_to_sndnum(type, index);

	if (sndnum < 0)
	{
		logerror("sndti_set_output_gain called for invalid sound type %d, index %d\n", type, index);
		return;
	}
	if (output >= sound[sndnum].outputs)
	{
		logerror("sndti_set_output_gain called for invalid sound output %d (type %d, index %d)\n", output, type, index);
		return;
	}
	stream_set_output_gain(sound[sndnum].output[output].stream, sound[sndnum].output[output].output, gain);
}



/***************************************************************************
    USER GAIN CONTROLS
***************************************************************************/

/*-------------------------------------------------
    index_to_input - map an absolute index to
    a particular input
-------------------------------------------------*/

INLINE speaker_info *index_to_input(running_machine *machine, int index, int *input)
{
	const device_config *curspeak;
	int count = 0;

	/* scan through the speakers until we find the indexed input */
	for (curspeak = speaker_output_first(machine->config); curspeak != NULL; curspeak = speaker_output_next(curspeak))
	{
		speaker_info *info = curspeak->token;
		if (index < count + info->inputs)
		{
			*input = index - count;
			return info;
		}
		count += info->inputs;
	}

	/* index out of range */
	return NULL;
}


/*-------------------------------------------------
    sound_get_user_gain_count - return the number
    of user-controllable gain parameters
-------------------------------------------------*/

int sound_get_user_gain_count(running_machine *machine)
{
	const device_config *curspeak;
	int count = 0;

	/* count up the number of speaker inputs */
	for (curspeak = speaker_output_first(machine->config); curspeak != NULL; curspeak = speaker_output_next(curspeak))
	{
		speaker_info *info = curspeak->token;
		count += info->inputs;
	}
	return count;
}


/*-------------------------------------------------
    sound_set_user_gain - set the nth user gain
    value
-------------------------------------------------*/

void sound_set_user_gain(running_machine *machine, int index, float gain)
{
	int inputnum;
	speaker_info *spk = index_to_input(machine, index, &inputnum);

	if (spk != NULL)
	{
		spk->input[inputnum].gain = gain;
		stream_set_input_gain(spk->mixer_stream, inputnum, gain);
	}
}


/*-------------------------------------------------
    sound_get_user_gain - get the nth user gain
    value
-------------------------------------------------*/

float sound_get_user_gain(running_machine *machine, int index)
{
	int inputnum;
	speaker_info *spk = index_to_input(machine, index, &inputnum);
	return (spk != NULL) ? spk->input[inputnum].gain : 0;
}


/*-------------------------------------------------
    sound_get_default_gain - return the default
    gain of the nth user value
-------------------------------------------------*/

float sound_get_default_gain(running_machine *machine, int index)
{
	int inputnum;
	speaker_info *spk = index_to_input(machine, index, &inputnum);
	return (spk != NULL) ? spk->input[inputnum].default_gain : 0;
}


/*-------------------------------------------------
    sound_get_user_gain_name - return the name
    of the nth user value
-------------------------------------------------*/

const char *sound_get_user_gain_name(running_machine *machine, int index)
{
	int inputnum;
	speaker_info *spk = index_to_input(machine, index, &inputnum);
	return (spk != NULL) ? spk->input[inputnum].name : NULL;
}


/*-------------------------------------------------
    sound_find_sndnum_by_tag - return a sndnum
    by finding the appropriate tag
-------------------------------------------------*/

int sound_find_sndnum_by_tag(const char *tag)
{
	int index;

	/* find a match */
	for (index = 0; index < MAX_SOUND; index++)
		if (sound[index].sound != NULL && sound[index].sound->tag != NULL)
			if (strcmp(tag, sound[index].sound->tag) == 0)
				return index;

	return -1;
}
