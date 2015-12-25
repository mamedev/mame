// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    sound.h

    Core sound interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SOUND_H__
#define __SOUND_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int STREAM_SYNC       = -1;       // special rate value indicating a one-sample-at-a-time stream
										// with actual rate defined by its input

//**************************************************************************
//  MACROS
//**************************************************************************

typedef delegate<void (sound_stream &, stream_sample_t **inputs, stream_sample_t **outputs, int samples)> stream_update_delegate;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
struct wav_file;


// structure describing an indexed mixer
struct mixer_input
{
	device_mixer_interface *mixer;          // owning device interface
	sound_stream *          stream;         // stream within the device
	int                     inputnum;       // input on the stream
};


// ======================> sound_stream

class sound_stream
{
	friend class simple_list<sound_stream>;
	friend class sound_manager;

	typedef void (*stream_update_func)(device_t *device, sound_stream *stream, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// stream output class
	class stream_output
	{
	public:
		// construction/destruction
		stream_output();
		stream_output &operator=(const stream_output &rhs) { assert(false); return *this; }

		// internal state
		sound_stream *      m_stream;               // owning stream
		std::vector<stream_sample_t> m_buffer;    // output buffer
		int                 m_dependents;           // number of dependents
		INT16               m_gain;                 // gain to apply to the output
	};

	// stream input class
	class stream_input
	{
	public:
		// construction/destruction
		stream_input();
		stream_input &operator=(const stream_input &rhs) { assert(false); return *this; }

		// internal state
		stream_output *     m_source;               // pointer to the sound_output for this source
		std::vector<stream_sample_t> m_resample;  // buffer for resampling to the stream's sample rate
		attoseconds_t       m_latency_attoseconds;  // latency between this stream and the input stream
		INT16               m_gain;                 // gain to apply to this input
		INT16               m_user_gain;            // user-controlled gain to apply to this input
	};

	// constants
	static const int OUTPUT_BUFFER_UPDATES      = 5;
	static const UINT32 FRAC_BITS               = 22;
	static const UINT32 FRAC_ONE                = 1 << FRAC_BITS;
	static const UINT32 FRAC_MASK               = FRAC_ONE - 1;

	// construction/destruction
	sound_stream(device_t &device, int inputs, int outputs, int sample_rate, stream_update_delegate callback);

public:
	// getters
	sound_stream *next() const { return m_next; }
	device_t &device() const { return m_device; }
	int sample_rate() const { return (m_new_sample_rate != 0) ? m_new_sample_rate : m_sample_rate; }
	attotime sample_time() const;
	attotime sample_period() const { return attotime(0, m_attoseconds_per_sample); }
	int input_count() const { return m_input.size(); }
	int output_count() const { return m_output.size(); }
	const char *input_name(int inputnum, std::string &str) const;
	device_t *input_source_device(int inputnum) const;
	int input_source_outputnum(int inputnum) const;
	float user_gain(int inputnum) const;
	float input_gain(int inputnum) const;
	float output_gain(int outputnum) const;

	// operations
	void set_input(int inputnum, sound_stream *input_stream, int outputnum = 0, float gain = 1.0f);
	void update();
	const stream_sample_t *output_since_last_update(int outputnum, int &numsamples);

	// timing
	void set_sample_rate(int sample_rate);
	void set_user_gain(int inputnum, float gain);
	void set_input_gain(int inputnum, float gain);
	void set_output_gain(int outputnum, float gain);

private:
	// helpers called by our friends only
	void update_with_accounting(bool second_tick);
	void apply_sample_rate_changes();

	// internal helpers
	void recompute_sample_rate_data();
	void allocate_resample_buffers();
	void allocate_output_buffers();
	void postload();
	void generate_samples(int samples);
	stream_sample_t *generate_resampled_data(stream_input &input, UINT32 numsamples);
	void sync_update(void *, INT32);

	// linking information
	device_t &          m_device;                     // owning device
	sound_stream *      m_next;                       // next stream in the chain

	// general information
	UINT32              m_sample_rate;                // sample rate of this stream
	UINT32              m_new_sample_rate;            // newly-set sample rate for the stream
	bool                m_synchronous;                // synchronous stream that runs at the rate of its input

	// timing information
	attoseconds_t       m_attoseconds_per_sample;     // number of attoseconds per sample
	INT32               m_max_samples_per_update;     // maximum samples per update
	emu_timer *         m_sync_timer;                 // update timer for synchronous streams

	// input information
	std::vector<stream_input> m_input;              // list of streams we directly depend upon
	std::vector<stream_sample_t *> m_input_array;   // array of inputs for passing to the callback

	// resample buffer information
	UINT32              m_resample_bufalloc;          // allocated size of each resample buffer

	// output information
	std::vector<stream_output> m_output;            // list of streams which directly depend upon us
	std::vector<stream_sample_t *> m_output_array;  // array of outputs for passing to the callback

	// output buffer information
	UINT32              m_output_bufalloc;            // allocated size of each output buffer
	INT32               m_output_sampindex;           // current position within each output buffer
	INT32               m_output_update_sampindex;    // position at time of last global update
	INT32               m_output_base_sampindex;      // sample at base of buffer, relative to the current emulated second

	// callback information
	stream_update_delegate  m_callback;                   // callback function
};


// ======================> sound_manager

class sound_manager
{
	friend class sound_stream;

	// reasons for muting
	static const UINT8 MUTE_REASON_PAUSE = 0x01;
	static const UINT8 MUTE_REASON_UI = 0x02;
	static const UINT8 MUTE_REASON_DEBUGGER = 0x04;
	static const UINT8 MUTE_REASON_SYSTEM = 0x08;

	// stream updates
	static const attotime STREAMS_UPDATE_ATTOTIME;

public:
	static const int STREAMS_UPDATE_FREQUENCY = 50;

	// construction/destruction
	sound_manager(running_machine &machine);
	~sound_manager();

	// getters
	running_machine &machine() const { return m_machine; }
	int attenuation() const { return m_attenuation; }
	sound_stream *first_stream() const { return m_stream_list.first(); }
	attotime last_update() const { return m_last_update; }
	attoseconds_t update_attoseconds() const { return m_update_attoseconds; }

	// stream creation
	sound_stream *stream_alloc(device_t &device, int inputs, int outputs, int sample_rate, stream_update_delegate callback = stream_update_delegate());

	// global controls
	void set_attenuation(int attenuation);
	void ui_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_UI); }
	void debugger_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_DEBUGGER); }
	void system_mute(bool turn_off = true) { mute(turn_off, MUTE_REASON_SYSTEM); }
	void system_enable(bool turn_on = true) { mute(!turn_on, MUTE_REASON_SYSTEM); }

	// user gain controls
	bool indexed_mixer_input(int index, mixer_input &info) const;

private:
	// internal helpers
	void mute(bool mute, UINT8 reason);
	void reset();
	void pause();
	void resume();
	void config_load(int config_type, xml_data_node *parentnode);
	void config_save(int config_type, xml_data_node *parentnode);

	void update(void *ptr = nullptr, INT32 param = 0);

	// internal state
	running_machine &   m_machine;              // reference to our machine
	emu_timer *         m_update_timer;         // timer to drive periodic updates

	UINT32              m_finalmix_leftover;
	std::vector<INT16>       m_finalmix;
	std::vector<INT32>       m_leftmix;
	std::vector<INT32>       m_rightmix;

	UINT8               m_muted;
	int                 m_attenuation;
	int                 m_nosound_mode;

	wav_file *          m_wavfile;

	// streams data
	simple_list<sound_stream> m_stream_list;    // list of streams
	attoseconds_t       m_update_attoseconds;   // attoseconds between global updates
	attotime            m_last_update;          // last update time
};


#endif  /* __SOUND_H__ */
