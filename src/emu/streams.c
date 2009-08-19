/***************************************************************************

    streams.c

    Handle general purpose audio streams

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Streaming works as follows:

    Each stream can have 'n' inputs and 'm' outputs. The inputs for a
    stream are the outputs from another stream. Note that for tracking
    purposes, each stream tracks its input streams directly, but does
    not explicitly track its output streams. Instead, each output simply
    tracks the number of dependent input streams.

    Each stream has a sample rate. This rate controls the sample rate
    of the outputs. All outputs on a stream output at the same sample
    rate.

    Each stream also has a callback function. This function is called
    with an array of input sample streams, and an array out output
    sample streams. The input sample streams are automatically resampled
    by the streaming engine to match the stream's current sample rate.
    The output sample streams are expected to be generated at the
    stream's current sample rate.

    Before the callback can be invoked, all the inputs that flow into it
    must be updated as well. However, each stream can have an independent
    sample  rate, so this isn't as easy as it sounds.

    To update a stream, the engine must iterate over all the inputs. For
    each input, it requests that input to update to the current time.
    Then it resamples the input data into a local resample buffer at the
    stream's sample rate. Once all inputs are up-to-date, it calls the
    callback with the array of resampled input sample buffers. The
    callback is expected to fill in the array of output sample buffers.
    These sample buffers can then be further resampled and passed to
    other streams, or output as desired.

***************************************************************************/

#include "driver.h"
#include "streams.h"
#include "profiler.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE			(0)

#define VPRINTF(x)	do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define OUTPUT_BUFFER_UPDATES			(5)

#define FRAC_BITS						22
#define FRAC_ONE						(1 << FRAC_BITS)
#define FRAC_MASK						(FRAC_ONE - 1)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _stream_input stream_input;
typedef struct _stream_output stream_output;

struct _stream_input
{
	/* linking information */
	sound_stream *		owner;					/* pointer to the owning stream */
	stream_output *		source;					/* pointer to the sound_output for this source */

	/* resample buffer */
	stream_sample_t *	resample;				/* buffer for resampling to the stream's sample rate */
	UINT32				bufsize;				/* size of output buffer, in samples */
	UINT32				bufalloc;				/* allocated size of output buffer, in samples */

	/* resampling information */
	attoseconds_t		latency_attoseconds;	/* latency between this stream and the input stream */
	INT16				gain;					/* gain to apply to this input */
};


struct _stream_output
{
	/* linking information */
	sound_stream *		owner;					/* pointer to the owning stream */

	/* output buffer */
	stream_sample_t *	buffer;					/* output buffer */

	/* output buffer position */
	int					dependents;				/* number of dependents */
	INT16				gain;					/* gain to apply to the output */
};


struct _sound_stream
{
	/* linking information */
	const device_config *	device;				/* owning device */
	sound_stream *		next;					/* next stream in the chain */
	int					index;					/* index for save states */

	/* general information */
	UINT32				sample_rate;			/* sample rate of this stream */
	UINT32				new_sample_rate;		/* newly-set sample rate for the stream */

	/* timing information */
	attoseconds_t		attoseconds_per_sample;	/* number of attoseconds per sample */
	INT32				max_samples_per_update;	/* maximum samples per update */

	/* input information */
	int					inputs;					/* number of inputs */
	stream_input *		input;					/* list of streams we directly depend upon */
	stream_sample_t **	input_array;			/* array of inputs for passing to the callback */

	/* resample buffer information */
	UINT32				resample_bufalloc;		/* allocated size of each resample buffer */

	/* output information */
	int					outputs;				/* number of outputs */
	stream_output *		output;					/* list of streams which directly depend upon us */
	stream_sample_t **	output_array;			/* array of outputs for passing to the callback */

	/* output buffer information */
	UINT32				output_bufalloc;		/* allocated size of each output buffer */
	INT32				output_sampindex;		/* current position within each output buffer */
	INT32				output_update_sampindex;/* position at time of last global update */
	INT32				output_base_sampindex;	/* sample at base of buffer, relative to the current emulated second */

	/* callback information */
	stream_update_func 	callback;				/* callback function */
	void *				param;					/* callback function parameter */
};


struct _streams_private
{
	sound_stream *		stream_head;			/* pointer to first stream */
	sound_stream **		stream_tailptr;			/* pointer to pointer to last stream */
	int					stream_index;			/* index of the current stream */
	attoseconds_t		update_attoseconds;		/* attoseconds between global updates */
	attotime			last_update;			/* last update time */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static STATE_POSTLOAD( stream_postload );
static void allocate_resample_buffers(running_machine *machine, sound_stream *stream);
static void allocate_output_buffers(running_machine *machine, sound_stream *stream);
static void recompute_sample_rate_data(running_machine *machine, sound_stream *stream);
static void generate_samples(sound_stream *stream, int samples);
static stream_sample_t *generate_resampled_data(stream_input *input, UINT32 numsamples);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    time_to_sampindex - convert an absolute
    time to a sample index in a given stream
-------------------------------------------------*/

INLINE INT32 time_to_sampindex(const streams_private *strdata, const sound_stream *stream, attotime time)
{
	/* determine the number of samples since the start of this second */
	INT32 sample = (INT32)(time.attoseconds / stream->attoseconds_per_sample);

	/* if we're ahead of the last update, then adjust upwards */
	if (time.seconds > strdata->last_update.seconds)
	{
		assert(time.seconds == strdata->last_update.seconds + 1);
		sample += stream->sample_rate;
	}

	/* if we're behind the last update, then adjust downwards */
	if (time.seconds < strdata->last_update.seconds)
	{
		assert(time.seconds == strdata->last_update.seconds - 1);
		sample -= stream->sample_rate;
	}
	return sample;
}



/***************************************************************************
    SYSTEM-LEVEL MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    streams_init - initialize the streams engine
-------------------------------------------------*/

void streams_init(running_machine *machine)
{
	streams_private *strdata;

	/* allocate memory for our private data */
	strdata = auto_alloc_clear(machine, streams_private);

	/* reset globals */
	strdata->stream_tailptr = &strdata->stream_head;
	strdata->update_attoseconds = STREAMS_UPDATE_FREQUENCY.attoseconds;

	/* set the global pointer */
	machine->streams_data = strdata;

	/* register global states */
	state_save_register_global(machine, strdata->last_update.seconds);
	state_save_register_global(machine, strdata->last_update.attoseconds);
}


/*-------------------------------------------------
    streams_update - update all the streams
    periodically
-------------------------------------------------*/

void streams_update(running_machine *machine)
{
	streams_private *strdata = machine->streams_data;
	attotime curtime = timer_get_time(machine);
	int second_tick = FALSE;
	sound_stream *stream;

	VPRINTF(("streams_update\n"));

	/* see if we ticked over to the next second */
	if (curtime.seconds != strdata->last_update.seconds)
	{
		assert(curtime.seconds == strdata->last_update.seconds + 1);
		second_tick = TRUE;
	}

	/* iterate over all the streams */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
	{
		INT32 output_bufindex = stream->output_sampindex - stream->output_base_sampindex;
		int outputnum;

		/* make sure this stream is up-to-date */
		stream_update(stream);

		/* if we've ticked over another second, adjust all the counters that are relative to
           the current second */
		if (second_tick)
		{
			stream->output_sampindex -= stream->sample_rate;
			stream->output_base_sampindex -= stream->sample_rate;
		}

		/* note our current output sample */
		stream->output_update_sampindex = stream->output_sampindex;

		/* if we don't have enough output buffer space to hold two updates' worth of samples,
           we need to shuffle things down */
		if (stream->output_bufalloc - output_bufindex < 2 * stream->max_samples_per_update)
		{
			INT32 samples_to_lose = output_bufindex - stream->max_samples_per_update;
			if (samples_to_lose > 0)
			{
				/* if we have samples to move, do so for each output */
				if (output_bufindex > 0)
					for (outputnum = 0; outputnum < stream->outputs; outputnum++)
					{
						stream_output *output = &stream->output[outputnum];
						memmove(&output->buffer[0], &output->buffer[samples_to_lose], sizeof(output->buffer[0]) * (output_bufindex - samples_to_lose));
					}

				/* update the base position */
				stream->output_base_sampindex += samples_to_lose;
			}
		}
	}

	/* remember the update time */
	strdata->last_update = curtime;

	/* update sample rates if they have changed */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
		if (stream->new_sample_rate != 0)
		{
			UINT32 old_rate = stream->sample_rate;
			int outputnum;

			/* update to the new rate and remember the old rate */
			stream->sample_rate = stream->new_sample_rate;
			stream->new_sample_rate = 0;

			/* recompute all the data */
			recompute_sample_rate_data(machine, stream);

			/* reset our sample indexes to the current time */
			stream->output_sampindex = (INT64)stream->output_sampindex * (INT64)stream->sample_rate / old_rate;
			stream->output_update_sampindex = (INT64)stream->output_update_sampindex * (INT64)stream->sample_rate / old_rate;
			stream->output_base_sampindex = stream->output_sampindex - stream->max_samples_per_update;

			/* clear out the buffer */
			for (outputnum = 0; outputnum < stream->outputs; outputnum++)
				memset(stream->output[outputnum].buffer, 0, stream->max_samples_per_update * sizeof(stream->output[outputnum].buffer[0]));
		}
}



/***************************************************************************
    STREAM CONFIGURATION AND SETUP
***************************************************************************/

/*-------------------------------------------------
    stream_create - create a new stream
-------------------------------------------------*/

sound_stream *stream_create(const device_config *device, int inputs, int outputs, int sample_rate, void *param, stream_update_func callback)
{
	running_machine *machine = device->machine;
	streams_private *strdata = machine->streams_data;
	int inputnum, outputnum;
	sound_stream *stream;
	char statetag[30];

	/* allocate memory */
	stream = auto_alloc_clear(device->machine, sound_stream);

	VPRINTF(("stream_create(%d, %d, %d) => %p\n", inputs, outputs, sample_rate, stream));

	/* fill in the data */
	stream->device = device;
	stream->index = strdata->stream_index++;
	stream->sample_rate = sample_rate;
	stream->inputs = inputs;
	stream->outputs = outputs;
	stream->callback = callback;
	stream->param = param;

	/* create a unique tag for saving */
	sprintf(statetag, "%d", stream->index);
	state_save_register_item(machine, "stream", statetag, 0, stream->sample_rate);
	state_save_register_postload(machine, stream_postload, stream);

	/* allocate space for the inputs */
	if (inputs > 0)
	{
		stream->input = auto_alloc_array_clear(device->machine, stream_input, inputs);
		stream->input_array = auto_alloc_array_clear(device->machine, stream_sample_t *, inputs);
	}

	/* initialize the state of each input */
	for (inputnum = 0; inputnum < inputs; inputnum++)
	{
		stream->input[inputnum].owner = stream;
		stream->input[inputnum].gain = 0x100;
		state_save_register_item(machine, "stream", statetag, inputnum, stream->input[inputnum].gain);
	}

	/* allocate space for the outputs */
	if (outputs > 0)
	{
		stream->output = auto_alloc_array_clear(device->machine, stream_output, outputs);
		stream->output_array = auto_alloc_array_clear(device->machine, stream_sample_t *, outputs);
	}

	/* initialize the state of each output */
	for (outputnum = 0; outputnum < outputs; outputnum++)
	{
		stream->output[outputnum].owner = stream;
		stream->output[outputnum].gain = 0x100;
		state_save_register_item(machine, "stream", statetag, outputnum, stream->output[outputnum].gain);
	}

	/* hook us into the master stream list */
	*strdata->stream_tailptr = stream;
	strdata->stream_tailptr = &stream->next;

	/* force an update to the sample rates; this will cause everything to be recomputed
       and will generate the initial resample buffers for our inputs */
	recompute_sample_rate_data(device->machine, stream);

	/* set up the initial output buffer positions now that we have data */
	stream->output_base_sampindex = -stream->max_samples_per_update;

	return stream;
}


/*-------------------------------------------------
    stream_device_output_to_stream_output -
    convert a device/output pair to a stream/
    output pair
-------------------------------------------------*/

int stream_device_output_to_stream_output(const device_config *device, int outputnum, sound_stream **streamptr, int *streamoutputptr)
{
	streams_private *strdata = device->machine->streams_data;
	sound_stream *stream;

	/* scan the list looking for the nth stream that matches the tag */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
		if (stream->device == device)
		{
			if (outputnum < stream->outputs)
			{
				*streamptr = stream;
				*streamoutputptr = outputnum;
				return TRUE;
			}
			outputnum -= stream->outputs;
		}
	return FALSE;
}


/*-------------------------------------------------
    stream_device_input_to_stream_input -
    convert a device/input pair to a stream/
    input pair
-------------------------------------------------*/

int stream_device_input_to_stream_input(const device_config *device, int inputnum, sound_stream **streamptr, int *streaminputptr)
{
	streams_private *strdata = device->machine->streams_data;
	sound_stream *stream;

	/* scan the list looking for the nth stream that matches the tag */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
		if (stream->device == device)
		{
			if (inputnum < stream->inputs)
			{
				*streamptr = stream;
				*streaminputptr = inputnum;
				return TRUE;
			}
			inputnum -= stream->inputs;
		}
	return FALSE;
}


/*-------------------------------------------------
    stream_set_input - configure a stream's input
-------------------------------------------------*/

void stream_set_input(sound_stream *stream, int index, sound_stream *input_stream, int output_index, float gain)
{
	stream_input *input;

	VPRINTF(("stream_set_input(%p, %d, %p, %d, %f)\n", stream, index, input_stream, output_index, gain));

	/* make sure it's a valid input */
	if (index >= stream->inputs)
		fatalerror("Fatal error: stream_set_input attempted to configure non-existant input %d (%d max)", index, stream->inputs);

	/* make sure it's a valid output */
	if (input_stream != NULL && output_index >= input_stream->outputs)
		fatalerror("Fatal error: stream_set_input attempted to use a non-existant output %d (%d max)", output_index, input_stream->outputs);

	/* if this input is already wired, update the dependent info */
	input = &stream->input[index];
	if (input->source != NULL)
		input->source->dependents--;

	/* wire it up */
	input->source = (input_stream != NULL) ? &input_stream->output[output_index] : NULL;
	input->gain = (int)(0x100 * gain);

	/* update the dependent info */
	if (input->source != NULL)
		input->source->dependents++;

	/* update sample rates now that we know the input */
	recompute_sample_rate_data(stream->device->machine, stream);
}


/*-------------------------------------------------
    stream_update - force a stream to update to
    the current emulated time
-------------------------------------------------*/

void stream_update(sound_stream *stream)
{
	running_machine *machine = stream->device->machine;
	streams_private *strdata = machine->streams_data;
	INT32 update_sampindex = time_to_sampindex(strdata, stream, timer_get_time(machine));

	/* generate samples to get us up to the appropriate time */
	profiler_mark_start(PROFILER_SOUND);
	assert(stream->output_sampindex - stream->output_base_sampindex >= 0);
	assert(update_sampindex - stream->output_base_sampindex <= stream->output_bufalloc);
	generate_samples(stream, update_sampindex - stream->output_sampindex);
	profiler_mark_end();

	/* remember this info for next time */
	stream->output_sampindex = update_sampindex;
}


/*-------------------------------------------------
    stream_get_output_since_last_update - return a
    pointer to the output buffer and the number of
    samples since the last global update
-------------------------------------------------*/

const stream_sample_t *stream_get_output_since_last_update(sound_stream *stream, int outputnum, int *numsamples)
{
	stream_output *output = &stream->output[outputnum];

	/* force an update on the stream */
	stream_update(stream);

	/* compute the number of samples and a pointer to the output buffer */
	*numsamples = stream->output_sampindex - stream->output_update_sampindex;
	return output->buffer + (stream->output_update_sampindex - stream->output_base_sampindex);
}



/***************************************************************************
    STREAM TIMING
***************************************************************************/

/*-------------------------------------------------
    stream_get_sample_rate - return the currently
    set sample rate on a given stream
-------------------------------------------------*/

int stream_get_sample_rate(sound_stream *stream)
{
	/* take into account any pending sample rate changes */
	return (stream->new_sample_rate != 0) ? stream->new_sample_rate : stream->sample_rate;
}


/*-------------------------------------------------
    stream_set_sample_rate - set the sample rate
    on a given stream
-------------------------------------------------*/

void stream_set_sample_rate(sound_stream *stream, int sample_rate)
{
	/* we will update this on the next global update */
	if (sample_rate != stream_get_sample_rate(stream))
		stream->new_sample_rate = sample_rate;
}


/*-------------------------------------------------
    stream_get_time - return the emulation time
    of the next sample to be generated on the
    stream
-------------------------------------------------*/

attotime stream_get_time(sound_stream *stream)
{
	streams_private *strdata = stream->device->machine->streams_data;
	attotime base = attotime_make(strdata->last_update.seconds, 0);
	return attotime_add_attoseconds(base, stream->output_sampindex * stream->attoseconds_per_sample);
}


/*-------------------------------------------------
    stream_get_sample_period - return the duration
    of a single sample for a stream
-------------------------------------------------*/

attotime stream_get_sample_period(sound_stream *stream)
{
	return attotime_make(0, stream->attoseconds_per_sample);
}



/***************************************************************************
    STREAM INFORMATION AND CONTROL
***************************************************************************/

/*-------------------------------------------------
    stream_get_device_outputs - return the total
    number of outputs for the given device
-------------------------------------------------*/

int stream_get_device_outputs(const device_config *device)
{
	streams_private *strdata = device->machine->streams_data;
	sound_stream *stream;
	int outputs = 0;

	/* scan the list looking for the nth stream that matches the tag */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
		if (stream->device == device)
			outputs += stream->outputs;
	return outputs;
}


/*-------------------------------------------------
    stream_find_by_device - find a stream using a
    device and index
-------------------------------------------------*/

sound_stream *stream_find_by_device(const device_config *device, int streamindex)
{
	streams_private *strdata = device->machine->streams_data;
	sound_stream *stream;

	/* scan the list looking for the nth stream that matches the tag */
	for (stream = strdata->stream_head; stream != NULL; stream = stream->next)
		if (stream->device == device && streamindex-- == 0)
			return stream;
	return NULL;
}


/*-------------------------------------------------
    stream_get_inputs - return the number of
    inputs for a given stream
-------------------------------------------------*/

int stream_get_inputs(sound_stream *stream)
{
	return stream->inputs;
}


/*-------------------------------------------------
    stream_get_outputs - return the number of
    outputs for a given stream
-------------------------------------------------*/

int stream_get_outputs(sound_stream *stream)
{
	return stream->outputs;
}


/*-------------------------------------------------
    stream_set_input_gain - set the input gain on
    a given stream
-------------------------------------------------*/

void stream_set_input_gain(sound_stream *stream, int input, float gain)
{
	stream_update(stream);
	stream->input[input].gain = (int)(0x100 * gain);
}


/*-------------------------------------------------
    stream_set_output_gain - set the output gain on
    a given stream
-------------------------------------------------*/

void stream_set_output_gain(sound_stream *stream, int output, float gain)
{
	stream_update(stream);
	stream->output[output].gain = (int)(0x100 * gain);
}



/***************************************************************************
    STREAM BUFFER MAINTENANCE
***************************************************************************/

/*-------------------------------------------------
    stream_postload - save/restore callback
-------------------------------------------------*/

static STATE_POSTLOAD( stream_postload )
{
	streams_private *strdata = machine->streams_data;
	sound_stream *stream = (sound_stream *)param;
	int outputnum;

	/* recompute the same rate information */
	recompute_sample_rate_data(machine, stream);

	/* make sure our output buffers are fully cleared */
	for (outputnum = 0; outputnum < stream->outputs; outputnum++)
		memset(stream->output[outputnum].buffer, 0, stream->output_bufalloc * sizeof(stream->output[outputnum].buffer[0]));

	/* recompute the sample indexes to make sense */
	stream->output_sampindex = strdata->last_update.attoseconds / stream->attoseconds_per_sample;
	stream->output_update_sampindex = stream->output_sampindex;
	stream->output_base_sampindex = stream->output_sampindex - stream->max_samples_per_update;
}


/*-------------------------------------------------
    allocate_resample_buffers - recompute the
    resample buffer sizes and expand if necessary
-------------------------------------------------*/

static void allocate_resample_buffers(running_machine *machine, sound_stream *stream)
{
	/* compute the target number of samples */
	INT32 bufsize = 2 * stream->max_samples_per_update;

	/* if we don't have enough room, allocate more */
	if (stream->resample_bufalloc < bufsize)
	{
		int inputnum;

		/* this becomes the new allocation size */
		stream->resample_bufalloc = bufsize;

		/* iterate over outputs and realloc their buffers */
		for (inputnum = 0; inputnum < stream->inputs; inputnum++)
		{
			stream_input *input = &stream->input[inputnum];
			input->resample = auto_extend_array(machine, input->resample, stream_sample_t, stream->resample_bufalloc);
		}
	}
}


/*-------------------------------------------------
    allocate_output_buffers - recompute the
    output buffer sizes and expand if necessary
-------------------------------------------------*/

static void allocate_output_buffers(running_machine *machine, sound_stream *stream)
{
	/* compute the target number of samples */
	INT32 bufsize = OUTPUT_BUFFER_UPDATES * stream->max_samples_per_update;

	/* if we don't have enough room, allocate more */
	if (stream->output_bufalloc < bufsize)
	{
		int outputnum;
		int oldsize;

		/* this becomes the new allocation size */
		oldsize = stream->output_bufalloc;
		stream->output_bufalloc = bufsize;

		/* iterate over outputs and realloc their buffers */
		for (outputnum = 0; outputnum < stream->outputs; outputnum++)
		{
			stream_output *output = &stream->output[outputnum];
			output->buffer = auto_extend_array(machine, output->buffer, stream_sample_t, stream->output_bufalloc);
			memset(&output->buffer[oldsize], 0, (stream->output_bufalloc - oldsize) * sizeof(output->buffer[0]));
		}
	}
}


/*-------------------------------------------------
    recompute_sample_rate_data - recompute sample
    rate data, and all streams that are affected
    by this stream
-------------------------------------------------*/

static void recompute_sample_rate_data(running_machine *machine, sound_stream *stream)
{
	streams_private *strdata = machine->streams_data;
	int inputnum;

	/* recompute the timing parameters */
	stream->attoseconds_per_sample = ATTOSECONDS_PER_SECOND / stream->sample_rate;
	stream->max_samples_per_update = (strdata->update_attoseconds + stream->attoseconds_per_sample - 1) / stream->attoseconds_per_sample;

	/* update resample and output buffer sizes */
	allocate_resample_buffers(machine, stream);
	allocate_output_buffers(machine, stream);

	/* iterate over each input */
	for (inputnum = 0; inputnum < stream->inputs; inputnum++)
	{
		stream_input *input = &stream->input[inputnum];

		/* if we have a source, see if its sample rate changed */
		if (input->source != NULL)
		{
			sound_stream *input_stream = input->source->owner;
			attoseconds_t new_attosecs_per_sample = ATTOSECONDS_PER_SECOND / input_stream->sample_rate;
			attoseconds_t latency;

			/* okay, we have a new sample rate; recompute the latency to be the maximum
               sample period between us and our input */
			latency = MAX(new_attosecs_per_sample, stream->attoseconds_per_sample);

			/* if the input stream's sample rate is lower, we will use linear interpolation */
			/* this requires an extra sample from the source */
			if (input_stream->sample_rate < stream->sample_rate)
				latency += new_attosecs_per_sample;

			/* if our sample rates match exactly, we don't need any latency */
			else if (input_stream->sample_rate == stream->sample_rate)
				latency = 0;

			/* we generally don't want to tweak the latency, so we just keep the greatest
               one we've computed thus far */
			input->latency_attoseconds = MAX(input->latency_attoseconds, latency);
			assert(input->latency_attoseconds < strdata->update_attoseconds);
		}
	}
}



/***************************************************************************
    SOUND GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_samples - generate the requested
    number of samples for a stream, making sure
    all inputs have the appropriate number of
    samples generated
-------------------------------------------------*/

static void generate_samples(sound_stream *stream, int samples)
{
	int inputnum, outputnum;

	/* if we're already there, skip it */
	if (samples <= 0)
		return;

	VPRINTF(("generate_samples(%p, %d)\n", stream, samples));

	/* ensure all inputs are up to date and generate resampled data */
	for (inputnum = 0; inputnum < stream->inputs; inputnum++)
	{
		stream_input *input = &stream->input[inputnum];

		/* update the stream to the current time */
		if (input->source != NULL)
			stream_update(input->source->owner);

		/* generate the resampled data */
		stream->input_array[inputnum] = generate_resampled_data(input, samples);
	}

	/* loop over all outputs and compute the output pointer */
	for (outputnum = 0; outputnum < stream->outputs; outputnum++)
	{
		stream_output *output = &stream->output[outputnum];
		stream->output_array[outputnum] = output->buffer + (stream->output_sampindex - stream->output_base_sampindex);
	}

	/* run the callback */
	VPRINTF(("  callback(%p, %d)\n", stream, samples));
	(*stream->callback)(stream->device, stream->param, stream->input_array, stream->output_array, samples);
	VPRINTF(("  callback done\n"));
}


/*-------------------------------------------------
    generate_resampled_data - generate the
    resample buffer for a given input
-------------------------------------------------*/

static stream_sample_t *generate_resampled_data(stream_input *input, UINT32 numsamples)
{
	stream_sample_t *dest = input->resample;
	stream_output *output = input->source;
	sound_stream *stream = input->owner;
	sound_stream *input_stream;
	stream_sample_t *source;
	stream_sample_t sample;
	attoseconds_t basetime;
	INT32 basesample;
	UINT32 basefrac;
	UINT32 step;
	int gain;

	/* if we don't have an output to pull data from, generate silence */
	if (output == NULL)
	{
		memset(dest, 0, numsamples * sizeof(*dest));
		return input->resample;
	}

	/* grab data from the output */
	input_stream = output->owner;
	gain = (input->gain * output->gain) >> 8;

	/* determine the time at which the current sample begins, accounting for the
       latency we calculated between the input and output streams */
	basetime = stream->output_sampindex * stream->attoseconds_per_sample - input->latency_attoseconds;

	/* now convert that time into a sample in the input stream */
	if (basetime >= 0)
		basesample = basetime / input_stream->attoseconds_per_sample;
	else
		basesample = -(-basetime / input_stream->attoseconds_per_sample) - 1;

	/* compute a source pointer to the first sample */
	assert(basesample >= input_stream->output_base_sampindex);
	source = output->buffer + (basesample - input_stream->output_base_sampindex);

	/* determine the current fraction of a sample */
	basefrac = (basetime - basesample * input_stream->attoseconds_per_sample) / ((input_stream->attoseconds_per_sample + FRAC_ONE - 1) >> FRAC_BITS);
	assert(basefrac >= 0);
	assert(basefrac < FRAC_ONE);

	/* compute the stepping fraction */
	step = ((UINT64)input_stream->sample_rate << FRAC_BITS) / stream->sample_rate;

	/* if we have equal sample rates, we just need to copy */
	if (step == FRAC_ONE)
	{
		while (numsamples--)
		{
			/* compute the sample */
			sample = *source++;
			*dest++ = (sample * gain) >> 8;
		}
	}

	/* input is undersampled: use linear interpolation */
	else if (step < FRAC_ONE)
	{
		while (numsamples--)
		{
			int interp_frac = basefrac >> (FRAC_BITS - 12);

			/* compute the sample */
			sample = (source[0] * (0x1000 - interp_frac) + source[1] * interp_frac) >> 12;
			*dest++ = (sample * gain) >> 8;

			/* advance */
			basefrac += step;
			source += basefrac >> FRAC_BITS;
			basefrac &= FRAC_MASK;
		}
	}

	/* input is oversampled: sum the energy */
	else
	{
		/* use 8 bits to allow some extra headroom */
		int smallstep = step >> (FRAC_BITS - 8);

		while (numsamples--)
		{
			int remainder = smallstep;
			int tpos = 0;
			int scale;

			/* compute the sample */
			scale = (FRAC_ONE - basefrac) >> (FRAC_BITS - 8);
			sample = source[tpos++] * scale;
			remainder -= scale;
			while (remainder > 0x100)
			{
				sample += source[tpos++] * 0x100;
				remainder -= 0x100;
			}
			sample += source[tpos] * remainder;
			sample /= smallstep;

			*dest++ = (sample * gain) >> 8;

			/* advance */
			basefrac += step;
			source += basefrac >> FRAC_BITS;
			basefrac &= FRAC_MASK;
		}
	}

	return input->resample;
}
