// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    disound.h

    Device sound interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DISOUND_H
#define MAME_EMU_DISOUND_H

#include <functional>
#include <string>
#include <utility>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// special sample-rate values
constexpr u32 SAMPLE_RATE_INPUT_ADAPTIVE = 0xffffffff;
constexpr u32 SAMPLE_RATE_OUTPUT_ADAPTIVE = 0xfffffffe;
constexpr u32 SAMPLE_RATE_ADAPTIVE = 0xfffffffd;

constexpr int ALL_OUTPUTS       = 65535;    // special value indicating all outputs for the current chip



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

using stream_update_delegate = delegate<void (sound_stream &stream)>;


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
	const std::string &name() const { return m_name; }
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
	u64 start_index() const     { return m_output_buffer.write_sample(); }
	u64 end_index() const       { return m_output_buffer.write_sample() + samples(); }
	attotime start_time() const { return sample_to_time(start_index()); }
	attotime end_time() const   { return sample_to_time(end_index()); }

	// convert from absolute sample index to time
	attotime sample_to_time(u64 index) const;

	// gain management
	float user_output_gain() const           { return m_user_output_gain; }
	float user_output_gain(s32 output) const { return m_user_output_channel_gain[output]; }
	float input_gain(s32 input) const        { return m_input_channel_gain[input]; }
	float output_gain(s32 output) const      { return m_output_channel_gain[output]; }

	void set_user_output_gain(float gain);
	void set_user_output_gain(s32 output, float gain);
	void set_input_gain(s32 input, float gain);
	void apply_input_gain(s32 input, float gain);
	void set_output_gain(s32 output, float gain);
	void apply_output_gain(s32 output, float gain);

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
	void add(s32 output, s32 index, sample_t sample) { *m_output_buffer.ptrw(output, index) += sample; }

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


// ======================> device_sound_interface

class device_sound_interface : public device_interface
{
	friend class sound_manager;

public:
	class sound_route
	{
	public:
		u32                                 m_output;           // output index, or ALL_OUTPUTS
		u32                                 m_input;            // target input index
		float                               m_gain;             // gain
		std::reference_wrapper<device_t>    m_base;             // target search base
		std::string                         m_target;           // target tag
		device_sound_interface             *m_interface;        // target device interface
	};

	// construction/destruction
	device_sound_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sound_interface();

	// configuration access
	std::vector<sound_route> const &routes() const { return m_route_list; }

	// configuration helpers
	template <typename T, bool R>
	device_sound_interface &add_route(u32 output, const device_finder<T, R> &target, double gain, u32 channel = 0)
	{
		const std::pair<device_t &, const char *> ft(target.finder_target());
		return add_route(output, ft.first, ft.second, gain, channel);
	}
	device_sound_interface &add_route(u32 output, const char *target, double gain, u32 channel = 0);
	device_sound_interface &add_route(u32 output, device_sound_interface &target, double gain, u32 channel = 0);
	device_sound_interface &reset_routes() { m_route_list.clear(); return *this; }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) = 0;

	// stream creation
	sound_stream *stream_alloc(int inputs, int outputs, int sample_rate, sound_stream_flags flags = sound_stream_flags(0));

	// helpers
	int inputs() const;
	int outputs() const;
	std::pair<sound_stream *, int> input_to_stream_input(int inputnum) const;
	std::pair<sound_stream *, int> output_to_stream_output(int outputnum) const;
	float input_gain(int inputnum) const;
	float output_gain(int outputnum) const;
	float user_output_gain() const;
	float user_output_gain(int outputnum) const;
	void set_input_gain(int inputnum, float gain);
	void set_output_gain(int outputnum, float gain);
	void set_user_output_gain(float gain);
	void set_user_output_gain(int outputnum, float gain);
	void set_route_gain(int source_channel, device_sound_interface *target, int target_channel, float gain);

	void set_sound_hook(bool enable) { m_sound_hook = enable; }
	bool get_sound_hook() const { return m_sound_hook; }

protected:
	// configuration access
	std::vector<sound_route> &routes() { return m_route_list; }
	device_sound_interface &add_route(u32 output, device_t &base, const char *tag, double gain, u32 channel);

	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const override;

	u32 get_sound_requested_inputs() const { return m_sound_requested_inputs; }
	u32 get_sound_requested_outputs() const { return m_sound_requested_outputs; }
	u64 get_sound_requested_inputs_mask() const { return m_sound_requested_inputs_mask; }
	u64 get_sound_requested_outputs_mask() const { return m_sound_requested_outputs_mask; }

private:
	void sound_request_input(u32 input);

	// internal state
	std::vector<sound_route> m_route_list;      // list of sound routes
	std::vector<sound_stream *> m_sound_streams;
	u64 m_sound_requested_inputs_mask;
	u64 m_sound_requested_outputs_mask;
	u32 m_sound_requested_inputs;
	u32 m_sound_requested_outputs;
	bool m_sound_hook;

	void sound_before_devices_init();
	void sound_after_devices_init();
};

// iterator
typedef device_interface_enumerator<device_sound_interface> sound_interface_enumerator;



// ======================> device_mixer_interface

class device_mixer_interface : public device_sound_interface
{
public:
	// construction/destruction
	device_mixer_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_mixer_interface();

protected:
	// optional operation overrides
	virtual void interface_pre_start() override;

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
};

// iterator
typedef device_interface_enumerator<device_mixer_interface> mixer_interface_enumerator;


#endif // MAME_EMU_DISOUND_H
