/***************************************************************************

    sound.c

    Core sound functions and definitions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "config.h"
#include "sound/wavwrite.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE			(0)

#define VPRINTF(x)		do { if (VERBOSE) mame_printf_debug x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const attotime sound_manager::STREAMS_UPDATE_ATTOTIME = attotime::from_hz(STREAMS_UPDATE_FREQUENCY);



//**************************************************************************
//  INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  sound_stream - constructor
//-------------------------------------------------

sound_stream::sound_stream(device_t &device, int inputs, int outputs, int sample_rate, void *param, stream_update_func callback)
	: m_device(device),
	  m_next(NULL),
	  m_sample_rate(sample_rate),
	  m_new_sample_rate(0),
	  m_attoseconds_per_sample(0),
	  m_max_samples_per_update(0),
	  m_inputs(inputs),
	  m_input((inputs == 0) ? NULL : auto_alloc_array_clear(device.machine(), stream_input, inputs)),
	  m_input_array((inputs == 0) ? NULL : auto_alloc_array_clear(device.machine(), stream_sample_t *, inputs)),
	  m_resample_bufalloc(0),
	  m_outputs(outputs),
	  m_output((outputs == 0) ? NULL : auto_alloc_array_clear(device.machine(), stream_output, outputs)),
	  m_output_array((outputs == 0) ? NULL : auto_alloc_array_clear(device.machine(), stream_sample_t *, outputs)),
	  m_output_bufalloc(0),
	  m_output_sampindex(0),
	  m_output_update_sampindex(0),
	  m_output_base_sampindex(0),
	  m_callback(callback),
	  m_param(param)
{
	// get the device's sound interface
	device_sound_interface *sound;
	if (!device.interface(sound))
		throw emu_fatalerror("Attempted to create a sound_stream with a non-sound device");

	// this is also the implicit parameter if we are using our internal stub
	if (m_callback == &sound_stream::device_stream_update_stub)
		m_param = sound;

	// create a unique tag for saving
	astring state_tag;
	state_tag.printf("%d", m_device.machine().sound().m_stream_list.count());
	m_device.machine().save().save_item("stream", state_tag, 0, NAME(m_sample_rate));
	m_device.machine().save().register_postload(save_prepost_delegate(FUNC(sound_stream::postload), this));

	// save the gain of each input and output
	for (int inputnum = 0; inputnum < m_inputs; inputnum++)
		m_device.machine().save().save_item("stream", state_tag, inputnum, NAME(m_input[inputnum].m_gain));
	for (int outputnum = 0; outputnum < m_outputs; outputnum++)
	{
		m_output[outputnum].m_stream = this;
		m_device.machine().save().save_item("stream", state_tag, outputnum, NAME(m_output[outputnum].m_gain));
	}

	// force an update to the sample rates; this will cause everything to be recomputed
    // and will generate the initial resample buffers for our inputs
	recompute_sample_rate_data();

	// set up the initial output buffer positions now that we have data
	m_output_base_sampindex = -m_max_samples_per_update;
}


//-------------------------------------------------
//  sample_time - return the emulation time of the
//  next sample to be generated on the stream
//-------------------------------------------------

attotime sound_stream::sample_time() const
{
	return attotime(m_device.machine().sound().last_update().seconds, 0) + attotime(0, m_output_sampindex * m_attoseconds_per_sample);
}


//-------------------------------------------------
//  input_gain - return the input gain on a
//  given stream's input
//-------------------------------------------------

float sound_stream::input_gain(int inputnum) const
{
	assert(inputnum >= 0 && inputnum < m_inputs);
	return float(m_input[inputnum].m_gain) / 256.0f;
}


//-------------------------------------------------
//  initial_input_gain - return the original input
//  gain on a given stream's input
//-------------------------------------------------

float sound_stream::initial_input_gain(int inputnum) const
{
	assert(inputnum >= 0 && inputnum < m_inputs);
	return float(m_input[inputnum].m_initial_gain) / 256.0f;
}


//-------------------------------------------------
//  input_name - return the original input gain
//  on a given stream's input
//-------------------------------------------------

const char *sound_stream::input_name(int inputnum, astring &string) const
{
	// start with our device name and tag
	assert(inputnum >= 0 && inputnum < m_inputs);
	string.printf("%s '%s': ", m_device.name(), m_device.tag());

	// if we have a source, indicate where the sound comes from by device name and tag
	if (m_input[inputnum].m_source != NULL && m_input[inputnum].m_source->m_stream != NULL)
	{
		device_t &source = m_input[inputnum].m_source->m_stream->device();
		string.catprintf("%s '%s'", source.name(), source.tag());

		// get the sound interface; if there is more than 1 output we need to figure out which one
		device_sound_interface *sound;
		if (source.interface(sound) && sound->outputs() > 1)
		{
			// iterate over outputs until we find the stream that matches our source
			// then look for a match on the output number
			sound_stream *outstream;
			int streamoutputnum;
			for (int outputnum = 0; (outstream = sound->output_to_stream_output(outputnum, streamoutputnum)) != NULL; outputnum++)
				if (outstream == m_input[inputnum].m_source->m_stream && m_input[inputnum].m_source == &outstream->m_output[streamoutputnum])
				{
					string.catprintf(" Ch.%d", outputnum);
					break;
				}
		}
	}
	return string;
}


//-------------------------------------------------
//  output_gain - return the output gain on a
//  given stream's output
//-------------------------------------------------

float sound_stream::output_gain(int outputnum) const
{
	assert(outputnum >= 0 && outputnum < m_outputs);
	return float(m_output[outputnum].m_gain) / 256.0f;
}


//-------------------------------------------------
//  set_input - configure a stream's input
//-------------------------------------------------

void sound_stream::set_input(int index, sound_stream *input_stream, int output_index, float gain)
{
	VPRINTF(("stream_set_input(%p, '%s', %d, %p, %d, %f)\n", this, m_device.tag(), index, input_stream, output_index, gain));

	// make sure it's a valid input
	if (index >= m_inputs)
		fatalerror("Fatal error: stream_set_input attempted to configure non-existant input %d (%d max)", index, m_inputs);

	// make sure it's a valid output
	if (input_stream != NULL && output_index >= input_stream->m_outputs)
		fatalerror("Fatal error: stream_set_input attempted to use a non-existant output %d (%d max)", output_index, m_outputs);

	// if this input is already wired, update the dependent info
	stream_input &input = m_input[index];
	if (input.m_source != NULL)
		input.m_source->m_dependents--;

	// wire it up
	input.m_source = (input_stream != NULL) ? &input_stream->m_output[output_index] : NULL;
	input.m_gain = input.m_initial_gain = int(0x100 * gain);

	// update the dependent info
	if (input.m_source != NULL)
		input.m_source->m_dependents++;

	// update sample rates now that we know the input
	recompute_sample_rate_data();
}


//-------------------------------------------------
//  update - force a stream to update to
//  the current emulated time
//-------------------------------------------------

void sound_stream::update()
{
	// determine the number of samples since the start of this second
	attotime time = m_device.machine().time();
	INT32 update_sampindex = INT32(time.attoseconds / m_attoseconds_per_sample);

	// if we're ahead of the last update, then adjust upwards
	attotime last_update = m_device.machine().sound().last_update();
	if (time.seconds > last_update.seconds)
	{
		assert(time.seconds == last_update.seconds + 1);
		update_sampindex += m_sample_rate;
	}

	// if we're behind the last update, then adjust downwards
	if (time.seconds < last_update.seconds)
	{
		assert(time.seconds == last_update.seconds - 1);
		update_sampindex -= m_sample_rate;
	}

	// generate samples to get us up to the appropriate time
	g_profiler.start(PROFILER_SOUND);
	assert(m_output_sampindex - m_output_base_sampindex >= 0);
	assert(update_sampindex - m_output_base_sampindex <= m_output_bufalloc);
	generate_samples(update_sampindex - m_output_sampindex);
	g_profiler.stop();

	// remember this info for next time
	m_output_sampindex = update_sampindex;
}


//-------------------------------------------------
//  output_since_last_update - return a pointer to
//  the output buffer and the number of samples
//  since the last global update
//-------------------------------------------------

const stream_sample_t *sound_stream::output_since_last_update(int outputnum, int &numsamples)
{
	// force an update on the stream
	update();

	// compute the number of samples and a pointer to the output buffer
	numsamples = m_output_sampindex - m_output_update_sampindex;
	return m_output[outputnum].m_buffer + (m_output_update_sampindex - m_output_base_sampindex);
}


//-------------------------------------------------
//  set_sample_rate - set the sample rate on a
//  given stream
//-------------------------------------------------

void sound_stream::set_sample_rate(int new_rate)
{
	// we will update this on the next global update
	if (new_rate != sample_rate())
		m_new_sample_rate = new_rate;
}


//-------------------------------------------------
//  set_input_gain - set the input gain on a
//  given stream's input
//-------------------------------------------------

void sound_stream::set_input_gain(int inputnum, float gain)
{
	update();
	assert(inputnum >= 0 && inputnum < m_inputs);
	m_input[inputnum].m_gain = int(0x100 * gain);
}


//-------------------------------------------------
//  set_output_gain - set the output gain on a
//  given stream's output
//-------------------------------------------------

void sound_stream::set_output_gain(int outputnum, float gain)
{
	update();
	assert(outputnum >= 0 && outputnum < m_outputs);
	m_output[outputnum].m_gain = int(0x100 * gain);
}


//-------------------------------------------------
//  update_with_accounting - do a regular update,
//  but also do periodic accounting
//-------------------------------------------------

void sound_stream::update_with_accounting(bool second_tick)
{
	// do the normal update
	update();

	// if we've ticked over another second, adjust all the counters that are relative to
	// the current second
	INT32 output_bufindex = m_output_sampindex - m_output_base_sampindex;
	if (second_tick)
	{
		m_output_sampindex -= m_sample_rate;
		m_output_base_sampindex -= m_sample_rate;
	}

	// note our current output sample
	m_output_update_sampindex = m_output_sampindex;

	// if we don't have enough output buffer space to hold two updates' worth of samples,
	// we need to shuffle things down
	if (m_output_bufalloc - output_bufindex < 2 * m_max_samples_per_update)
	{
		INT32 samples_to_lose = output_bufindex - m_max_samples_per_update;
		if (samples_to_lose > 0)
		{
			// if we have samples to move, do so for each output
			if (output_bufindex > 0)
				for (int outputnum = 0; outputnum < m_outputs; outputnum++)
				{
					stream_output &output = m_output[outputnum];
					memmove(&output.m_buffer[0], &output.m_buffer[samples_to_lose], sizeof(output.m_buffer[0]) * (output_bufindex - samples_to_lose));
				}

			// update the base position
			m_output_base_sampindex += samples_to_lose;
		}
	}
}


//-------------------------------------------------
//  apply_sample_rate_changes - if there is a
//  pending sample rate change, apply it now
//-------------------------------------------------

void sound_stream::apply_sample_rate_changes()
{
	// skip if nothing to do
	if (m_new_sample_rate == 0)
		return;

	// update to the new rate and remember the old rate
	UINT32 old_rate = m_sample_rate;
	m_sample_rate = m_new_sample_rate;
	m_new_sample_rate = 0;

	// recompute all the data
	recompute_sample_rate_data();

	// reset our sample indexes to the current time
	m_output_sampindex = (INT64)m_output_sampindex * (INT64)m_sample_rate / old_rate;
	m_output_update_sampindex = (INT64)m_output_update_sampindex * (INT64)m_sample_rate / old_rate;
	m_output_base_sampindex = m_output_sampindex - m_max_samples_per_update;

	// clear out the buffer
	for (int outputnum = 0; outputnum < m_outputs; outputnum++)
		memset(m_output[outputnum].m_buffer, 0, m_max_samples_per_update * sizeof(m_output[outputnum].m_buffer[0]));
}


//-------------------------------------------------
//  device_stream_update_stub - stub callback for
//  passing through to modern devices
//-------------------------------------------------

STREAM_UPDATE( sound_stream::device_stream_update_stub )
{
	device_sound_interface *sound = reinterpret_cast<device_sound_interface *>(param);
	sound->sound_stream_update(*stream, inputs, outputs, samples);
}


//-------------------------------------------------
//  recompute_sample_rate_data - recompute sample
//  rate data, and all streams that are affected
//  by this stream
//-------------------------------------------------

void sound_stream::recompute_sample_rate_data()
{
	// recompute the timing parameters
	attoseconds_t update_attoseconds = m_device.machine().sound().update_attoseconds();
	m_attoseconds_per_sample = ATTOSECONDS_PER_SECOND / m_sample_rate;
	m_max_samples_per_update = (update_attoseconds + m_attoseconds_per_sample - 1) / m_attoseconds_per_sample;

	// update resample and output buffer sizes
	allocate_resample_buffers();
	allocate_output_buffers();

	// iterate over each input
	for (int inputnum = 0; inputnum < m_inputs; inputnum++)
	{
		// if we have a source, see if its sample rate changed
		stream_input &input = m_input[inputnum];
		if (input.m_source != NULL)
		{
			// okay, we have a new sample rate; recompute the latency to be the maximum
            // sample period between us and our input
			attoseconds_t new_attosecs_per_sample = ATTOSECONDS_PER_SECOND / input.m_source->m_stream->m_sample_rate;
			attoseconds_t latency = MAX(new_attosecs_per_sample, m_attoseconds_per_sample);

			// if the input stream's sample rate is lower, we will use linear interpolation
			// this requires an extra sample from the source
			if (input.m_source->m_stream->m_sample_rate < m_sample_rate)
				latency += new_attosecs_per_sample;

			// if our sample rates match exactly, we don't need any latency
			else if (input.m_source->m_stream->m_sample_rate == m_sample_rate)
				latency = 0;

			// we generally don't want to tweak the latency, so we just keep the greatest
            // one we've computed thus far
			input.m_latency_attoseconds = MAX(input.m_latency_attoseconds, latency);
			assert(input.m_latency_attoseconds < update_attoseconds);
		}
	}
}


//-------------------------------------------------
//  allocate_resample_buffers - recompute the
//  resample buffer sizes and expand if necessary
//-------------------------------------------------

void sound_stream::allocate_resample_buffers()
{
	// compute the target number of samples
	INT32 bufsize = 2 * m_max_samples_per_update;

	// if we don't have enough room, allocate more
	if (m_resample_bufalloc < bufsize)
	{
		// this becomes the new allocation size
		int oldsize = m_resample_bufalloc;
		m_resample_bufalloc = bufsize;

		// iterate over outputs and realloc their buffers
		for (int inputnum = 0; inputnum < m_inputs; inputnum++)
		{
			stream_input &input = m_input[inputnum];
			stream_sample_t *newbuffer = auto_alloc_array(m_device.machine(), stream_sample_t, m_resample_bufalloc);
			memcpy(newbuffer, input.m_resample, oldsize * sizeof(stream_sample_t));
			auto_free(m_device.machine(), input.m_resample);
			input.m_resample = newbuffer;
		}
	}
}


//-------------------------------------------------
//  allocate_output_buffers - recompute the
//  output buffer sizes and expand if necessary
//-------------------------------------------------

void sound_stream::allocate_output_buffers()
{
	// if we don't have enough room, allocate more
	INT32 bufsize = OUTPUT_BUFFER_UPDATES * m_max_samples_per_update;
	if (m_output_bufalloc < bufsize)
	{
		// this becomes the new allocation size
		int oldsize = m_output_bufalloc;
		m_output_bufalloc = bufsize;

		// iterate over outputs and realloc their buffers
		for (int outputnum = 0; outputnum < m_outputs; outputnum++)
		{
			stream_output &output = m_output[outputnum];
			stream_sample_t *newbuffer = auto_alloc_array(m_device.machine(), stream_sample_t, m_output_bufalloc);
			memcpy(newbuffer, output.m_buffer, oldsize * sizeof(stream_sample_t));
			memset(newbuffer + oldsize, 0, (m_output_bufalloc - oldsize) * sizeof(stream_sample_t));
			auto_free(m_device.machine(), output.m_buffer);
			output.m_buffer = newbuffer;
		}
	}
}


//-------------------------------------------------
//  postload - save/restore callback
//-------------------------------------------------

void sound_stream::postload()
{
	// recompute the same rate information
	recompute_sample_rate_data();

	// make sure our output buffers are fully cleared
	for (int outputnum = 0; outputnum < m_outputs; outputnum++)
		memset(m_output[outputnum].m_buffer, 0, m_output_bufalloc * sizeof(m_output[outputnum].m_buffer[0]));

	// recompute the sample indexes to make sense
	m_output_sampindex = m_device.machine().sound().last_update().attoseconds / m_attoseconds_per_sample;
	m_output_update_sampindex = m_output_sampindex;
	m_output_base_sampindex = m_output_sampindex - m_max_samples_per_update;
}


//-------------------------------------------------
//  generate_samples - generate the requested
//  number of samples for a stream, making sure
//  all inputs have the appropriate number of
//  samples generated
//-------------------------------------------------

void sound_stream::generate_samples(int samples)
{
	// if we're already there, skip it
	if (samples <= 0)
		return;

	VPRINTF(("generate_samples(%p, %d)\n", this, samples));

	// ensure all inputs are up to date and generate resampled data
	for (int inputnum = 0; inputnum < m_inputs; inputnum++)
	{
		// update the stream to the current time
		stream_input &input = m_input[inputnum];
		if (input.m_source != NULL)
			input.m_source->m_stream->update();

		// generate the resampled data
		m_input_array[inputnum] = generate_resampled_data(input, samples);
	}

	// loop over all outputs and compute the output pointer
	for (int outputnum = 0; outputnum < m_outputs; outputnum++)
	{
		stream_output &output = m_output[outputnum];
		m_output_array[outputnum] = output.m_buffer + (m_output_sampindex - m_output_base_sampindex);
	}

	// run the callback
	VPRINTF(("  callback(%p, %d)\n", this, samples));
	(*m_callback)(&m_device, this, m_param, m_input_array, m_output_array, samples);
	VPRINTF(("  callback done\n"));
}


//-------------------------------------------------
//  generate_resampled_data - generate the
//  resample buffer for a given input
//-------------------------------------------------

stream_sample_t *sound_stream::generate_resampled_data(stream_input &input, UINT32 numsamples)
{
	// if we don't have an output to pull data from, generate silence
	stream_sample_t *dest = input.m_resample;
	if (input.m_source == NULL)
	{
		memset(dest, 0, numsamples * sizeof(*dest));
		return input.m_resample;
	}

	// grab data from the output
	stream_output &output = *input.m_source;
	sound_stream &input_stream = *output.m_stream;
	int gain = (input.m_gain * output.m_gain) >> 8;

	// determine the time at which the current sample begins, accounting for the
    // latency we calculated between the input and output streams
	attoseconds_t basetime = m_output_sampindex * m_attoseconds_per_sample - input.m_latency_attoseconds;

	// now convert that time into a sample in the input stream
	INT32 basesample;
	if (basetime >= 0)
		basesample = basetime / input_stream.m_attoseconds_per_sample;
	else
		basesample = -(-basetime / input_stream.m_attoseconds_per_sample) - 1;

	// compute a source pointer to the first sample
	assert(basesample >= input_stream.m_output_base_sampindex);
	stream_sample_t *source = output.m_buffer + (basesample - input_stream.m_output_base_sampindex);

	// determine the current fraction of a sample
	UINT32 basefrac = (basetime - basesample * input_stream.m_attoseconds_per_sample) / ((input_stream.m_attoseconds_per_sample + FRAC_ONE - 1) >> FRAC_BITS);
	assert(basefrac >= 0);
	assert(basefrac < FRAC_ONE);

	// compute the stepping fraction
	UINT32 step = ((UINT64)input_stream.m_sample_rate << FRAC_BITS) / m_sample_rate;

	// if we have equal sample rates, we just need to copy
	if (step == FRAC_ONE)
	{
		while (numsamples--)
		{
			// compute the sample
			stream_sample_t sample = *source++;
			*dest++ = (sample * gain) >> 8;
		}
	}

	// input is undersampled: point sample except where our sample period covers a boundary
	else if (step < FRAC_ONE)
	{
		while (numsamples != 0)
		{
			// fill in with point samples until we hit a boundary
			int nextfrac;
			while ((nextfrac = basefrac + step) < FRAC_ONE && numsamples--)
			{
				*dest++ = (source[0] * gain) >> 8;
				basefrac = nextfrac;
			}

			// if we're done, we're done
			if ((INT32)numsamples-- < 0)
				break;

			// compute starting and ending fractional positions
			int startfrac = basefrac >> (FRAC_BITS - 12);
			int endfrac = nextfrac >> (FRAC_BITS - 12);

			// blend between the two samples accordingly
			stream_sample_t sample = (source[0] * (0x1000 - startfrac) + source[1] * (endfrac - 0x1000)) / (endfrac - startfrac);
			*dest++ = (sample * gain) >> 8;

			// advance
			basefrac = nextfrac & FRAC_MASK;
			source++;
		}
	}

	// input is oversampled: sum the energy
	else
	{
		// use 8 bits to allow some extra headroom
		int smallstep = step >> (FRAC_BITS - 8);
		while (numsamples--)
		{
			int remainder = smallstep;
			int tpos = 0;

			// compute the sample
			int scale = (FRAC_ONE - basefrac) >> (FRAC_BITS - 8);
			stream_sample_t sample = source[tpos++] * scale;
			remainder -= scale;
			while (remainder > 0x100)
			{
				sample += source[tpos++] * 0x100;
				remainder -= 0x100;
			}
			sample += source[tpos] * remainder;
			sample /= smallstep;

			*dest++ = (sample * gain) >> 8;

			// advance
			basefrac += step;
			source += basefrac >> FRAC_BITS;
			basefrac &= FRAC_MASK;
		}
	}

	return input.m_resample;
}



//**************************************************************************
//  STREAM INPUT
//**************************************************************************

//-------------------------------------------------
//  stream_input - constructor
//-------------------------------------------------

sound_stream::stream_input::stream_input()
	: m_source(NULL),
	  m_resample(NULL),
	  m_bufsize(0),
	  m_bufalloc(0),
	  m_latency_attoseconds(0),
	  m_gain(0x100),
	  m_initial_gain(0x100)
{
}



//**************************************************************************
//  STREAM OUTPUT
//**************************************************************************

//-------------------------------------------------
//  stream_output - constructor
//-------------------------------------------------

sound_stream::stream_output::stream_output()
	: m_buffer(NULL),
	  m_dependents(0),
	  m_gain(0x100)
{
}



//**************************************************************************
//  SOUND MANAGER
//**************************************************************************

//-------------------------------------------------
//  sound_manager - constructor
//-------------------------------------------------

sound_manager::sound_manager(running_machine &machine)
	: m_machine(machine),
	  m_update_timer(machine.scheduler().timer_alloc(FUNC(update_static), this)),
	  m_finalmix_leftover(0),
	  m_finalmix(NULL),
	  m_leftmix(NULL),
	  m_rightmix(NULL),
	  m_muted(0),
	  m_attenuation(0),
	  m_nosound_mode(!machine.options().sound()),
	  m_wavfile(NULL),
	  m_stream_list(machine.respool()),
	  m_update_attoseconds(STREAMS_UPDATE_ATTOTIME.attoseconds),
	  m_last_update(attotime::zero)
{
	// get filename for WAV file or AVI file if specified
	const char *wavfile = machine.options().wav_write();
	const char *avifile = machine.options().avi_write();

	// handle -nosound and lower sample rate if not recording WAV or AVI
	if (m_nosound_mode && wavfile[0] == 0 && avifile[0] == 0)
		machine.m_sample_rate = 11025;

	// count the mixers
#if VERBOSE
	mixer_interface_iterator iter(machine.root_device());
	VPRINTF(("total mixers = %d\n", iter.count()));
#endif

	// allocate memory for mix buffers
	m_leftmix = auto_alloc_array(machine, INT32, machine.sample_rate());
	m_rightmix = auto_alloc_array(machine, INT32, machine.sample_rate());
	m_finalmix = auto_alloc_array(machine, INT16, machine.sample_rate());

	// open the output WAV file if specified
	if (wavfile[0] != 0)
		m_wavfile = wav_open(wavfile, machine.sample_rate(), 2);

	// register callbacks
	config_register(machine, "mixer", config_saveload_delegate(FUNC(sound_manager::config_load), this), config_saveload_delegate(FUNC(sound_manager::config_save), this));
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(FUNC(sound_manager::pause), this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(FUNC(sound_manager::resume), this));
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(sound_manager::reset), this));

	// register global states
	state_save_register_global(machine, m_last_update);

	// set the starting attenuation
	set_attenuation(machine.options().volume());

	// start the periodic update flushing timer
	m_update_timer->adjust(STREAMS_UPDATE_ATTOTIME, 0, STREAMS_UPDATE_ATTOTIME);
}


//-------------------------------------------------
//  sound_manager - destructor
//-------------------------------------------------

sound_manager::~sound_manager()
{
	// close any open WAV file
	if (m_wavfile != NULL)
		wav_close(m_wavfile);
	m_wavfile = NULL;
}


//-------------------------------------------------
//  stream_alloc - allocate a new stream
//-------------------------------------------------

sound_stream *sound_manager::stream_alloc(device_t &device, int inputs, int outputs, int sample_rate, void *param, sound_stream::stream_update_func callback)
{
	if (callback != NULL)
		return &m_stream_list.append(*auto_alloc(device.machine(), sound_stream(device, inputs, outputs, sample_rate, param, callback)));
	else
		return &m_stream_list.append(*auto_alloc(device.machine(), sound_stream(device, inputs, outputs, sample_rate)));
}


//-------------------------------------------------
//  set_attenuation - set the global volume
//-------------------------------------------------

void sound_manager::set_attenuation(int attenuation)
{
	m_attenuation = attenuation;
	machine().osd().set_mastervolume(m_muted ? -32 : m_attenuation);
}


//-------------------------------------------------
//  indexed_mixer_input - return the mixer
//  device and input index of the global mixer
//  input
//-------------------------------------------------

bool sound_manager::indexed_mixer_input(int index, mixer_input &info) const
{
	// scan through the mixers until we find the indexed input
	mixer_interface_iterator iter(machine().root_device());
	for (info.mixer = iter.first(); info.mixer != NULL; info.mixer = iter.next())
	{
		if (index < info.mixer->inputs())
		{
			info.stream = info.mixer->input_to_stream_input(index, info.inputnum);
			assert(info.stream != NULL);
			return true;
		}
		index -= info.mixer->inputs();
	}

	// didn't locate
	return false;
}


//-------------------------------------------------
//  mute - mute sound output
//-------------------------------------------------

void sound_manager::mute(bool mute, UINT8 reason)
{
	if (mute)
		m_muted |= reason;
	else
		m_muted &= ~reason;
	set_attenuation(m_attenuation);
}


//-------------------------------------------------
//  reset - reset all sound chips
//-------------------------------------------------

void sound_manager::reset()
{
	// reset all the sound chips
	sound_interface_iterator iter(machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
		sound->device().reset();
}


//-------------------------------------------------
//  pause - pause sound output
//-------------------------------------------------

void sound_manager::pause()
{
	mute(true, MUTE_REASON_PAUSE);
}


//-------------------------------------------------
//  resume - resume sound output
//-------------------------------------------------

void sound_manager::resume()
{
	mute(false, MUTE_REASON_PAUSE);
}


//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void sound_manager::config_load(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// might not have any data
	if (parentnode == NULL)
		return;

	// iterate over channel nodes
	for (xml_data_node *channelnode = xml_get_sibling(parentnode->child, "channel"); channelnode != NULL; channelnode = xml_get_sibling(channelnode->next, "channel"))
	{
		mixer_input info;
		if (indexed_mixer_input(xml_get_attribute_int(channelnode, "index", -1), info))
		{
			float defvol = xml_get_attribute_float(channelnode, "defvol", -1000.0);
			float newvol = xml_get_attribute_float(channelnode, "newvol", -1000.0);
			if (fabs(defvol - info.stream->initial_input_gain(info.inputnum)) < 1e-6 && newvol != -1000.0)
				info.stream->set_input_gain(info.inputnum, newvol);
		}
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void sound_manager::config_save(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// iterate over mixer channels
	if (parentnode != NULL)
		for (int mixernum = 0; ; mixernum++)
		{
			mixer_input info;
			if (!indexed_mixer_input(mixernum, info))
				break;
			float defvol = info.stream->initial_input_gain(info.inputnum);
			float newvol = info.stream->input_gain(info.inputnum);

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


//-------------------------------------------------
//  update - mix everything down to its final form
//  and send it to the OSD layer
//-------------------------------------------------

void sound_manager::update()
{
	VPRINTF(("sound_update\n"));

	g_profiler.start(PROFILER_SOUND);

	// force all the speaker streams to generate the proper number of samples
	int samples_this_update = 0;
	speaker_device_iterator iter(machine().root_device());
	for (speaker_device *speaker = iter.first(); speaker != NULL; speaker = iter.next())
		speaker->mix(m_leftmix, m_rightmix, samples_this_update, (m_muted & MUTE_REASON_SYSTEM));

	// now downmix the final result
	UINT32 finalmix_step = machine().video().speed_factor();
	UINT32 finalmix_offset = 0;
	INT16 *finalmix = m_finalmix;
	int sample;
	for (sample = m_finalmix_leftover; sample < samples_this_update * 1000; sample += finalmix_step)
	{
		int sampindex = sample / 1000;

		// clamp the left side
		INT32 samp = m_leftmix[sampindex];
		if (samp < -32768)
			samp = -32768;
		else if (samp > 32767)
			samp = 32767;
		finalmix[finalmix_offset++] = samp;

		// clamp the right side
		samp = m_rightmix[sampindex];
		if (samp < -32768)
			samp = -32768;
		else if (samp > 32767)
			samp = 32767;
		finalmix[finalmix_offset++] = samp;
	}
	m_finalmix_leftover = sample - samples_this_update * 1000;

	// play the result
	if (finalmix_offset > 0)
	{
		if (!m_nosound_mode)
			machine().osd().update_audio_stream(finalmix, finalmix_offset / 2);
		machine().video().add_sound_to_recording(finalmix, finalmix_offset / 2);
		if (m_wavfile != NULL)
			wav_add_data_16(m_wavfile, finalmix, finalmix_offset);
	}

	// see if we ticked over to the next second
	attotime curtime = machine().time();
	bool second_tick = false;
	if (curtime.seconds != m_last_update.seconds)
	{
		assert(curtime.seconds == m_last_update.seconds + 1);
		second_tick = true;
	}

	// iterate over all the streams and update them
	for (sound_stream *stream = m_stream_list.first(); stream != NULL; stream = stream->next())
		stream->update_with_accounting(second_tick);

	// remember the update time
	m_last_update = curtime;

	// update sample rates if they have changed
	for (sound_stream *stream = m_stream_list.first(); stream != NULL; stream = stream->next())
		stream->apply_sample_rate_changes();

	g_profiler.stop();
}
