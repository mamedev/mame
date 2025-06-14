// license:BSD-3-Clause
// copyright-holders:O. Galibert, Aaron Giles
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
    current end time is (via stream.end_time()), it will say t=1.0,
    which is in the future, because it knows it will hold that last
    sample until 1.0s.

    Sound generation callbacks are presented with a std::vector of inputs
    and outputs. The vectors contain objects of read_stream_view and
    write_stream_view respectively, which wrap access to a circular buffer
    of samples. Sound generation callbacks are expected to fill all the
    samples described by the outputs' write_stream_view objects. At the
    moment, all outputs have the same sample rate, so the number of samples
    that need to be generated will be consistent across all outputs.

    By default, the inputs will have been resampled to match the output
    sample rate, unless otherwise specified.

    SOUND_DISABLE_THREADING if to be defined when your environment does
    not support threads (e.g. emscripten).  The effects suddendly become
    costly then though.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_SOUND_H
#define MAME_EMU_SOUND_H

#include "wavwrite.h"
#include "interface/audio.h"

#ifndef SOUND_DISABLE_THREADING
#include <mutex>
#include <thread>
#include <condition_variable>
#endif


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// special sample-rate values
constexpr u32 SAMPLE_RATE_INPUT_ADAPTIVE = 0xffffffff;
constexpr u32 SAMPLE_RATE_OUTPUT_ADAPTIVE = 0xfffffffe;
constexpr u32 SAMPLE_RATE_ADAPTIVE  = 0xfffffffd;

//**************************************************************************
//  DEBUGGING
//**************************************************************************

// turn this on to enable aggressive assertions and other checks
#ifdef MAME_DEBUG
#define SOUND_DEBUG (1)
#else
#define SOUND_DEBUG (1)
#endif

// if SOUND_DEBUG is on, make assertions fire regardless of MAME_DEBUG
#if (SOUND_DEBUG)
#define sound_assert(x) do { if (!(x)) { osd_printf_error("sound_assert: " #x "\n"); osd_break_into_debugger("sound_assert: " #x "\n"); } } while (0)
#else
#define sound_assert assert
#endif

using stream_update_delegate = delegate<void (sound_stream &stream)>;
class audio_effect;
class audio_resampler;

// ======================> sound_stream_flags

enum sound_stream_flags : u32
{
	// default is no special flags
	STREAM_DEFAULT_FLAGS = 0x00,

	// specify that updates should be forced to one sample at a time, in real time
	// this implicitly creates a timer that runs at the stream's output frequency
	// so only use when strictly necessary
	STREAM_SYNCHRONOUS = 0x01
};

namespace emu::detail {
	template<typename S> class output_buffer_interleaved {
	public:
		output_buffer_interleaved(u32 buffer_size, u32 channels);

		void set_buffer_size(u32 buffer_size);

		u32 channels() const { return m_channels; }
		u64 sync_sample() const { return m_sync_sample; }
		void set_sync_sample(u64 sample) { m_sync_sample = sample; }
		u64 write_sample() const { return m_sync_sample + m_write_position - m_sync_position; }
		void prepare_space(u32 samples);
		void commit(u32 samples);
		void sync();

		void ensure_size(u32 buffer_size);
		void set_history(u32 history);

		u32 available_samples() const { return m_write_position - m_sync_position; }
		S *ptrw(u32 channel, s32 index) { return &m_buffer[(m_write_position + index) * m_channels + channel]; }
		const S *ptrw(u32 channel, s32 index) const { return &m_buffer[(m_write_position + index) * m_channels + channel]; }
		const S *ptrs(u32 channel, s32 index) const { return &m_buffer[(m_sync_position + index) * m_channels + channel]; }

	private:
		std::vector<S> m_buffer;
		u64 m_sync_sample;
		u32 m_write_position;
		u32 m_sync_position;
		u32 m_history;
		u32 m_channels;
	};

	template<typename S> class output_buffer_flat {
	public:
		output_buffer_flat(u32 buffer_size, u32 channels);

		void set_buffer_size(u32 buffer_size);

		u32 channels() const { return m_channels; }
		u64 sync_sample() const { return m_sync_sample; }
		void set_sync_sample(u64 sample) { m_sync_sample = sample; }
		u64 write_sample() const { return m_sync_sample + m_write_position - m_sync_position; }

		void prepare_space(u32 samples);
		void commit(u32 samples);
		void sync();

		void ensure_size(u32 buffer_size);
		void set_history(u32 history);

		void resample(u32 previous_rate, u32 next_rate, attotime sync_time, attotime now);

		void register_save_state(device_t &device, const char *id1, const char *id2);

		u32 available_samples() const { return m_write_position - m_sync_position; }
		S *ptrw(u32 channel, s32 index) { return &m_buffer[channel][m_write_position + index]; }
		const S *ptrw(u32 channel, s32 index) const { return &m_buffer[channel][m_write_position + index]; }
		const S *ptrs(u32 channel, s32 index) const { return &m_buffer[channel][m_sync_position + index]; }

	private:
		std::vector<std::vector<S>> m_buffer;
		u64 m_sync_sample;
		u32 m_write_position;
		u32 m_sync_position;
		u32 m_history;
		u32 m_channels;
	};
}

// ======================> sound_stream

class sound_stream
{
public:
	friend class sound_manager;
	using sample_t = float;

	// construction/destruction
	sound_stream(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_delegate callback, sound_stream_flags flags = sound_stream_flags::STREAM_DEFAULT_FLAGS);
	virtual ~sound_stream();

	// simple getters
	device_t &device() const { return m_device; }
	std::string name() const { return m_name; }
	bool input_adaptive() const { return m_input_adaptive; }
	bool output_adaptive() const { return m_output_adaptive; }
	bool synchronous() const { return m_synchronous; }
	bool is_active() const { return m_sample_rate != 0; }

	// input and output getters
	u32 input_count() const  { return m_input_count; }
	u32 output_count() const { return m_output_count; }

	// sample rate and timing getters
	u32 sample_rate() const { return m_sample_rate; }
	attotime sample_period() const { return attotime::from_hz(m_sample_rate); }

	// sample id and timing of the first and last sample of the current update block, and first of the next sample block
	u64 start_index() const      { return m_output_buffer.write_sample(); }
	u64 end_index() const        { return m_output_buffer.write_sample() + samples(); }
	attotime start_time() const  { return sample_to_time(start_index()); }
	attotime end_time() const    { return sample_to_time(end_index()); }

	// convert from absolute sample index to time
	attotime sample_to_time(u64 index) const;

	// gain management
	float user_output_gain() const                    { return m_user_output_gain; }
	void set_user_output_gain(float gain)             { update(); m_user_output_gain = gain; }
	float user_output_gain(s32 output) const          { return m_user_output_channel_gain[output]; }
	void set_user_output_gain(s32 output, float gain) { update(); m_user_output_channel_gain[output] = gain; }

	float input_gain(s32 input) const                 { return m_input_channel_gain[input]; }
	void set_input_gain(s32 input, float gain)        { update(); m_input_channel_gain[input] = gain; }
	void apply_input_gain(s32 input, float gain)      { update(); m_input_channel_gain[input] *= gain; }
	float output_gain(s32 output) const               { return m_output_channel_gain[output]; }
	void set_output_gain(s32 output, float gain)      { update(); m_output_channel_gain[output] = gain; }
	void apply_output_gain(s32 output, float gain)    { update(); m_output_channel_gain[output] *= gain; }

	// set the sample rate of the stream
	void set_sample_rate(u32 sample_rate);

	// force an update to the current time
	void update();

	// number of samples to handle
	s32 samples() const { return m_samples_to_update; }

	// write a sample to the buffer
	void put(s32 output, s32 index, sample_t sample) { *m_output_buffer.ptrw(output, index) = sample; }

	// write a sample to the buffer, clamping to +/- the clamp value
	void put_clamp(s32 output, s32 index, sample_t sample, sample_t clamp = 1.0) { put(output, index, std::clamp(sample, -clamp, clamp)); }

	// write a sample to the buffer, converting from an integer with the given maximum
	void put_int(s32 output, s32 index, s32 sample, s32 max) { put(output, index, double(sample)/max); }

	// write a sample to the buffer, converting from an integer with the given maximum
	void put_int_clamp(s32 output, s32 index, s32 sample, s32 maxclamp) { put_int(output, index, std::clamp(sample, -maxclamp, maxclamp-1), maxclamp); }

	// safely add a sample to the buffer
	void add(s32 output, s32 index, sample_t sample)  { *m_output_buffer.ptrw(output, index) += sample; }

	// add a sample to the buffer, converting from an integer with the given maximum
	void add_int(s32 output, s32 index, s32 sample, s32 max) { add(output, index, double(sample)/max); }

	// fill part of the view with the given value
	void fill(s32 output, sample_t value, s32 start, s32 count) { std::fill(m_output_buffer.ptrw(output, start), m_output_buffer.ptrw(output, start) + count, value); }
	void fill(s32 output, sample_t value, s32 start) { std::fill(m_output_buffer.ptrw(output, start), m_output_buffer.ptrw(output, 0) + samples(), value); }
	void fill(s32 output, sample_t value) { std::fill(m_output_buffer.ptrw(output, 0), m_output_buffer.ptrw(output, 0) + samples(), value); }

	// copy data from the input
	void copy(s32 output, s32 input, s32 start, s32 count) { std::copy(m_input_buffer[input].begin() + start, m_input_buffer[input].begin() + start + count, m_output_buffer.ptrw(output, start)); }
	void copy(s32 output, s32 input, s32 start) { std::copy(m_input_buffer[input].begin() + start, m_input_buffer[input].begin() + samples(), m_output_buffer.ptrw(output, start)); }
	void copy(s32 output, s32 input) { std::copy(m_input_buffer[input].begin(), m_input_buffer[input].begin() + samples(), m_output_buffer.ptrw(output, 0)); }

	// fetch a sample from the input buffer
	sample_t get(s32 input, s32 index) const { return m_input_buffer[input][index]; }

	// fetch a sample from the output buffer
	sample_t get_output(s32 output, s32 index) const { return *m_output_buffer.ptrw(output, index); }

	void add_bw_route(sound_stream *source, int output, int input, float gain);
	void add_fw_route(sound_stream *target, int input, int output);
	std::vector<sound_stream *> sources() const;
	std::vector<sound_stream *> targets() const;

	bool set_route_gain(sound_stream *source, int source_channel, int target_channel, float gain);

private:
	struct route_bw {
		sound_stream *m_source;
		int m_output;
		int m_input;
		float m_gain;
		const audio_resampler *m_resampler;

		route_bw(sound_stream *source, int output, int input, float gain) : m_source(source), m_output(output), m_input(input), m_gain(gain), m_resampler(nullptr) {}
	};

	struct route_fw {
		sound_stream *m_target;
		int m_input;
		int m_output;

		route_fw(sound_stream *target, int input, int output) : m_target(target), m_input(input), m_output(output) {}
	};

	// perform most of the initialization here
	void init();

	// re-print the synchronization timer
	void reprime_sync_timer();

	// timer callback for synchronous streams
	void sync_update(s32);

	void update_nodeps();
	void sync(attotime now);
	u64 get_current_sample_index() const;
	void do_update();

	bool frequency_is_solved() const { return (!(m_input_adaptive || m_output_adaptive)) || m_sample_rate != 0; }
	bool try_solving_frequency();
	void register_state();
	void add_dependants(std::vector<sound_stream *> &deps);
	void compute_dependants();
	void create_resamplers();
	void lookup_history_sizes();
	u32 get_history_for_bw_route(const sound_stream *source, u32 channel) const;
	void internal_set_sample_rate(u32 sample_rate);

	std::string m_name;                            // name of this stream
	std::string m_state_tag;

	// linking information
	device_t &m_device;                            // owning device
	std::vector<route_bw> m_bw_routes;
	std::vector<route_fw> m_fw_routes;
	std::vector<sound_stream *> m_dependant_streams;

	// buffers
	std::vector<std::vector<sample_t>> m_input_buffer;
	emu::detail::output_buffer_flat<sample_t> m_output_buffer;
	attotime m_sync_time;
	s32 m_samples_to_update;

	// gains
	std::vector<float> m_input_channel_gain;
	std::vector<float> m_output_channel_gain;
	std::vector<float> m_user_output_channel_gain;
	float m_user_output_gain;

	// general information
	u32 m_sample_rate;                             // current sample rate
	u32 m_input_count;
	u32 m_output_count;
	bool m_input_adaptive;                         // adaptive stream that runs at the sample rate of its input
	bool m_output_adaptive;                        // adaptive stream that runs at the sample rate of its output
	bool m_synchronous;                            // synchronous stream that runs at the rate of its input
	bool m_started;
	bool m_in_update;
	emu_timer *m_sync_timer;                       // update timer for synchronous streams

	// callback information
	stream_update_delegate m_callback;             // update callback function
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
	using sample_t = sound_stream::sample_t;

	enum {
		RESAMPLER_LOFI,
		RESAMPLER_HQ
	};

	struct mapping {
		struct node_mapping {
			u32 m_node;
			float m_db;
			bool m_is_system_default;
		};

		struct channel_mapping {
			u32 m_guest_channel;
			u32 m_node;
			u32 m_node_channel;
			float m_db;
			bool m_is_system_default;
		};
		sound_io_device *m_dev;
		std::vector<node_mapping> m_node_mappings;
		std::vector<channel_mapping> m_channel_mappings;
	};

	static constexpr int STREAMS_UPDATE_FREQUENCY = 50;

	// construction/destruction
	sound_manager(running_machine &machine);
	~sound_manager();

	// getters
	running_machine &machine() const { return m_machine; }
	const std::vector<std::unique_ptr<sound_stream>> &streams() const { return m_stream_list; }
	int unique_id() { return m_unique_id++; }
	bool no_sound() const { return m_nosound_mode; }

	const typename osd::audio_info &get_osd_info() const { return m_osd_info; }
	const std::vector<mapping> &get_mappings() const { return m_mappings; }

	// allocate a new stream
	sound_stream *stream_alloc(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_delegate callback, sound_stream_flags flags);

	// WAV recording
	bool is_recording() const { return bool(m_wavfile); }
	bool start_recording();
	bool start_recording(std::string_view filename);
	void stop_recording();
	u32 outputs_count() const { return m_outputs_count; }

	// manage the sound_io mapping and volume configuration
	void config_add_sound_io_connection_node(sound_io_device *dev, std::string name, float db);
	void config_add_sound_io_connection_default(sound_io_device *dev, float db);
	void config_remove_sound_io_connection_node(sound_io_device *dev, std::string name);
	void config_remove_sound_io_connection_default(sound_io_device *dev);
	void config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string name, float db);
	void config_set_volume_sound_io_connection_default(sound_io_device *dev, float db);
	void config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db);
	void config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db);
	void config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel);
	void config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel);
	void config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db);
	void config_set_volume_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db);

	// mute sound for one of various independent reasons
	bool muted() const { return bool(m_muted); }
	bool ui_mute() const { return bool(m_muted & MUTE_REASON_UI); }
	bool debugger_mute() const { return bool(m_muted & MUTE_REASON_DEBUGGER); }
	bool system_mute() const { return bool(m_muted & MUTE_REASON_SYSTEM); }
	void ui_mute(bool turn_off) { mute(turn_off, MUTE_REASON_UI); }
	void debugger_mute(bool turn_off) { mute(turn_off, MUTE_REASON_DEBUGGER); }
	void system_mute(bool turn_off) { mute(turn_off, MUTE_REASON_SYSTEM); }

	// master gain
	float master_gain() const { return m_master_gain; }
	void set_master_gain(float gain) { m_master_gain = gain; }

	void before_devices_init();
	void after_devices_init();
	void postload();

	void input_get(int m_id, sound_stream &stream);
	void output_push(int m_id, sound_stream &stream);
	const audio_resampler *get_resampler(u32 fs, u32 ft);

	u32 effect_chains() const { return m_speakers.size(); }
	std::string effect_chain_tag(s32 index) const;
	std::vector<audio_effect *> effect_chain(s32 index) const;
	std::vector<audio_effect *> default_effect_chain() const;
	void default_effect_changed(u32 entry);

	void mapping_update();

	const char *resampler_type_names(u32 type) const;

	u32 resampler_type() const { return m_resampler_type; }
	double resampler_hq_latency() const { return m_resampler_hq_latency; }
	u32 resampler_hq_length() const { return m_resampler_hq_length; }
	u32 resampler_hq_phases() const { return m_resampler_hq_phases; }

	void set_resampler_type(u32 type);
	void set_resampler_hq_latency(double latency);
	void set_resampler_hq_length(u32 length);
	void set_resampler_hq_phases(u32 phases);

private:
	struct effect_step {
		std::unique_ptr<audio_effect> m_effect;
		emu::detail::output_buffer_flat<sample_t> m_buffer;
		effect_step(u32 buffer_size, u32 channels);
	};

	struct mixing_step {
		enum : u32 { CLEAR, COPY, ADD };
		u32 m_mode;
		u32 m_osd_index;
		u32 m_osd_channel;
		u32 m_device_index;
		u32 m_device_channel;
		float m_linear_volume;
	};

	struct speaker_info {
		speaker_device &m_dev;
		sound_stream *m_stream;
		u32 m_channels;
		u32 m_first_output;
		double m_speed_phase;

		emu::detail::output_buffer_flat<sample_t> m_buffer;
		emu::detail::output_buffer_flat<sample_t> m_effects_buffer;

		std::vector<effect_step> m_effects;

		speaker_info(speaker_device &dev, u32 rate, u32 first_output);
	};

	struct microphone_info {
		microphone_device &m_dev;
		u32 m_channels;

		std::vector<mixing_step> m_input_mixing_steps; // actions to take to fill the buffer
		std::vector<sample_t> m_buffer;
		microphone_info(microphone_device &dev);
	};

	struct osd_stream {
		u32 m_id;
		u32 m_node;
		std::string m_node_name;
		u32 m_channels;
		u32 m_rate;
		u32 m_unused_channels_mask;
		bool m_is_system_default;
		bool m_is_channel_mapping;
		sound_io_device *m_dev;
		std::vector<float> m_volumes;
		const audio_resampler *m_resampler;

		osd_stream(u32 node, std::string node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			m_id(0),
			m_node(node),
			m_node_name(node_name),
			m_channels(channels),
			m_rate(rate),
			m_unused_channels_mask(util::make_bitmask<u32>(channels)),
			m_is_system_default(is_system_default),
			m_is_channel_mapping(false),
			m_dev(dev),
			m_resampler(nullptr)
		{ }
	};

	struct osd_input_stream : public osd_stream {
		emu::detail::output_buffer_interleaved<s16> m_buffer;
		osd_input_stream(u32 node, std::string node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			osd_stream(node, node_name, channels, rate, is_system_default, dev),
			m_buffer(rate, channels)
		{ }
	};

	struct osd_output_stream : public osd_stream {
		u32 m_samples;
		std::vector<s16> m_buffer;
		osd_output_stream(u32 node, std::string node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			osd_stream(node, node_name, channels, rate, is_system_default, dev),
			m_samples(0),
			m_buffer(channels*rate, 0)
		{ }
	};

	struct config_mapping {
		std::string m_name;
		// "" to indicates default node
		std::vector<std::pair<std::string, float>> m_node_mappings;
		std::vector<std::tuple<u32, std::string, u32, float>> m_channel_mappings;
	};

	// set/reset the mute state for the given reason
	void mute(bool mute, u8 reason);

	// reset all sound chips
	void reset();

	// pause/resume sound output
	void pause();
	void resume();

	// handle configuration load/save
	void config_load(config_type cfg_type, config_level cfg_lvl, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// periodic sound update, called STREAMS_UPDATE_FREQUENCY per second
	void update(s32);

	// handle mixing mapping update if needed
	static std::vector<u32> find_channel_mapping(const std::array<double, 3> &position, const osd::audio_info::node_info *node);
	void startup_cleanups();
	void streams_update();
	template<bool is_output, typename S> void apply_osd_changes(std::vector<S> &streams);
	void osd_information_update();
	void generate_mapping();
	void update_osd_streams();
	void update_osd_input();
	void speakers_update(attotime endtime);
	void rebuild_all_resamplers();
	void rebuild_all_stream_resamplers();
	void run_effects();

	u64 rate_and_time_to_index(attotime time, u32 sample_rate) const;
	u64 rate_and_last_sync_to_index(u32 sample_rate) const { return rate_and_time_to_index(m_last_sync_time, sample_rate); }

	// manage the sound_io mapping and volume configuration,
	// but don't change generation because we're in the update process

	config_mapping &config_get_sound_io(sound_io_device *dev);
	void internal_config_add_sound_io_connection_node(sound_io_device *dev, std::string name, float db);
	void internal_config_add_sound_io_connection_default(sound_io_device *dev, float db);
	void internal_config_remove_sound_io_connection_node(sound_io_device *dev, std::string name);
	void internal_config_remove_sound_io_connection_default(sound_io_device *dev);
	void internal_config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string name, float db);
	void internal_config_set_volume_sound_io_connection_default(sound_io_device *dev, float db);
	void internal_config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db);
	void internal_config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db);
	void internal_config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel);
	void internal_config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel);
	void internal_config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db);
	void internal_config_set_volume_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db);


	// internal state
	running_machine &m_machine;            // reference to the running machine
	emu_timer *m_update_timer;             // timer that runs the update function
	attotime m_last_sync_time;
	std::vector<speaker_info> m_speakers;
	std::vector<microphone_info> m_microphones;

	std::vector<s16> m_record_buffer;      // pre-effects speaker samples for recording
	u32 m_record_samples;                  // how many samples for the next update
	osd::audio_info m_osd_info;            // current state of the osd information
	std::vector<mapping> m_mappings;       // current state of the mappings
	std::vector<osd_input_stream> m_osd_input_streams; // currently active osd streams
	std::vector<osd_output_stream> m_osd_output_streams; // currently active osd streams
	std::vector<mixing_step> m_output_mixing_steps; // actions to take to fill the osd streams buffers
	std::vector<config_mapping> m_configs; // mapping user configuration

#ifndef SOUND_DISABLE_THREADING
	std::mutex                      m_effects_mutex;
	std::mutex                      m_effects_data_mutex;
	std::condition_variable         m_effects_condition;
	std::unique_ptr<std::thread>    m_effects_thread;
#endif

	std::vector<std::unique_ptr<audio_effect>> m_default_effects;
	bool m_effects_done;
	attotime m_effects_prev_time, m_effects_cur_time;

	float m_master_gain;

	std::map<std::pair<u32, u32>, std::unique_ptr<audio_resampler>> m_resamplers;

	u8 m_muted;                            // bitmask of muting reasons
	bool m_nosound_mode;                   // true if we're in "nosound" mode
	int m_unique_id;                       // unique ID used for stream identification
	util::wav_file_ptr m_wavfile;          // WAV file for streaming

	// streams data
	std::vector<std::unique_ptr<sound_stream>> m_stream_list; // list of streams
	std::vector<sound_stream *> m_ordered_streams;  // Streams in update order
	u32 m_outputs_count;

	// resampler data
	u32 m_resampler_type;
	double m_resampler_hq_latency;
	u32 m_resampler_hq_length, m_resampler_hq_phases;
};


#endif // MAME_EMU_SOUND_H
