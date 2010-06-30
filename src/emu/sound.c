/***************************************************************************

    sound.c

    Core sound functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "streams.h"
#include "config.h"
#include "profiler.h"
#include "sound/wavwrite.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE			(0)

#define VPRINTF(x)		do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_MIXER_CHANNELS		100



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _sound_private
{
	emu_timer *update_timer;

	int totalsnd;

	UINT32 finalmix_leftover;
	INT16 *finalmix;
	INT32 *leftmix;
	INT32 *rightmix;

	int muted;
	int attenuation;
	int enabled;
	int nosound_mode;

	wav_file *wavfile;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void sound_reset(running_machine &machine);
static void sound_exit(running_machine &machine);
static void sound_pause(running_machine &machine);
static void sound_resume(running_machine &machine);
static void sound_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void sound_save(running_machine *machine, int config_type, xml_data_node *parentnode);
static TIMER_CALLBACK( sound_update );
static void route_sound(running_machine *machine);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  index_to_input - map an absolute index to
//  a particular input
//-------------------------------------------------

INLINE speaker_device *index_to_input(running_machine *machine, int index, int &input)
{
	// scan through the speakers until we find the indexed input
	for (speaker_device *speaker = speaker_first(*machine); speaker != NULL; speaker = speaker_next(speaker))
	{
		if (index < speaker->inputs())
		{
			input = index;
			return speaker;
		}
		index -= speaker->inputs();
	}

	// index out of range
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
	sound_private *global;
	const char *filename;

	machine->sound_data = global = auto_alloc_clear(machine, sound_private);

	/* handle -nosound */
	global->nosound_mode = !options_get_bool(machine->options(), OPTION_SOUND);
	if (global->nosound_mode)
		machine->sample_rate = 11025;

	/* count the speakers */
	VPRINTF(("total speakers = %d\n", speaker_output_count(machine->config)));

	/* allocate memory for mix buffers */
	global->leftmix = auto_alloc_array(machine, INT32, machine->sample_rate);
	global->rightmix = auto_alloc_array(machine, INT32, machine->sample_rate);
	global->finalmix = auto_alloc_array(machine, INT16, machine->sample_rate);

	/* allocate a global timer for sound timing */
	global->update_timer = timer_alloc(machine, sound_update, NULL);
	timer_adjust_periodic(global->update_timer, STREAMS_UPDATE_ATTOTIME, 0, STREAMS_UPDATE_ATTOTIME);

	/* finally, do all the routing */
	VPRINTF(("route_sound\n"));
	route_sound(machine);

	/* open the output WAV file if specified */
	filename = options_get_string(machine->options(), OPTION_WAVWRITE);
	if (filename[0] != 0)
		global->wavfile = wav_open(filename, machine->sample_rate, 2);

	/* enable sound by default */
	global->enabled = TRUE;
	global->muted = FALSE;
	sound_set_attenuation(machine, options_get_int(machine->options(), OPTION_VOLUME));

	/* register callbacks */
	config_register(machine, "mixer", sound_load, sound_save);
	machine->add_notifier(MACHINE_NOTIFY_PAUSE, sound_pause);
	machine->add_notifier(MACHINE_NOTIFY_RESUME, sound_resume);
	machine->add_notifier(MACHINE_NOTIFY_RESET, sound_reset);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, sound_exit);
}


/*-------------------------------------------------
    sound_exit - clean up after ourselves
-------------------------------------------------*/

static void sound_exit(running_machine &machine)
{
	sound_private *global = machine.sound_data;

	/* close any open WAV file */
	if (global->wavfile != NULL)
		wav_close(global->wavfile);
	global->wavfile = NULL;

	/* reset variables */
	global->totalsnd = 0;
}



/***************************************************************************
    INITIALIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    route_sound - route sound outputs to target
    inputs
-------------------------------------------------*/

static void route_sound(running_machine *machine)
{
	/* iterate again over all the sound chips */
	device_sound_interface *sound = NULL;
	for (bool gotone = machine->m_devicelist.first(sound); gotone; gotone = sound->next(sound))
	{
		int numoutputs = stream_get_device_outputs(*sound);

		/* iterate over all routes */
		for (const device_config_sound_interface::sound_route *route = sound->sound_config().m_route_list; route != NULL; route = route->m_next)
		{
			device_t *target_device = machine->device(route->m_target);
			if (target_device->type() == SPEAKER)
				continue;

			int inputnum = route->m_input;

			/* iterate over all outputs, matching any that apply */
			for (int outputnum = 0; outputnum < numoutputs; outputnum++)
				if (route->m_output == outputnum || route->m_output == ALL_OUTPUTS)
				{
					sound_stream *inputstream, *stream;
					int streaminput, streamoutput;

					if (stream_device_input_to_stream_input(target_device, inputnum++, &inputstream, &streaminput))
						if (stream_device_output_to_stream_output(*sound, outputnum, &stream, &streamoutput))
							stream_set_input(inputstream, streaminput, stream, streamoutput, route->m_gain);
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

static void sound_reset(running_machine &machine)
{
	device_sound_interface *sound = NULL;

	/* reset all the sound chips */
	for (bool gotone = machine.m_devicelist.first(sound); gotone; gotone = sound->next(sound))
		sound->device().reset();
}


/*-------------------------------------------------
    sound_pause - pause sound output
-------------------------------------------------*/

static void sound_pause(running_machine &machine)
{
	sound_private *global = machine.sound_data;
	global->muted |= 0x02;
	osd_set_mastervolume(global->muted ? -32 : global->attenuation);
}

static void sound_resume(running_machine &machine)
{
	sound_private *global = machine.sound_data;
	global->muted &= ~0x02;
	osd_set_mastervolume(global->muted ? -32 : global->attenuation);
}


/*-------------------------------------------------
    sound_mute - mute sound output
-------------------------------------------------*/

void sound_mute(running_machine *machine, int mute)
{
	sound_private *global = machine->sound_data;

	if (mute)
		global->muted |= 0x01;
	else
		global->muted &= ~0x01;
	osd_set_mastervolume(global->muted ? -32 : global->attenuation);
}


/*-------------------------------------------------
    sound_set_attenuation - set the global volume
-------------------------------------------------*/

void sound_set_attenuation(running_machine *machine, int attenuation)
{
	sound_private *global = machine->sound_data;
	global->attenuation = attenuation;
	osd_set_mastervolume(global->muted ? -32 : global->attenuation);
}


/*-------------------------------------------------
    sound_get_attenuation - return the global
    volume
-------------------------------------------------*/

int sound_get_attenuation(running_machine *machine)
{
	sound_private *global = machine->sound_data;
	return global->attenuation;
}


/*-------------------------------------------------
    sound_global_enable - enable/disable sound
    globally
-------------------------------------------------*/

void sound_global_enable(running_machine *machine, int enable)
{
	sound_private *global = machine->sound_data;
	global->enabled = enable;
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
	int samples_this_update = 0;
	int sample;
	sound_private *global = machine->sound_data;
	INT16 *finalmix;
	INT32 *leftmix, *rightmix;

	VPRINTF(("sound_update\n"));

	profiler_mark_start(PROFILER_SOUND);

	leftmix = global->leftmix;
	rightmix = global->rightmix;
	finalmix = global->finalmix;

	/* force all the speaker streams to generate the proper number of samples */
	for (speaker_device *speaker = speaker_first(*machine); speaker != NULL; speaker = speaker_next(speaker))
		speaker->mix(leftmix, rightmix, samples_this_update, !global->enabled || global->nosound_mode);

	/* now downmix the final result */
	finalmix_step = video_get_speed_factor();
	finalmix_offset = 0;
	for (sample = global->finalmix_leftover; sample < samples_this_update * 100; sample += finalmix_step)
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
	global->finalmix_leftover = sample - samples_this_update * 100;

	/* play the result */
	if (finalmix_offset > 0)
	{
		osd_update_audio_stream(machine, finalmix, finalmix_offset / 2);
		video_avi_add_sound(machine, finalmix, finalmix_offset / 2);
		if (global->wavfile != NULL)
			wav_add_data_16(global->wavfile, finalmix, finalmix_offset);
	}

	/* update the streamer */
	streams_update(machine);

	profiler_mark_end();
}



//**************************************************************************
//  SPEAKER DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  speaker_device_config - constructor
//-------------------------------------------------

speaker_device_config::speaker_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Speaker", tag, owner, clock),
	  m_x(0.0),
	  m_y(0.0),
	  m_z(0.0)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *speaker_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(speaker_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *speaker_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, speaker_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void speaker_device_config::device_config_complete()
{
	// move inline data into its final home
	m_x = static_cast<double>(static_cast<INT32>(m_inline_data[INLINE_X])) / (double)(1 << 24);
	m_y = static_cast<double>(static_cast<INT32>(m_inline_data[INLINE_Y])) / (double)(1 << 24);
	m_z = static_cast<double>(static_cast<INT32>(m_inline_data[INLINE_Z])) / (double)(1 << 24);
}



//**************************************************************************
//  LIVE SPEAKER DEVICE
//**************************************************************************

//-------------------------------------------------
//  speaker_device - constructor
//-------------------------------------------------

speaker_device::speaker_device(running_machine &_machine, const speaker_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_mixer_stream(NULL),
	  m_inputs(0),
	  m_input(NULL)
#ifdef MAME_DEBUG
	,
	  m_max_sample(0),
	  m_clipped_samples(0),
	  m_total_samples(0)
#endif
{
}


//-------------------------------------------------
//  ~speaker_device - destructor
//-------------------------------------------------

speaker_device::~speaker_device()
{
#ifdef MAME_DEBUG
	// log the maximum sample values for all speakers
	if (m_max_sample > 0)
		mame_printf_debug("Speaker \"%s\" - max = %d (gain *= %f) - %d%% samples clipped\n", tag(), m_max_sample, 32767.0 / (m_max_sample ? m_max_sample : 1), (int)((double)m_clipped_samples * 100.0 / m_total_samples));
#endif /* MAME_DEBUG */
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void speaker_device::device_start()
{
	// scan all the sound devices and count our inputs
	int inputs = 0;
	device_sound_interface *sound = NULL;
	for (bool gotone = machine->m_devicelist.first(sound); gotone; gotone = sound->next(sound))
	{
		// scan each route on the device
		for (const device_config_sound_interface::sound_route *route = sound->sound_config().m_route_list; route != NULL; route = route->m_next)
		{
			// if we are the target of this route, accumulate inputs
			device_t *target_device = machine->device(route->m_target);
			if (target_device == this)
			{
				// if the sound device is not yet started, bail however -- we need the its stream
				if (!sound->device().started())
					throw device_missing_dependencies();

				// accumulate inputs
				inputs += (route->m_output == ALL_OUTPUTS) ? stream_get_device_outputs(*sound) : 1;
			}
		}
	}

	// no inputs? that's weird
	if (inputs == 0)
	{
		logerror("Warning: speaker \"%s\" has no inputs\n", tag());
		return;
	}

	// now we know how many inputs; allocate the mixers and input data
	m_mixer_stream = stream_create(this, inputs, 1, machine->sample_rate, NULL, static_mixer_update);
	m_input = auto_alloc_array(machine, speaker_input, inputs);
	m_inputs = 0;

	// iterate again over all the sound devices
	for (bool gotone = machine->m_devicelist.first(sound); gotone; gotone = sound->next(sound))
	{
		// scan each route on the device
		for (const device_config_sound_interface::sound_route *route = sound->sound_config().m_route_list; route != NULL; route = route->m_next)
		{
			// if we are the target of this route, hook it up
			device_t *target_device = machine->device(route->m_target);
			if (target_device == this)
			{
				// iterate over all outputs, matching any that apply
				int numoutputs = stream_get_device_outputs(*sound);
				for (int outputnum = 0; outputnum < numoutputs; outputnum++)
					if (route->m_output == outputnum || route->m_output == ALL_OUTPUTS)
					{
						// fill in the input data on this speaker
						m_input[m_inputs].m_gain = route->m_gain;
						m_input[m_inputs].m_default_gain = route->m_gain;
						m_input[m_inputs].m_name.printf("Speaker '%s': %s '%s'", tag(), sound->device().name(), sound->device().tag());
						if (numoutputs > 1)
							m_input[m_inputs].m_name.catprintf(" Ch.%d", outputnum);

						// connect the output to the input
						sound_stream *stream;
						int streamoutput;
						if (stream_device_output_to_stream_output(*sound, outputnum, &stream, &streamoutput))
							stream_set_input(m_mixer_stream, m_inputs++, stream, streamoutput, route->m_gain);
					}
			}
		}
	}
}


//-------------------------------------------------
//  device_post_load - after we load a save state
//  be sure to update the mixer stream's output
//  sample rate
//-------------------------------------------------

void speaker_device::device_post_load()
{
	stream_set_sample_rate(m_mixer_stream, machine->sample_rate);
}


//-------------------------------------------------
//  mixer_update - mix all inputs to one output
//-------------------------------------------------

void speaker_device::mixer_update(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	VPRINTF(("Mixer_update(%d)\n", samples));

	// loop over samples
	for (int pos = 0; pos < samples; pos++)
	{
		INT32 sample = inputs[0][pos];
		int inp;

		// add up all the inputs
		for (inp = 1; inp < m_inputs; inp++)
			sample += inputs[inp][pos];
		outputs[0][pos] = sample;
	}
}


//-------------------------------------------------
//  mix - mix in samples from the speaker's stream
//-------------------------------------------------

void speaker_device::mix(INT32 *leftmix, INT32 *rightmix, int &samples_this_update, bool suppress)
{
	// skip if no stream
	if (m_mixer_stream == NULL)
		return;

	// update the stream, getting the start/end pointers around the operation
	int numsamples;
	const stream_sample_t *stream_buf = stream_get_output_since_last_update(m_mixer_stream, 0, &numsamples);

	// set or assert that all streams have the same count
	if (samples_this_update == 0)
	{
		samples_this_update = numsamples;

		/* reset the mixing streams */
		memset(leftmix, 0, samples_this_update * sizeof(*leftmix));
		memset(rightmix, 0, samples_this_update * sizeof(*rightmix));
	}
	assert(samples_this_update == numsamples);

#ifdef MAME_DEBUG
	// debug version: keep track of the maximum sample
	for (int sample = 0; sample < samples_this_update; sample++)
	{
		if (stream_buf[sample] > m_max_sample)
			m_max_sample = stream_buf[sample];
		else if (-stream_buf[sample] > m_max_sample)
			m_max_sample = -stream_buf[sample];
		if (stream_buf[sample] > 32767 || stream_buf[sample] < -32768)
			m_clipped_samples++;
		m_total_samples++;
	}
#endif

	// mix if sound is enabled
	if (!suppress)
	{
		// if the speaker is centered, send to both left and right
		if (m_config.m_x == 0)
			for (int sample = 0; sample < samples_this_update; sample++)
			{
				leftmix[sample] += stream_buf[sample];
				rightmix[sample] += stream_buf[sample];
			}

		// if the speaker is to the left, send only to the left
		else if (m_config.m_x < 0)
			for (int sample = 0; sample < samples_this_update; sample++)
				leftmix[sample] += stream_buf[sample];

		// if the speaker is to the right, send only to the right
		else
			for (int sample = 0; sample < samples_this_update; sample++)
				rightmix[sample] += stream_buf[sample];
	}
}


//-------------------------------------------------
//  set_input_gain - set the gain on a given
//  input
//-------------------------------------------------

void speaker_device::set_input_gain(int inputnum, float gain)
{
	m_input[inputnum].m_gain = gain;
	stream_set_input_gain(m_mixer_stream, inputnum, gain);
}



/***************************************************************************
    MISCELLANEOUS HELPERS
***************************************************************************/

/*-------------------------------------------------
    sound_set_output_gain - set the gain of a
    particular output
-------------------------------------------------*/

void sound_set_output_gain(device_t *device, int output, float gain)
{
	sound_stream *stream;
	int outputnum;

	if (stream_device_output_to_stream_output(device, output, &stream, &outputnum))
		stream_set_output_gain(stream, outputnum, gain);
}



/***************************************************************************
    USER GAIN CONTROLS
***************************************************************************/

/*-------------------------------------------------
    sound_get_user_gain_count - return the number
    of user-controllable gain parameters
-------------------------------------------------*/

int sound_get_user_gain_count(running_machine *machine)
{
	// count up the number of speaker inputs
	int count = 0;
	for (speaker_device *speaker = speaker_first(*machine); speaker != NULL; speaker = speaker_next(speaker))
		count += speaker->inputs();

	return count;
}


/*-------------------------------------------------
    sound_set_user_gain - set the nth user gain
    value
-------------------------------------------------*/

void sound_set_user_gain(running_machine *machine, int index, float gain)
{
	int inputnum;
	speaker_device *speaker = index_to_input(machine, index, inputnum);

	if (speaker != NULL)
		speaker->set_input_gain(inputnum, gain);
}


/*-------------------------------------------------
    sound_get_user_gain - get the nth user gain
    value
-------------------------------------------------*/

float sound_get_user_gain(running_machine *machine, int index)
{
	int inputnum;
	speaker_device *speaker = index_to_input(machine, index, inputnum);
	return (speaker != NULL) ? speaker->input_gain(inputnum) : 0;
}


/*-------------------------------------------------
    sound_get_default_gain - return the default
    gain of the nth user value
-------------------------------------------------*/

float sound_get_default_gain(running_machine *machine, int index)
{
	int inputnum;
	speaker_device *speaker = index_to_input(machine, index, inputnum);
	return (speaker != NULL) ? speaker->input_default_gain(inputnum) : 0;
}


/*-------------------------------------------------
    sound_get_user_gain_name - return the name
    of the nth user value
-------------------------------------------------*/

const char *sound_get_user_gain_name(running_machine *machine, int index)
{
	int inputnum;
	speaker_device *speaker = index_to_input(machine, index, inputnum);
	return (speaker != NULL) ? speaker->input_name(inputnum) : 0;
}

const device_type SPEAKER = speaker_device_config::static_alloc_device_config;
