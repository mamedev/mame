// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    sound.cpp

    Core sound functions and definitions.

***************************************************************************/

#include "emu.h"
#include "speaker.h"
#include "emuopts.h"
#include "osdepend.h"
#include "config.h"
#include "wavwrite.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE         (0)

#define VPRINTF(x)      do { if (VERBOSE) osd_printf_debug x; } while (0)

#define LOG_OUTPUT_WAV  (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const attotime sound_manager::STREAMS_UPDATE_ATTOTIME = attotime::from_hz(STREAMS_UPDATE_FREQUENCY);



//**************************************************************************
//  STREAM BUFFER
//**************************************************************************

//-------------------------------------------------
//  stream_buffer - constructor
//-------------------------------------------------

stream_buffer::stream_buffer(u32 sample_rate) :
	m_end_second(0),
	m_end_sample(0),
	m_sample_rate(sample_rate),
	m_sample_attos((sample_rate == 0) ? ATTOSECONDS_PER_SECOND : ((ATTOSECONDS_PER_SECOND + sample_rate - 1) / sample_rate)),
	m_buffer(sample_rate)
{
}


//-------------------------------------------------
//  stream_buffer - destructor
//-------------------------------------------------

stream_buffer::~stream_buffer()
{
#if (SOUND_DEBUG)
	if (m_wav_file != nullptr)
	{
		flush_wav();
		close_wav();
	}
#endif
}


//-------------------------------------------------
//  set_sample_rate - set a new sample rate for
//  this buffer
//-------------------------------------------------

void stream_buffer::set_sample_rate(u32 rate)
{
	sound_assert(rate > 0);

	// skip if nothing is actually changing
	if (rate == m_sample_rate)
		return;

	// buffer a short runway of previous samples; in order to support smooth
	// sample rate changes (needed by, e.g., Q*Bert's Votrax), we buffer a few
	// samples at the previous rate, and then reconstitute them resampled
	// (via simple point sampling) at the new rate. The litmus test is the
	// voice when jumping off the edge in Q*Bert; without this extra effort
	// it is crackly and/or glitchy at times
	sample_t buffer[256];
	for (int index = 0; index < ARRAY_LENGTH(buffer); index++)
		buffer[index] = m_buffer[clamp_index(m_end_sample - 1 - index)];

	// also note the time and rate of these samples (end_time is AFTER the final sample)
	attotime buffer_period = sample_period();
	attotime buffer_time = end_time();

	// ensure our buffer is large enough to hold a full second at the new rate
	if (m_buffer.size() < rate)
		m_buffer.resize(rate);

	// set the new rate
	m_sample_rate = rate;
	m_sample_attos = (rate == 0) ? ATTOSECONDS_PER_SECOND : ((ATTOSECONDS_PER_SECOND + rate - 1) / rate);

#if (SOUND_DEBUG)
	// for aggressive debugging, fill the buffer with NANs to catch anyone
	// reading beyond what we resample below
	std::fill_n(&m_buffer[0], m_buffer.size(), NAN);
#endif

	// compute the new end sample index based on the buffer time
	m_end_sample = time_to_buffer_index(buffer_time);

	// compute the time of the first sample to be backfilled; start one period before
	attotime backfill_period = sample_period();
	attotime backfill_time = end_time() - backfill_period;

	// also adjust the buffered sample end time to point to the sample time of the
	// final sample captured
	buffer_time -= buffer_period;

	// loop until we run out of buffered data
	for (int srcindex = 0, dstindex = 0; ; dstindex++)
	{
		// if our backfill time is before the current buffered sample time,
		// back up until we have a sample that covers this time
		while (backfill_time < buffer_time && srcindex < ARRAY_LENGTH(buffer))
		{
			buffer_time -= buffer_period;
			srcindex++;
		}

		// stop when we run out of source
		if (srcindex >= ARRAY_LENGTH(buffer))
			break;

		// write this sample, and back up to the next sample time
		put(m_end_sample - 1 - dstindex, buffer[srcindex]);
		backfill_time -= backfill_period;
	}
}


//-------------------------------------------------
//  index_time - return the attotime of a given
//  index within the buffer
//-------------------------------------------------

attotime stream_buffer::index_time(s32 index) const
{
	index = clamp_index(index);
	return attotime(m_end_second - ((index > m_end_sample) ? 1 : 0), index * m_sample_attos);
}


//-------------------------------------------------
//  time_to_buffer_index - given an attotime,
//  return the buffer index corresponding to it
//-------------------------------------------------

u32 stream_buffer::time_to_buffer_index(attotime time, bool round_up)
{
	// compute the sample index within the second
	int sample = (time.attoseconds() + (round_up ? (m_sample_attos - 1) : 0)) / m_sample_attos;
	sound_assert(sample >= 0 && sample <= m_sample_rate);

	// if the time is past the current end, make it the end
	if (time.seconds() > m_end_second || (time.seconds() == m_end_second && sample > m_end_sample))
	{
		m_end_sample = sample;
		m_end_second = time.m_seconds;

		// due to round_up, we could tweak over the line into the next second
		if (sample >= m_sample_rate)
		{
			m_end_sample -= m_sample_rate;
			m_end_second++;
		}
	}

	// if the time is before the start, fail
	if (time.seconds() + 1 < m_end_second || (time.seconds() + 1 == m_end_second && sample < m_end_sample))
		throw emu_fatalerror("Attempt to create an out-of-bounds view");

	return clamp_index(sample);
}


#if (SOUND_DEBUG)

//-------------------------------------------------
//  open_wav - open a WAV file for logging purposes
//-------------------------------------------------

void stream_buffer::open_wav(char const *filename)
{
	// always open at 48k so that sound programs can handle it
	// re-sample as needed
	m_wav_file = wav_open(filename, 48000, 1);
}


//-------------------------------------------------
//  flush_wav - flush data to the WAV file
//-------------------------------------------------

void stream_buffer::flush_wav()
{
	// skip if no file
	if (m_wav_file == nullptr)
		return;

	// grab a view of the data from the last-written point
	read_stream_view view(this, m_last_written, m_end_sample, 1.0f);
	m_last_written = m_end_sample;

	// iterate over chunks for conversion
	s16 buffer[1024];
	for (int samplebase = 0; samplebase < view.samples(); samplebase += ARRAY_LENGTH(buffer))
	{
		// clamp to the buffer size
		int cursamples = view.samples() - samplebase;
		if (cursamples > ARRAY_LENGTH(buffer))
			cursamples = ARRAY_LENGTH(buffer);

		// convert and fill
		for (int sampindex = 0; sampindex < cursamples; sampindex++)
			buffer[sampindex] = s16(view.get(samplebase + sampindex) * 32768.0);

		// write to the WAV
		wav_add_data_16(m_wav_file, buffer, cursamples);
	}
}


//-------------------------------------------------
//  close_wav - close the logging WAV file
//-------------------------------------------------

void stream_buffer::close_wav()
{
	if (m_wav_file != nullptr)
		wav_close(m_wav_file);
	m_wav_file = nullptr;
}

#endif



//**************************************************************************
//  SOUND STREAM
//**************************************************************************

//-------------------------------------------------
//  sound_stream - constructor with old-style
//  callback
//-------------------------------------------------

sound_stream::sound_stream(device_t &device, u32 inputs, u32 outputs, u32 output_base, u32 sample_rate, stream_update_delegate callback, sound_stream_flags flags) :
	m_device(device),
	m_next(nullptr),
	m_sample_rate((sample_rate < SAMPLE_RATE_OUTPUT_ADAPTIVE) ? sample_rate : 48000),
	m_pending_sample_rate(SAMPLE_RATE_INVALID),
	m_last_sample_rate_update(0),
	m_input_adaptive(sample_rate == SAMPLE_RATE_INPUT_ADAPTIVE),
	m_output_adaptive(sample_rate == SAMPLE_RATE_OUTPUT_ADAPTIVE),
	m_synchronous((flags & STREAM_SYNCHRONOUS) != 0),
	m_sync_timer(nullptr),
	m_input(inputs),
	m_input_array(inputs),
	m_input_view(inputs),
	m_output_base(output_base),
	m_output(outputs),
	m_output_array(outputs),
	m_output_view(outputs),
	m_callback(std::move(callback)),
	m_callback_ex(stream_update_ex_delegate(&sound_stream::oldstyle_callback_ex, this))
{
	// common init
	init_common(inputs, outputs, sample_rate, flags);
}


//-------------------------------------------------
//  sound_stream - constructor with new-style
//  callback
//-------------------------------------------------

sound_stream::sound_stream(device_t &device, u32 inputs, u32 outputs, u32 output_base, u32 sample_rate, stream_update_ex_delegate callback, sound_stream_flags flags) :
	m_device(device),
	m_next(nullptr),
	m_sample_rate((sample_rate < SAMPLE_RATE_OUTPUT_ADAPTIVE) ? sample_rate : 48000),
	m_pending_sample_rate(SAMPLE_RATE_INVALID),
	m_input_adaptive(sample_rate == SAMPLE_RATE_INPUT_ADAPTIVE),
	m_output_adaptive(sample_rate == SAMPLE_RATE_OUTPUT_ADAPTIVE),
	m_synchronous((flags & STREAM_SYNCHRONOUS) != 0),
	m_sync_timer(nullptr),
	m_input(inputs),
	m_input_array(inputs),
	m_input_view(inputs),
	m_output_base(output_base),
	m_output(outputs),
	m_output_array(outputs),
	m_output_view(outputs),
	m_callback_ex(std::move(callback))
{
	// common init
	init_common(inputs, outputs, sample_rate, flags);
}


//-------------------------------------------------
//  ~sound_stream - destructor
//-------------------------------------------------

sound_stream::~sound_stream()
{
}


//-------------------------------------------------
//  init_common - constructor
//-------------------------------------------------

void sound_stream::init_common(u32 inputs, u32 outputs, u32 sample_rate, sound_stream_flags flags)
{
	sound_assert(outputs > 0);

	// create a name
	m_name = m_device.name();
	m_name += " '";
	m_name += m_device.tag();
	m_name += "'";

	// create a unique tag for saving
	std::string state_tag = string_format("%d", m_device.machine().sound().unique_id());
	auto &save = m_device.machine().save();
	save.register_postload(save_prepost_delegate(FUNC(sound_stream::postload), this));

	// initialize all inputs
	for (unsigned int inputnum = 0; inputnum < m_input.size(); inputnum++)
	{
		// allocate a resampler stream if needed, and get a pointer to its output
		sound_stream_output *resampler = nullptr;
		int resampler_type = flags & STREAM_RESAMPLER_MASK;
		if (resampler_type != STREAM_RESAMPLER_NONE)
		{
			switch (resampler_type)
			{
				default:
				case STREAM_RESAMPLER_DEFAULT:
					m_resampler_list.push_back(std::make_unique<default_resampler_stream>(m_device));
					break;
			}
			resampler = &m_resampler_list.back()->m_output[0];
		}

		// add the new input
		m_input[inputnum].init(*this, inputnum, state_tag.c_str(), resampler);
	}

	// initialize all outputs
	for (unsigned int outputnum = 0; outputnum < m_output.size(); outputnum++)
		m_output[outputnum].init(*this, outputnum, state_tag.c_str());

	// create an update timer for synchronous streams
	if (synchronous())
		m_sync_timer = m_device.machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sound_stream::sync_update), this));

	// force an update to the sample rates
	sample_rate_changed();
}


//-------------------------------------------------
//  set_input - configure a stream's input
//-------------------------------------------------

void sound_stream::set_input(int index, sound_stream *input_stream, int output_index, float gain)
{
	VPRINTF(("stream_set_input(%p, '%s', %d, %p, %d, %f)\n", (void *)this, m_device.tag(),
			index, (void *)input_stream, output_index, (double) gain));

	// make sure it's a valid input
	if (index >= m_input.size())
		fatalerror("stream_set_input attempted to configure nonexistent input %d (%d max)\n", index, int(m_input.size()));

	// make sure it's a valid output
	if (input_stream != nullptr && output_index >= input_stream->m_output.size())
		fatalerror("stream_set_input attempted to use a nonexistent output %d (%d max)\n", output_index, int(m_output.size()));

	// wire it up
	m_input[index].set_source((input_stream != nullptr) ? &input_stream->m_output[output_index] : nullptr);
	m_input[index].set_gain(gain);

	// if our input is output-adaptive, add us to the list of dependents
	if (input_stream != nullptr && input_stream->output_adaptive())
		input_stream->m_dependents.push_back(this);

	// update sample rates now that we know the input
	sample_rate_changed();
}


//-------------------------------------------------
//  update - force a stream to update to
//  the current emulated time
//-------------------------------------------------

void sound_stream::update()
{
	attotime start = m_output[0].end_time();
	attotime end = m_device.machine().time();

	// ignore any update requests if we're already up to date
	if (start >= end)
		return;
	update_view(start, end);
}


//-------------------------------------------------
//  update_view - force a stream to update to
//  the current emulated time and return a view
//  to the generated samples from the given
//  output number
//-------------------------------------------------

read_stream_view sound_stream::update_view(attotime start, attotime end, u32 outputnum)
{
	sound_assert(start <= end);
	sound_assert(outputnum < m_output.size());

	// clean up parameters for when the asserts go away
	if (outputnum >= m_output.size())
		outputnum = 0;
	if (start > end)
		start = end;

	g_profiler.start(PROFILER_SOUND);

	// reposition our start to coincide with the current buffer end
	attotime update_start = m_output[outputnum].end_time();
	if (update_start <= end)
	{
		// create views for all the outputs
		for (unsigned int outindex = 0; outindex < m_output.size(); outindex++)
			m_output_view[outindex] = m_output[outindex].view(update_start, end);

		// skip if nothing to do
		u32 samples = m_output_view[0].samples();
		sound_assert(samples >= 0);
		if (samples != 0)
		{
			sound_assert(!synchronous() || samples == 1);

			// ensure all input streams are up to date, and create views for them as well
			for (unsigned int inputnum = 0; inputnum < m_input.size(); inputnum++)
				if (m_input[inputnum].valid())
					m_input_view[inputnum] = m_input[inputnum].update(update_start, end);

#if (SOUND_DEBUG)
			// clear each output view to NANs before we call the callback
			for (unsigned int outindex = 0; outindex < m_output.size(); outindex++)
				m_output_view[outindex].fill(NAN);
#endif

			// if we have an extended callback, that's all we need
			m_callback_ex(*this, m_input_view, m_output_view);

#if (SOUND_DEBUG)
			// make sure everything was overwritten
			for (unsigned int outindex = 0; outindex < m_output.size(); outindex++)
				for (int sampindex = 0; sampindex < m_output_view[outindex].samples(); sampindex++)
					m_output_view[outindex].get(sampindex);

			for (unsigned int outindex = 0; outindex < m_output.size(); outindex++)
				m_output[outindex].m_buffer.flush_wav();
#endif
		}
		g_profiler.stop();
	}

	// return the requested view
	return read_stream_view(m_output[outputnum].view(start, end));
}


//-------------------------------------------------
//  sync_update - timer callback to handle a
//  synchronous stream
//-------------------------------------------------

void sound_stream::sync_update(void *, s32)
{
	update();
	reprime_sync_timer();
}


//-------------------------------------------------
//  set_sample_rate - set the sample rate on a
//  given stream
//-------------------------------------------------

void sound_stream::set_sample_rate(u32 new_rate)
{
	// we will update this on the next global update
	if (new_rate != sample_rate())
		m_pending_sample_rate = new_rate;
}


//-------------------------------------------------
//  apply_sample_rate_changes - if there is a
//  pending sample rate change, apply it now
//-------------------------------------------------

void sound_stream::apply_sample_rate_changes(u32 updatenum, u32 downstream_rate)
{
	// grab the new rate and invalidate
	u32 new_rate = (m_pending_sample_rate != SAMPLE_RATE_INVALID) ? m_pending_sample_rate : m_sample_rate;
	m_pending_sample_rate = SAMPLE_RATE_INVALID;

	// if we're input adaptive, override with the rate of our input
	if (input_adaptive() && m_input.size() > 0 && m_input[0].valid())
		new_rate = m_input[0].source().stream().sample_rate();

	// if we're output adaptive, override with the rate of our output
	if (output_adaptive())
	{
		if (m_last_sample_rate_update == updatenum)
			sound_assert(new_rate == m_sample_rate);
		else
			m_last_sample_rate_update = updatenum;
		new_rate = downstream_rate;
	}

	// if something is different, process the change
	if (new_rate != SAMPLE_RATE_INVALID && new_rate != m_sample_rate)
	{
		// update to the new rate and notify everyone
#if (SOUND_DEBUG)
		printf("stream %s changing rates %d -> %d\n", name(), m_sample_rate, new_rate);
#endif
		m_sample_rate = new_rate;
		sample_rate_changed();
	}

	// now call through our inputs and apply the rate change there
	for (auto &input : m_input)
		if (input.valid())
			input.apply_sample_rate_changes(updatenum, m_sample_rate);
}


//-------------------------------------------------
//  sample_rate_changed - recompute sample
//  rate data, and all streams that are affected
//  by this stream
//-------------------------------------------------

void sound_stream::sample_rate_changed()
{
	// if invalid, just punt
	if (m_sample_rate == SAMPLE_RATE_INVALID)
		return;

	// update all output buffers
	for (auto &output : m_output)
		output.sample_rate_changed(m_sample_rate);

	// if synchronous, prime the timer
	if (synchronous())
		reprime_sync_timer();
}


//-------------------------------------------------
//  reprime_sync_timer - set up the next sync
//  timer to go off just a hair after the end of
//  the current sample period
//-------------------------------------------------

void sound_stream::reprime_sync_timer()
{
	attotime curtime = m_device.machine().time();
	attotime target = m_output[0].end_time() + attotime(0, 1);
	m_sync_timer->adjust(target - curtime);
}


//-------------------------------------------------
//  postload - save/restore callback
//-------------------------------------------------

void sound_stream::postload()
{
	// recompute the same rate information
	sample_rate_changed();
}


//-------------------------------------------------
//  oldstyle_callback_ex - extended callback which
//  resamples and then forward on to the old-style
//  traditional callback
//-------------------------------------------------

void sound_stream::oldstyle_callback_ex(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	// temporary buffer to hold stream_sample_t inputs and outputs
	stream_sample_t temp_buffer[1024];
	int chunksize = ARRAY_LENGTH(temp_buffer) / (inputs.size() + outputs.size());
	int chunknum = 0;

	// create the arrays to pass to the callback
	stream_sample_t **inputptr = m_input.empty() ? nullptr : &m_input_array[0];
	stream_sample_t **outputptr = &m_output_array[0];
	for (unsigned int inputnum = 0; inputnum < inputs.size(); inputnum++)
		inputptr[inputnum] = &temp_buffer[chunksize * chunknum++];
	for (unsigned int outputnum = 0; outputnum < m_output.size(); outputnum++)
		outputptr[outputnum] = &temp_buffer[chunksize * chunknum++];

	// loop until all chunks done
	for (int baseindex = 0; baseindex < outputs[0].samples(); baseindex += chunksize)
	{
		// determine the number of samples to process this time
		int cursamples = outputs[0].samples() - baseindex;
		if (cursamples > chunksize)
			cursamples = chunksize;

		// copy in the input data
		for (unsigned int inputnum = 0; inputnum < inputs.size(); inputnum++)
		{
			stream_sample_t *dest = inputptr[inputnum];
			for (int index = 0; index < cursamples; index++)
				dest[index] = stream_sample_t(inputs[inputnum].get(baseindex + index) * stream_buffer::sample_t(32768.0));
		}

		// run the callback
		m_callback(*this, inputptr, outputptr, cursamples);

		// copy out the output data
		for (unsigned int outputnum = 0; outputnum < m_output.size(); outputnum++)
		{
			stream_sample_t *src = outputptr[outputnum];
			for (int index = 0; index < cursamples; index++)
				outputs[outputnum].put(baseindex + index, stream_buffer::sample_t(src[index]) * stream_buffer::sample_t(1.0 / 32768.0));
		}
	}
}


//-------------------------------------------------
//  print_graph_recursive - helper for debugging;
//  prints info on this stream and then recursively
//  prints info on all inputs
//-------------------------------------------------

#if (SOUND_DEBUG)
void sound_stream::print_graph_recursive(int indent)
{
	printf("%c %*s%s @ %d\n", m_callback.isnull() ? ' ' : '!', indent, "", name(), sample_rate());
	for (int index = 0; index < m_input.size(); index++)
		if (m_input[index].valid())
			if (m_input[index].m_resampler_source != nullptr)
				m_input[index].m_resampler_source->stream().print_graph_recursive(indent + 2);
			else
				m_input[index].m_native_source->stream().print_graph_recursive(indent + 2);
}
#endif



//**************************************************************************
//  RESAMPLER STREAM
//**************************************************************************

//-------------------------------------------------
//  default_resampler_stream - derived sound_stream
//  class that handles resampling
//-------------------------------------------------

default_resampler_stream::default_resampler_stream(device_t &device) :
	sound_stream(device, 1, 1, 0, SAMPLE_RATE_OUTPUT_ADAPTIVE, stream_update_ex_delegate(&default_resampler_stream::resampler_sound_update, this), STREAM_RESAMPLER_NONE),
	m_max_latency(0)
{
	// create a name
	m_name = "Default Resampler '";
	m_name += device.tag();
	m_name += "'";
}


//-------------------------------------------------
//  resampler_sound_update - stream callback
//  handler for resampling an input stream to the
//  target sample rate of the output
//-------------------------------------------------

void default_resampler_stream::resampler_sound_update(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	sound_assert(inputs.size() == 1);
	sound_assert(outputs.size() == 1);

	auto &input = inputs[0];
	auto &output = outputs[0];

	// if we have equal sample rates, we just need to copy
	auto numsamples = output.samples();
	if (input.sample_rate() == output.sample_rate())
	{
		output.copy(input);
		return;
	}

	// compute the stepping value and the inverse
	float step = float(input.sample_rate()) / float(output.sample_rate());
	float stepinv = 1.0f / step;

	// determine the latency we need to introduce, in input samples:
	//    1 input sample for undersampled inputs
	//    1 + step input samples for oversampled inputs
	s64 latency_samples = 1 + ((step < 1.0f) ? 0 : s32(step));
	if (latency_samples <= m_max_latency)
		latency_samples = m_max_latency;
	else
		m_max_latency = latency_samples;
	attotime latency = latency_samples * input.sample_period();

	// clamp the latency to the start (only relevant at the beginning)
	s32 dstindex = 0;
	attotime output_start = output.start_time();
	while (latency > output_start && dstindex < numsamples)
	{
		output.put(dstindex++, 0);
		output_start += output.sample_period();
	}
	if (dstindex >= numsamples)
		return;

	// now rebase our input buffer around the adjusted start time
	input.set_start(output_start - latency);

	// compute the fractional input start position
	attotime delta = output_start - (input.start_time() + latency);
	sound_assert(delta.seconds() == 0);
	float srcpos = float(double(delta.attoseconds()) / double(input.sample_period_attoseconds()));
	sound_assert(srcpos <= 1.0f);

	// input is undersampled: point sample except where our sample period covers a boundary
	s32 srcindex = 0;
	if (step < 1.0f)
	{
		while (dstindex < numsamples)
		{
			// fill in with point samples until we hit a boundary
			float nextpos;
			while ((nextpos = srcpos + step) < 1.0f && dstindex < numsamples)
			{
				output.put(dstindex++, input.get(srcindex));
				srcpos = nextpos;
			}

			// if we're done, we're done
			if (dstindex >= numsamples)
				break;

			// blend between the two samples accordingly
			float sample = input.get(srcindex) * (1.0f - srcpos) + input.get(++srcindex) * (nextpos - 1.0f);
			output.put(dstindex++, sample * stepinv);

			// advance
			srcpos = nextpos - 1.0f;
			sound_assert(srcindex <= input.samples());
		}
	}

	// input is oversampled: sum the energy
	else
	{
		while (dstindex < numsamples)
		{
			// compute the partial first sample and advance
			float scale = 1.0f - srcpos;
			float sample = input.get(srcindex++) * scale;

			// add in complete samples until we only have a fraction left
			float remaining = step - scale;
			while (remaining >= 1.0f)
			{
				sample += input.get(srcindex++);
				remaining -= 1.0f;
			}

			// add in the final partial sample
			sample += input.get(srcindex) * remaining;
			output.put(dstindex++, sample * stepinv);

			// our position is now the remainder
			srcpos = remaining;
			sound_assert(srcindex <= input.samples());
		}
	}
}



//**************************************************************************
//  SOUND STREAM OUTPUT
//**************************************************************************

//-------------------------------------------------
//  sound_stream_output - constructor
//-------------------------------------------------

sound_stream_output::sound_stream_output() :
	m_stream(nullptr),
	m_index(0),
	m_gain(1.0)
{
}


//-------------------------------------------------
//  init - initialization
//-------------------------------------------------

void sound_stream_output::init(sound_stream &stream, u32 index, char const *tag)
{
	// set the passed-in data
	m_stream = &stream;
	m_index = index;

	// save our state
	auto &save = stream.device().machine().save();
	save.save_item(&stream.device(), "stream.output", tag, index, NAME(m_gain));

#if (LOG_OUTPUT_WAV)
	std::string filename = stream.device().machine().basename();
	filename +=	stream.device().tag();
	for (int index = 0; index < filename.size(); index++)
		if (filename[index] == ':')
			filename[index] = '_';
	if (dynamic_cast<default_resampler_stream *>(&stream) != nullptr)
		filename += "_resampler";
	filename += "_OUT_";
	char buf[10];
	sprintf(buf, "%d", index);
	filename += buf;
	filename += ".wav";
	m_buffer.open_wav(filename.c_str());
#endif
}


//-------------------------------------------------
//  name - return the friendly name of this output
//-------------------------------------------------

std::string sound_stream_output::name() const
{
	// start with our owning stream's name
	std::ostringstream str;
	util::stream_format(str, "%s Ch.%d", m_stream->name(), m_stream->output_base() + m_index);
	return str.str();
}



//**************************************************************************
//  SOUND STREAM INPUT
//**************************************************************************

//-------------------------------------------------
//  sound_stream_input - constructor
//-------------------------------------------------

sound_stream_input::sound_stream_input() :
	m_owner(nullptr),
	m_native_source(nullptr),
	m_resampler_source(nullptr),
	m_index(0),
	m_gain(1.0),
	m_user_gain(1.0)
{
}


//-------------------------------------------------
//  init - initialization
//-------------------------------------------------

void sound_stream_input::init(sound_stream &stream, u32 index, char const *tag, sound_stream_output *resampler)
{
	// set the passed-in values
	m_owner = &stream;
	m_index = index;
	m_resampler_source = resampler;

	// save our state
	auto &save = stream.device().machine().save();
	save.save_item(&stream.device(), "stream.input", tag, index, NAME(m_gain));
	save.save_item(&stream.device(), "stream.input", tag, index, NAME(m_user_gain));
}


//-------------------------------------------------
//  name - return the friendly name of this input
//-------------------------------------------------

std::string sound_stream_input::name() const
{
	// start with our owning stream's name
	std::ostringstream str;
	util::stream_format(str, "%s", m_owner->name());

	// if we have a source, indicate where the sound comes from by device name and tag
	if (valid())
		util::stream_format(str, " <- %s", m_native_source->name());
	return str.str();
}


//-------------------------------------------------
//  set_source - wire up the output source for
//  our consumption
//-------------------------------------------------

void sound_stream_input::set_source(sound_stream_output *source)
{
	m_native_source = source;
	if (m_resampler_source != nullptr)
		m_resampler_source->stream().set_input(0, &source->stream(), source->index());
}


//-------------------------------------------------
//  update - update our source's stream to the
//  current end time and return a view to its
//  contents
//-------------------------------------------------

read_stream_view sound_stream_input::update(attotime start, attotime end)
{
	// shouldn't get here unless valid
	sound_assert(valid());

	// determine if we need to use the resampler
	bool resampled = false;
	if (m_resampler_source != nullptr)
	{
		// if sample rates differ, then yet
		if (m_owner->sample_rate() != m_native_source->stream().sample_rate())
			resampled = true;

		// if not, keep the resampler's end time up to date
		else
			m_resampler_source->set_end_time(end);
	}

	// update the source, returning a view of the needed output over the start and end times
	sound_stream_output &source = resampled ? *m_resampler_source : *m_native_source;
	return source.stream().update_view(start, end, source.index()).set_gain(m_gain * m_user_gain * m_native_source->gain());
}


//-------------------------------------------------
//  apply_sample_rate_changes - tell our sources
//  to apply any sample rate changes, informing
//  them of our current rate
//-------------------------------------------------

void sound_stream_input::apply_sample_rate_changes(u32 updatenum, u32 downstream_rate)
{
	// skip if not valid
	if (!valid())
		return;

	// if we have a resampler, tell it (and it will tell the native source)
	if (m_resampler_source != nullptr)
		m_resampler_source->stream().apply_sample_rate_changes(updatenum, downstream_rate);

	// otherwise, just tell the native source directly
	else
		m_native_source->stream().apply_sample_rate_changes(updatenum, downstream_rate);
}



//**************************************************************************
//  SOUND MANAGER
//**************************************************************************

//-------------------------------------------------
//  sound_manager - constructor
//-------------------------------------------------

sound_manager::sound_manager(running_machine &machine) :
	m_machine(machine),
	m_update_timer(nullptr),
	m_update_number(0),
	m_last_update(attotime::zero),
	m_finalmix_leftover(0),
	m_samples_this_update(0),
	m_finalmix(machine.sample_rate()),
	m_leftmix(machine.sample_rate()),
	m_rightmix(machine.sample_rate()),
	m_compressor_scale(1.0),
	m_compressor_counter(0),
	m_muted(0),
	m_nosound_mode(machine.osd().no_sound()),
	m_attenuation(0),
	m_unique_id(0),
	m_wavfile(nullptr)
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

	// register callbacks
	machine.configuration().config_register("mixer", config_load_delegate(&sound_manager::config_load, this), config_save_delegate(&sound_manager::config_save, this));
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&sound_manager::pause, this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(&sound_manager::resume, this));
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&sound_manager::reset, this));
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&sound_manager::stop_recording, this));

	// register global states
	machine.save().save_item(NAME(m_last_update));

	// set the starting attenuation
	set_attenuation(machine.options().volume());

	// start the periodic update flushing timer
	m_update_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(sound_manager::update), this));
	m_update_timer->adjust(STREAMS_UPDATE_ATTOTIME, 0, STREAMS_UPDATE_ATTOTIME);
}


//-------------------------------------------------
//  sound_manager - destructor
//-------------------------------------------------

sound_manager::~sound_manager()
{
}


//-------------------------------------------------
//  start_recording - begin audio recording
//-------------------------------------------------

void sound_manager::start_recording()
{
	// open the output WAV file if specified
	const char *wavfile = machine().options().wav_write();
	if (wavfile[0] != 0 && m_wavfile == nullptr)
		m_wavfile = wav_open(wavfile, machine().sample_rate(), 2);
}


//-------------------------------------------------
//  stop_recording - end audio recording
//-------------------------------------------------

void sound_manager::stop_recording()
{
	// close any open WAV file
	if (m_wavfile != nullptr)
		wav_close(m_wavfile);
	m_wavfile = nullptr;
}


//-------------------------------------------------
//  stream_alloc - allocate a new stream
//-------------------------------------------------

sound_stream *sound_manager::stream_alloc(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_delegate callback)
{
	// determine output base
	u32 output_base = 0;
	for (auto &stream : m_stream_list)
		if (&stream->device() == &device)
			output_base += stream->output_count();

	m_stream_list.push_back(std::make_unique<sound_stream>(device, inputs, outputs, output_base, sample_rate, callback));
	return m_stream_list.back().get();
}


//-------------------------------------------------
//  stream_alloc - allocate a new stream with the
//  new-style callback and flags
//-------------------------------------------------

sound_stream *sound_manager::stream_alloc(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_ex_delegate callback, sound_stream_flags flags)
{
	// determine output base
	u32 output_base = 0;
	for (auto &stream : m_stream_list)
		if (&stream->device() == &device)
			output_base += stream->output_count();

	m_stream_list.push_back(std::make_unique<sound_stream>(device, inputs, outputs, output_base, sample_rate, callback, flags));
	return m_stream_list.back().get();
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
	for (device_mixer_interface &mixer : mixer_interface_iterator(machine().root_device()))
	{
		if (index < mixer.inputs())
		{
			info.mixer = &mixer;
			info.stream = mixer.input_to_stream_input(index, info.inputnum);
			sound_assert(info.stream != nullptr);
			return true;
		}
		index -= mixer.inputs();
	}

	// didn't locate
	info.mixer = nullptr;
	return false;
}


//-------------------------------------------------
//  mute - mute sound output
//-------------------------------------------------

void sound_manager::mute(bool mute, u8 reason)
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
	for (device_sound_interface &sound : sound_interface_iterator(machine().root_device()))
		sound.device().reset();
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

void sound_manager::config_load(config_type cfg_type, util::xml::data_node const *parentnode)
{
	// we only care about game files
	if (cfg_type != config_type::GAME)
		return;

	// might not have any data
	if (parentnode == nullptr)
		return;

	// iterate over channel nodes
	for (util::xml::data_node const *channelnode = parentnode->get_child("channel"); channelnode != nullptr; channelnode = channelnode->get_next_sibling("channel"))
	{
		mixer_input info;
		if (indexed_mixer_input(channelnode->get_attribute_int("index", -1), info))
		{
			float defvol = channelnode->get_attribute_float("defvol", 1.0f);
			float newvol = channelnode->get_attribute_float("newvol", -1000.0f);
			if (newvol != -1000.0f)
				info.stream->input(info.inputnum).set_user_gain(newvol / defvol);
		}
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void sound_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// we only care about game files
	if (cfg_type != config_type::GAME)
		return;

	// iterate over mixer channels
	if (parentnode != nullptr)
		for (int mixernum = 0; ; mixernum++)
		{
			mixer_input info;
			if (!indexed_mixer_input(mixernum, info))
				break;
			float newvol = info.stream->input(info.inputnum).user_gain();

			if (newvol != 1.0f)
			{
				util::xml::data_node *const channelnode = parentnode->add_child("channel", nullptr);
				if (channelnode != nullptr)
				{
					channelnode->set_attribute_int("index", mixernum);
					channelnode->set_attribute_float("newvol", newvol);
				}
			}
		}
}


//-------------------------------------------------
//  update - mix everything down to its final form
//  and send it to the OSD layer
//-------------------------------------------------

void sound_manager::update(void *ptr, int param)
{
	VPRINTF(("sound_update\n"));

	g_profiler.start(PROFILER_SOUND);

#if (SOUND_DEBUG)
	// dump the sound graph when we start up
	if (m_last_update == attotime::zero)
		for (speaker_device &speaker : speaker_device_iterator(machine().root_device()))
		{
			int dummy;
			sound_stream *output = speaker.output_to_stream_output(0, dummy);
			if (output)
				output->print_graph_recursive(0);
		}
#endif

	// force all the speaker streams to generate the proper number of samples
	m_samples_this_update = 0;
	for (speaker_device &speaker : speaker_device_iterator(machine().root_device()))
		speaker.mix(&m_leftmix[0], &m_rightmix[0], m_samples_this_update, (m_muted & MUTE_REASON_SYSTEM));

	// determine the maximum in this section
	stream_buffer::sample_t curmax = 0;
	for (int sampindex = 0; sampindex < m_samples_this_update; sampindex++)
	{
		auto sample = m_leftmix[sampindex];
		if (sample < 0)
			sample = -sample;
		if (sample > curmax)
			curmax = sample;

		sample = m_rightmix[sampindex];
		if (sample < 0)
			sample = -sample;
		if (sample > curmax)
			curmax = sample;
	}

	// pull in current compressor scale factor before modifying
	stream_buffer::sample_t lscale = m_compressor_scale;
	stream_buffer::sample_t rscale = m_compressor_scale;

	// if we're above what the compressor will handle, adjust the compression
	if (curmax * m_compressor_scale > 1.0)
	{
		m_compressor_scale = 1.0 / curmax;
		m_compressor_counter = STREAMS_UPDATE_FREQUENCY / 5;
	}

	// if we're currently scaled, wait a bit to see if we can trend back toward 1.0
	else if (m_compressor_counter != 0)
		m_compressor_counter--;

	// try to migrate toward 0 unless we're going to introduce clipping
	else if (m_compressor_scale < 1.0 && curmax * 1.01 * m_compressor_scale < 1.0)
		m_compressor_scale *= 1.01f;

#if (SOUND_DEBUG)
	if (lscale != m_compressor_scale)
	printf("scale=%.5f\n", m_compressor_scale);
#endif

	// track whether there are pending scale changes in left/right
	bool lpending = (lscale != m_compressor_scale);
	bool rpending = (rscale != m_compressor_scale);
	stream_buffer::sample_t lprev = 0, rprev = 0;

	// now downmix the final result
	u32 finalmix_step = machine().video().speed_factor();
	u32 finalmix_offset = 0;
	s16 *finalmix = &m_finalmix[0];
	int sample;
	for (sample = m_finalmix_leftover; sample < m_samples_this_update * 1000; sample += finalmix_step)
	{
		int sampindex = sample / 1000;

		// look for a zero-crossing to update compression
		stream_buffer::sample_t lsamp = m_leftmix[sampindex];
		if (lpending && ((lsamp < 0 && lprev > 0) || (lsamp > 0 && lprev < 0)))
		{
			lpending = false;
			lscale = m_compressor_scale;
		}
		lprev = lsamp;

		// clamp the left side
		lsamp *= lscale;
		if (lsamp > 1.0)
			lsamp = 1.0;
		else if (lsamp < -1.0)
			lsamp = -1.0;
		finalmix[finalmix_offset++] = s16(lsamp * 32767.0);

		// look for a zero-crossing to update compression
		stream_buffer::sample_t rsamp = m_rightmix[sampindex];
		if (rpending && ((rsamp < 0 && rprev > 0) || (rsamp > 0 && rprev < 0)))
		{
			rpending = false;
			rscale = m_compressor_scale;
		}
		rprev = rsamp;

		// clamp the left side
		rsamp *= rscale;
		if (rsamp > 1.0)
			rsamp = 1.0;
		else if (rsamp < -1.0)
			rsamp = -1.0;
		finalmix[finalmix_offset++] = s16(rsamp * 32767.0);
	}
	m_finalmix_leftover = sample - m_samples_this_update * 1000;

	// play the result
	if (finalmix_offset > 0)
	{
		if (!m_nosound_mode)
			machine().osd().update_audio_stream(finalmix, finalmix_offset / 2);
		machine().osd().add_audio_to_recording(finalmix, finalmix_offset / 2);
		machine().video().add_sound_to_recording(finalmix, finalmix_offset / 2);
		if (m_wavfile != nullptr)
			wav_add_data_16(m_wavfile, finalmix, finalmix_offset);
	}

	// see if we ticked over to the next second
	attotime curtime = machine().time();
	bool second_tick = false;
	if (curtime.seconds() != m_last_update.seconds())
	{
		sound_assert(curtime.seconds() == m_last_update.seconds() + 1);
		second_tick = true;
	}

	// remember the update time
	m_last_update = curtime;
	m_update_number++;

	// update sample rates if they have changed
	for (speaker_device &speaker : speaker_device_iterator(machine().root_device()))
	{
		sound_assert(speaker.outputs() == 1);
		int stream_out;
		sound_stream *stream = speaker.output_to_stream_output(0, stream_out);
		sound_assert(stream != nullptr);
		stream->apply_sample_rate_changes(m_update_number, machine().sample_rate());
	}

	// notify that new samples have been generated
	emulator_info::sound_hook();

	g_profiler.stop();
}


//-------------------------------------------------
//  samples - fills the specified buffer with
//  16-bit stereo audio samples generated during
//  the current frame
//-------------------------------------------------

void sound_manager::samples(s16 *buffer)
{
	for (int sample = 0; sample < m_samples_this_update * 2; sample++)
		*buffer++ = m_finalmix[sample];
}
