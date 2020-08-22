// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    sound.h

    Core sound interface functions and definitions.

****************************************************************************

	In MAME, sound is represented as a graph of sound "streams". Each
	stream has a fixed number of inputs and outputs, and is responsible
	for producing sound on demand.

	The graph is driven from the outputs, which are speaker devices.
	These devices are updated on a regular basis (~50 times per second),
	and when an update occurs, the graph is walked from the speaker
	through each input, until all connected streams are up to date.

	Individual streams can also be updated manually. This is important
	for sound chips and CPU-driven devices, who should force any
	affected streams to update prior to making changes.

	Sound streams are *not* part of the device execution model. This is
	very important to understand. If the process of producing the ouput
	stream affects state that might be consumed by an executing device
	(e.g., a CPU), then care must be taken to ensure that the stream is
	updated frequently enough

	The model for timing sound samples is very important and explained
	here. Each stream source has a clock (aka sample rate). Each clock
	edge represents a sample that is held for the duration of one clock
	period. This model has interesting effects:

	For example, if you have a 10Hz clock, and call stream.update() at
	t=0.91, it will compute 10 samples (for clock edges 0.0, 0.1, 0.2,
	..., 0.7, 0.8, and 0.9). And then if you ask the stream what its
	current end time is (via stream.sample_time()), it will say t=1.0,
	which is in the future, because it knows it will hold that last
	sample until 1.0s.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_SOUND_H
#define MAME_EMU_SOUND_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

constexpr u32 SAMPLE_RATE_INVALID = 0xffffffff;
constexpr u32 SAMPLE_RATE_INPUT_ADAPTIVE = 0xfffffffe;
constexpr u32 SAMPLE_RATE_OUTPUT_ADAPTIVE = 0xfffffffd;
constexpr u32 SAMPLE_RATE_SYNCHRONOUS = 0xfffffffc;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define SOUND_DEBUG (1)

#if (SOUND_DEBUG)
#define sound_assert(x) do { if (!(x)) __debugbreak(); } while (0)
#else
#define sound_assert assert
#endif


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> stream_buffer

class stream_buffer
{
	friend class read_stream_view;

public:
	using sample_t = float;

	// constructor/destructor
	stream_buffer(int sample_rate = 48000);
	~stream_buffer();

	// disable copying of stream_buffers directly
	stream_buffer(stream_buffer const &src) = delete;
	stream_buffer &operator=(stream_buffer const &rhs) = delete;

	// return the current sample rate
	u32 sample_rate() const { return m_sample_rate; }

	// return the current sample period in attoseconds
	attoseconds_t sample_period_attoseconds() const { return HZ_TO_ATTOSECONDS(m_sample_rate); }

	// return the attotime of the current end of buffer
	attotime end_time() const { return index_time(m_end_sample); }

	// set a new sample rate
	void set_sample_rate(u32 rate);

	// clear the buffer
	void clear();

	// set the time (for forced resyncs; generally not used)
	void set_end_time(attotime time) { m_end_second = time.seconds(); m_end_sample = u32(time.attoseconds() / m_sample_attos); }

	// read the sample at the given index (clamped)
	sample_t get(s32 index) const
	{
		sample_t value = m_buffer[clamp_index(index)];
#if (SOUND_DEBUG)
		sound_assert(!isnan(value));
#endif
		return value;
	}

	// write the sample at the given index (clamped)
	void put(s32 index, sample_t data)
	{
		m_buffer[clamp_index(index)] = data;
	}

private:
	// clamp an index to the size of the buffer; allows for indexing +/- one
	// buffers' worth of range
	u32 clamp_index(s32 index) const
	{
		if (index < 0)
			index += m_sample_rate;
		else if (index >= m_sample_rate)
			index -= m_sample_rate;
		sound_assert(index >= 0 && index < m_sample_rate);
		return index;
	}

	// return the attotime of a given index within the buffer
	attotime index_time(s32 index) const;

	// given an attotime, return the buffer index corresponding to it
	u32 time_to_buffer_index(attotime time, bool round_up = false);

	// internal state
	u32 m_end_second;
	u32 m_end_sample;
	u32 m_sample_rate;
	attoseconds_t m_sample_attos;
	std::vector<sample_t> m_buffer;

#if (SOUND_DEBUG)
public:
	void open_wav(char const *filename);
	void flush_wav();
	void close_wav();

private:
	struct wav_file *m_wav_file = nullptr;
	u32 m_last_written = 0;
#endif
};


// ======================> read_stream_view

class read_stream_view
{
	friend class write_stream_view;

public:
	using sample_t = stream_buffer::sample_t;

	// constructor
	read_stream_view() :
		m_buffer(nullptr),
		m_start(0),
		m_end(0),
		m_gain(1.0)
	{
	}

	// constructor
	read_stream_view(stream_buffer *buffer, s32 start, s32 end, sample_t gain) :
		m_buffer(buffer),
		m_start(start),
		m_end(end),
		m_gain(gain)
	{
		normalize_start_end();
	}

	// return a read_stream_view covering the given time period
	read_stream_view(stream_buffer &buffer, attotime start, attotime end, sample_t gain) :
		m_buffer(&buffer),
		m_start(0),
		m_end(buffer.time_to_buffer_index(end, true)),
		m_gain(gain)
	{
		// it's important to compute the end first, since it could invalidate the start
		m_start = buffer.time_to_buffer_index(start, false);
		normalize_start_end();
	}

	// copy constructor
	read_stream_view(read_stream_view const &src) :
		read_stream_view(src.m_buffer, src.m_start, src.m_end, src.m_gain)
	{
		normalize_start_end();
	}

	// copy assignment
	read_stream_view &operator=(read_stream_view const &rhs)
	{
		m_buffer = rhs.m_buffer;
		m_start = rhs.m_start;
		m_end = rhs.m_end;
		m_gain = rhs.m_gain;
		normalize_start_end();
		return *this;
	}

	// return the local gain
	sample_t gain() const { return m_gain; }

	// return the sample rate of the data
	u32 sample_rate() const { return m_buffer->sample_rate(); }

	// return the sample period (in attoseconds) of the data
	attoseconds_t sample_period_attoseconds() const { return m_buffer->sample_period_attoseconds(); }

	// return the number of samples represented by the buffer
	u32 samples() const { return m_end - m_start; }

	// return the starting or ending time of the buffer, accounting for
	// the latency that was applied at creation
	attotime start_time() const { return m_buffer->index_time(m_start); }
	attotime end_time() const { return m_buffer->index_time(m_end); }

	// set the start time
	read_stream_view &set_start(attotime start)
	{
		m_start = m_buffer->time_to_buffer_index(start);
		normalize_start_end();
		return *this;
	}

	// set the gain
	read_stream_view &set_gain(float gain) { m_gain = gain; return *this; }
	read_stream_view &apply_gain(float gain) { m_gain *= gain; return *this; }

	// safely fetch a gain-scaled sample from the buffer
	sample_t get(s32 index) const
	{
		if (u32(index) >= samples())
			return 0;
		return m_buffer->get(m_start + index) * m_gain;
	}

	// safely fetch a raw sample from the buffer, relative to the view start; if you
	// use this, you need to apply the gain yourself for correctness
	sample_t getraw(s32 index) const
	{
		if (u32(index) >= samples())
			return 0;
		return m_buffer->get(m_start + index);
	}

//protected:
	// normalize start/end
	void normalize_start_end()
	{
		// ensure that end is always greater than start; we'll
		// wrap to the buffer length as needed
		if (m_end < m_start && m_buffer != nullptr)
			m_end += m_buffer->sample_rate();
		sound_assert(m_end >= m_start);
	}

	// internal state
	stream_buffer *m_buffer;
	s32 m_start;
	s32 m_end;
	sample_t m_gain;
};


// ======================> write_stream_view

class write_stream_view : public read_stream_view
{
public:
	// constructor
	write_stream_view(stream_buffer *buffer = nullptr, s32 start = 0, s32 end = 0) :
		read_stream_view(buffer, start, end, sample_t(1.0))
	{
	}

	// return a write_stream_view covering the given time period
	write_stream_view(stream_buffer &buffer, attotime start, attotime end) :
		read_stream_view(buffer, start, end, sample_t(1.0))
	{
	}

	// converter from read
	write_stream_view(read_stream_view &src) :
		read_stream_view(src.m_buffer, src.m_start, src.m_end, sample_t(1.0))
	{
	}

	// safely write a gain-applied sample to the buffer
	void put(s32 index, sample_t sample)
	{
		sound_assert(u32(index) < samples());
		if (u32(index) < samples())
			m_buffer->put(m_start + index, sample);
	}

	// safely add a gain-applied sample to the buffer
	void add(s32 index, sample_t sample)
	{
		sound_assert(u32(index) < samples());
		if (u32(index) < samples())
			m_buffer->put(m_start + index, m_buffer->get(m_start + index) + sample);
	}

	// clear the view to the given value
	void clear(sample_t value = 0)
	{
		s32 count = samples();
		for (s32 index = 0; index < count; index++)
			put(index, value);
	}

	// copy the view from another view
	void copy(read_stream_view &src)
	{
		s32 count = samples();
		for (s32 index = 0; index < count; index++)
			put(index, src.get(index));
	}

	// add the view from another view to our current values
	void add(read_stream_view &src)
	{
		s32 count = samples();
		for (s32 index = 0; index < count; index++)
			add(index, src.get(index));
	}
};


// ======================> stream_update_delegate/stream_update_ex_delegate

using stream_update_delegate = delegate<void (sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)>;
using stream_update_ex_delegate = delegate<void (sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)>;


// ======================> sound_stream_flags

enum sound_stream_flags : u32
{
	STREAM_RESAMPLER_MASK = 0x0f,
		STREAM_RESAMPLER_DEFAULT = 0,
		STREAM_RESAMPLER_NONE = 1,

	STREAM_SYNCHRONOUS = 0x10,
};


// ======================> sound_stream

class sound_stream
{
	friend class sound_manager;

	// stream output class
	class stream_output
	{
		friend class sound_stream;

	public:
		// construction/destruction
		stream_output();

		// initialization
		void init(sound_stream &stream, u32 index, char const *tag_base);

		// no copying allowed
		stream_output(stream_output const &src) = delete;
		stream_output &operator=(stream_output const &rhs) = delete;

		// simple getters
		sound_stream &stream() const { sound_assert(m_stream != nullptr); return *m_stream; }
		attotime end_time() const { return m_buffer.end_time(); }
		u32 index() const { return m_index; }
		float gain() const { return m_gain; }

		// simple setters
		void set_gain(float gain) { m_gain = gain; }

		// handle a changing sample rate
		void sample_rate_changed(u32 rate) { m_buffer.set_sample_rate(rate); }

		// return an output view
		write_stream_view view(attotime start, attotime end) { return write_stream_view(m_buffer, start, end); }

		// resync the buffer to the given end time
		void set_end_time(attotime end) { m_buffer.set_end_time(end); }

	private:
		// internal state
		sound_stream *m_stream;               // owning stream
		stream_buffer m_buffer;               // output buffer
		u32 m_index;                          // output index
		float m_gain;                         // gain to apply to the output
	};

	// stream input class
	class stream_input
	{
		friend class sound_stream;

	public:
		// construction/destruction
		stream_input();

		// initialization
		void init(sound_stream &stream, u32 index, char const *tag_base, stream_output *resampler);

		// no copying allowed
		stream_input(stream_input const &src) = delete;
		stream_input &operator=(stream_input const &rhs) = delete;

		// simple getters
		sound_stream &owner() const { sound_assert(m_owner != nullptr); return *m_owner; }
		bool valid() const { return (m_native_source != nullptr); }
		stream_output &source() const { sound_assert(valid()); return *m_native_source; }
		u32 index() const { return m_index; }
		float gain() const { return m_gain; }
		float user_gain() const { return m_user_gain; }

		// simple setters
		void set_gain(float gain) { m_gain = gain; }
		void set_user_gain(float gain) { m_user_gain = gain; }

		// connect the source
		void set_source(stream_output *source);

		// handle a changing sample rate
		void sample_rate_changed(u32 rate);

		// update and return an input view
		read_stream_view update(attotime start, attotime end);

	private:
		// internal state
		sound_stream *m_owner;                 // reference to the owning stream
		stream_output *m_native_source;        // pointer to the native sound_output
		stream_output *m_resampler_source;     // pointer to the resampled output; changed dynamically
		std::unique_ptr<sound_stream> m_resampler; // the resampler stream, if needed
		u32 m_index;                           // index of ourself
		float m_gain;                          // gain to apply to this input
		float m_user_gain;                     // user-controlled gain to apply to this input
	};

public:
	// construction/destruction
	sound_stream(device_t &device, int inputs, int outputs, int sample_rate, stream_update_delegate callback, sound_stream_flags flags = STREAM_RESAMPLER_DEFAULT);
	sound_stream(device_t &device, int inputs, int outputs, int sample_rate, stream_update_ex_delegate callback, sound_stream_flags flags = STREAM_RESAMPLER_DEFAULT);
	virtual ~sound_stream();

	void init_common(int inputs, int outputs, int sample_rate, sound_stream_flags flags);

	// getters
	sound_stream *next() const { return m_next; }
	device_t &device() const { return m_device; }
	char const *name() const { return m_name.c_str(); }
	int sample_rate() const { return (m_pending_sample_rate != SAMPLE_RATE_INVALID) ? m_pending_sample_rate : m_sample_rate; }
	attotime sample_time() const;
	attotime sample_period() const { return attotime(0, sample_period_attoseconds()); }
	attoseconds_t sample_period_attoseconds() const { return (m_sample_rate != SAMPLE_RATE_INVALID) ? HZ_TO_ATTOSECONDS(m_sample_rate) : 0; }
	int input_count() const { return m_input.size(); }
	int output_count() const { return m_output.size(); }
	std::string input_name(int inputnum) const;
	device_t *input_source_device(int inputnum) const;
	int input_source_outputnum(int inputnum) const;
	float user_gain(int inputnum) const;
	float input_gain(int inputnum) const;
	float output_gain(int outputnum) const;
	bool synchronous() const { return m_synchronous; }
	bool input_adaptive() const { return m_input_adaptive || m_synchronous; }
	bool output_adaptive() const { return m_output_adaptive; }

	// operations
	void set_input(int inputnum, sound_stream *input_stream, int outputnum = 0, float gain = 1.0f);
	void update();
	read_stream_view update_view(attotime start, attotime end, u32 outputnum = 0);

	// timing
	void set_sample_rate(int sample_rate);
	void set_user_gain(int inputnum, float gain);
	void set_input_gain(int inputnum, float gain);
	void set_output_gain(int outputnum, float gain);

	// debugging
#if (SOUND_DEBUG)
	void print_graph_recursive(int indent);
#endif

private:
	// helpers called by our friends only
	void apply_sample_rate_changes();

	// internal helpers
	void sample_rate_changed();
	void reprime_sync_timer();
	void postload();
	void sync_update(void *, s32);
	void oldstyle_callback_ex(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs);

	// linking information
	device_t &m_device;                            // owning device
	sound_stream *m_next;                          // next stream in the chain

	// general information
	std::string m_name;                            // name of this stream
	u32 m_sample_rate;                             // current live sample rate
	u32 m_pending_sample_rate;                     // pending sample rate for dynamic changes
	bool m_input_adaptive;                         // adaptive stream that runs at the sample rate of its input
	bool m_output_adaptive;                        // adaptive stream that runs at the sample rate of its output
	bool m_synchronous;                            // synchronous stream that runs at the rate of its input
	emu_timer *m_sync_timer;                       // update timer for synchronous streams

	// input information
	std::vector<stream_input> m_input;             // list of streams we directly depend upon
	std::vector<stream_sample_t *> m_input_array;  // array of inputs for passing to the callback
	std::vector<read_stream_view> m_input_view;    // array of output views for passing to the callback
	std::vector<std::unique_ptr<sound_stream>> m_resampler_list; // internal list of resamplers

	// output information
	std::vector<stream_output> m_output;            // list of streams which directly depend upon us
	std::vector<stream_sample_t *> m_output_array;  // array of outputs for passing to the callback
	std::vector<write_stream_view> m_output_view;   // array of output views for passing to the callback

	// callback information
	stream_update_delegate m_callback;              // callback function
	stream_update_ex_delegate m_callback_ex;        // extended callback function
};


// ======================> default_resampler_stream

class default_resampler_stream : public sound_stream
{
public:
	// construction/destruction
	default_resampler_stream(device_t &device);

	// update handler
	void resampler_sound_update(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs);

private:
	// internal state
	u32 m_max_latency;
};


// ======================> sound_manager

// structure describing an indexed mixer
struct mixer_input
{
	device_mixer_interface *mixer;          // owning device interface
	sound_stream *          stream;         // stream within the device
	int                     inputnum;       // input on the stream
};

class sound_manager
{
	friend class sound_stream;

	// reasons for muting
	static constexpr u8 MUTE_REASON_PAUSE = 0x01;
	static constexpr u8 MUTE_REASON_UI = 0x02;
	static constexpr u8 MUTE_REASON_DEBUGGER = 0x04;
	static constexpr u8 MUTE_REASON_SYSTEM = 0x08;

	// stream updates
	static const attotime STREAMS_UPDATE_ATTOTIME;

public:
	static constexpr int STREAMS_UPDATE_FREQUENCY = 50;

	// construction/destruction
	sound_manager(running_machine &machine);
	~sound_manager();

	// getters
	running_machine &machine() const { return m_machine; }
	int attenuation() const { return m_attenuation; }
	const std::vector<std::unique_ptr<sound_stream>> &streams() const { return m_stream_list; }
	attotime last_update() const { return m_last_update; }
	attoseconds_t update_attoseconds() const { return m_update_attoseconds; }
	int sample_count() const { return m_samples_this_update; }
	void samples(s16 *buffer);
	int unique_id() { return m_unique_id++; }

	// stream creation
	sound_stream *stream_alloc(device_t &device, int inputs, int outputs, int sample_rate, stream_update_delegate callback);
	sound_stream *stream_alloc(device_t &device, int inputs, int outputs, int sample_rate, stream_update_ex_delegate callback, sound_stream_flags resampler = STREAM_RESAMPLER_DEFAULT);

	// global controls
	void start_recording();
	void stop_recording();
	void set_attenuation(int attenuation);
	void ui_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_UI); }
	void debugger_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_DEBUGGER); }
	void system_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_SYSTEM); }
	void system_enable(bool turn_on = true) { mute(!turn_on, MUTE_REASON_SYSTEM); }

	// user gain controls
	bool indexed_mixer_input(int index, mixer_input &info) const;

private:
	// internal helpers
	void mute(bool mute, u8 reason);
	void reset();
	void pause();
	void resume();
	void config_load(config_type cfg_type, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	void update(void *ptr = nullptr, s32 param = 0);

	// internal state
	running_machine &m_machine;
	emu_timer *m_update_timer;

	u32 m_finalmix_leftover;
	std::vector<s16> m_finalmix;
	std::vector<stream_buffer::sample_t> m_leftmix;
	std::vector<stream_buffer::sample_t> m_rightmix;
	int m_samples_this_update;

	u8 m_muted;
	int m_attenuation;
	int m_nosound_mode;
	int m_unique_id;
	wav_file *m_wavfile;

	// streams data
	std::vector<std::unique_ptr<sound_stream>> m_stream_list;    // list of streams
	attoseconds_t m_update_attoseconds;           // attoseconds between global updates
	attotime m_last_update;                       // last update time
};


#endif // MAME_EMU_SOUND_H
