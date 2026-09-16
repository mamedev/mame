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

    SOUND_DISABLE_THREADING is to be defined when your environment does
    not support threads (e.g. emscripten).  The effects suddenly become
    costly then though.

***************************************************************************/

#pragma once

#ifndef MAME_EMU_SOUND_H
#define MAME_EMU_SOUND_H

#include "wavwrite.h"
#include "interface/audio.h"

#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#ifndef SOUND_DISABLE_THREADING
#include <mutex>
#include <thread>
#include <condition_variable>
#endif


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum sound_stream_flags : u32;

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
	void config_add_sound_io_connection_node(sound_io_device *dev, std::string_view name, float db, u32 index = ~0);
	void config_add_sound_io_connection_default(sound_io_device *dev, float db, u32 index = ~0);
	void config_remove_sound_io_connection_node(sound_io_device *dev, std::string_view name);
	void config_remove_sound_io_connection_default(sound_io_device *dev);
	void config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string_view name, float db);
	void config_set_volume_sound_io_connection_default(sound_io_device *dev, float db);
	void config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel, float db, u32 index = ~0);
	void config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db, u32 index = ~0);
	void config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel);
	void config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel);
	void config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel, float db);
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
	float resampler_hq_latency() const { return m_resampler_hq_latency; }
	u32 resampler_hq_length() const { return m_resampler_hq_length; }
	u32 resampler_hq_phases() const { return m_resampler_hq_phases; }

	u32 default_resampler_type() const;
	float default_resampler_hq_latency() const;
	u32 default_resampler_hq_length() const;
	u32 default_resampler_hq_phases() const;

	void set_resampler_type(u32 type);
	void set_resampler_hq_latency(float latency);
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

		osd_stream(u32 node, std::string &&node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			m_id(0),
			m_node(node),
			m_node_name(std::move(node_name)),
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
		osd_input_stream(u32 node, std::string &&node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			osd_stream(node, std::move(node_name), channels, rate, is_system_default, dev),
			m_buffer(rate, channels)
		{ }
	};

	struct osd_output_stream : public osd_stream {
		u32 m_samples;
		std::vector<s16> m_buffer;
		osd_output_stream(u32 node, std::string &&node_name, u32 channels, u32 rate, bool is_system_default, sound_io_device *dev) :
			osd_stream(node, std::move(node_name), channels, rate, is_system_default, dev),
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
	static std::vector<u32> find_channel_mapping(const osd::channel_position &pos, const osd::audio_info::node_info *node);
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
	void internal_config_add_sound_io_connection_node(sound_io_device *dev, std::string_view name, float db, u32 index = ~0);
	void internal_config_add_sound_io_connection_default(sound_io_device *dev, float db, u32 index = ~0);
	void internal_config_remove_sound_io_connection_node(sound_io_device *dev, std::string_view name);
	void internal_config_remove_sound_io_connection_default(sound_io_device *dev);
	void internal_config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string_view name, float db);
	void internal_config_set_volume_sound_io_connection_default(sound_io_device *dev, float db);
	void internal_config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel, float db, u32 index = ~0);
	void internal_config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db, u32 index = ~0);
	void internal_config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel);
	void internal_config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel);
	void internal_config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string_view name, u32 node_channel, float db);
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
	float m_resampler_hq_latency;
	u32 m_resampler_hq_length, m_resampler_hq_phases;
};


#endif // MAME_EMU_SOUND_H
