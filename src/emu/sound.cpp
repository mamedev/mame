// license:BSD-3-Clause
// copyright-holders:O. Galibert, Aaron Giles
/***************************************************************************

    sound.cpp

    Core sound functions and definitions.

***************************************************************************/

#include "emu.h"

#include "audio_effects/aeffect.h"
#include "resampler.h"

#include "config.h"
#include "emuopts.h"
#include "main.h"
#include "speaker.h"

#include "wavwrite.h"
#include "xmlfile.h"

#include "osdepend.h"

#include "util/language.h"

#include <algorithm>

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_OUTPUT_FUNC m_machine.logerror

#define LOG_OSD_INFO    (1U << 1)
#define LOG_MAPPING     (1U << 2)
#define LOG_OSD_STREAMS (1U << 3)
#define LOG_ORDER       (1U << 4)

#define VERBOSE 0

#include "logmacro.h"



const attotime sound_manager::STREAMS_UPDATE_ATTOTIME = attotime::from_hz(STREAMS_UPDATE_FREQUENCY);


//**// Output buffer management

// Output buffers store samples produced every system-wide update.
// They give access to a window of samples produced before the update,
// and ensure that enough space is available to fit the update.


template<typename S> emu::detail::output_buffer_interleaved<S>::output_buffer_interleaved(u32 buffer_size, u32 channels) :
	m_buffer(channels*buffer_size, 0),
	m_sync_sample(0),
	m_write_position(0),
	m_sync_position(0),
	m_history(0),
	m_channels(channels)
{
}

template<typename S> void emu::detail::output_buffer_interleaved<S>::set_buffer_size(u32 buffer_size)
{
	m_buffer.resize(m_channels*buffer_size, 0);
}

template<typename S> void emu::detail::output_buffer_interleaved<S>::prepare_space(u32 samples)
{
	if(!m_channels)
		return;

	//   Check if potential overflow, bring data back up front if needed
	u32 buffer_size = m_buffer.size() / m_channels;
	if(m_write_position + samples > buffer_size) {
		u32 source_start = (m_sync_position - m_history) * m_channels;
		u32 source_end = m_write_position * m_channels;
		std::copy(m_buffer.begin() + source_start, m_buffer.begin() + source_end, m_buffer.begin());
		m_write_position -= m_sync_position - m_history;
		m_sync_position = m_history;
	}

	//   Clear the destination range
	u32 fill_start = m_write_position * m_channels;
	u32 fill_end = (m_write_position + samples) * m_channels;
	std::fill(m_buffer.begin() + fill_start, m_buffer.begin() + fill_end, 0.0);
}

template<typename S> void emu::detail::output_buffer_interleaved<S>::commit(u32 samples)
{
	m_write_position += samples;
}

template<typename S> void emu::detail::output_buffer_interleaved<S>::sync()
{
	m_sync_sample += m_write_position - m_sync_position;
	m_sync_position = m_write_position;
}


template<typename S> void emu::detail::output_buffer_interleaved<S>::set_history(u32 history)
{
	m_history = history;
	if(m_sync_position < m_history) {
		u32 delta = m_history - m_sync_position;
		if(m_write_position) {
			std::copy_backward(m_buffer.begin(), m_buffer.begin() + m_write_position * m_channels, m_buffer.begin() + (m_write_position + delta) * m_channels);
			for(u32 pos = m_channels; pos != delta * m_channels; pos++)
				m_buffer[pos] = m_buffer[pos - m_channels];
		} else
			std::fill(m_buffer.begin(), m_buffer.begin() + m_history * m_channels, 0.0);

		m_write_position += delta;
		m_sync_position = m_history;
	}
}

template<typename S> emu::detail::output_buffer_flat<S>::output_buffer_flat(u32 buffer_size, u32 channels) :
	m_buffer(channels),
	m_sync_sample(0),
	m_write_position(0),
	m_sync_position(0),
	m_history(0),
	m_channels(channels)
{
	for(auto &b : m_buffer)
		b.resize(buffer_size, 0);
}

template<typename S> void emu::detail::output_buffer_flat<S>::register_save_state(device_t &device, const char *id1, const char *id2)
{
	auto &save = device.machine().save();

	for(unsigned int i=0; i != m_buffer.size(); i++)
		save.save_item(&device, id1, id2, i, NAME(m_buffer[i]));

	save.save_item(&device, id1, id2, 0, NAME(m_sync_sample));
	save.save_item(&device, id1, id2, 0, NAME(m_write_position));
	save.save_item(&device, id1, id2, 0, NAME(m_sync_position));
	save.save_item(&device, id1, id2, 0, NAME(m_history));
}

template<typename S> void emu::detail::output_buffer_flat<S>::set_buffer_size(u32 buffer_size)
{
	for(auto &b : m_buffer)
		b.resize(buffer_size, 0);
}

template<typename S> void emu::detail::output_buffer_flat<S>::prepare_space(u32 samples)
{
	if(!m_channels)
		return;

	//   Check if potential overflow, bring data back up front if needed
	u32 buffer_size = m_buffer[0].size();
	if(m_write_position + samples > buffer_size) {
		u32 source_start = m_sync_position - m_history;
		u32 source_end = m_write_position;
		for(u32 channel = 0; channel != m_channels; channel++)
			std::copy(m_buffer[channel].begin() + source_start, m_buffer[channel].begin() + source_end, m_buffer[channel].begin());
		m_write_position -= source_start;
		m_sync_position = m_history;
	}

	//   Clear the destination range
	u32 fill_start = m_write_position;
	u32 fill_end = m_write_position + samples;
	for(u32 channel = 0; channel != m_channels; channel++)
		std::fill(m_buffer[channel].begin() + fill_start, m_buffer[channel].begin() + fill_end, 0.0);
}

template<typename S> void emu::detail::output_buffer_flat<S>::commit(u32 samples)
{
	m_write_position += samples;
}

template<typename S> void emu::detail::output_buffer_flat<S>::sync()
{
	m_sync_sample += m_write_position - m_sync_position;
	m_sync_position = m_write_position;
}

template<typename S> void emu::detail::output_buffer_flat<S>::set_history(u32 history)
{
	m_history = history;
	if(m_sync_position < m_history) {
		u32 delta = m_history - m_sync_position;
		if(m_write_position)
			for(u32 channel = 0; channel != m_channels; channel++) {
				std::copy_backward(m_buffer[channel].begin(), m_buffer[channel].begin() + m_write_position, m_buffer[channel].begin() + m_write_position + delta);
				std::fill(m_buffer[channel].begin() + 1, m_buffer[channel].begin() + delta, m_buffer[channel][0]);
			}
		else
			for(u32 channel = 0; channel != m_channels; channel++)
				std::fill(m_buffer[channel].begin(), m_buffer[channel].begin() + m_history, 0.0);

		m_write_position += delta;
		m_sync_position = m_history;
	}
}

template<typename S> void emu::detail::output_buffer_flat<S>::resample(u32 previous_rate, u32 next_rate, attotime sync_time, attotime now)
{
	auto si = [](attotime time, u32 rate) -> s64 {
		return time.m_seconds * rate + muldivu_64(time.m_attoseconds, rate, ATTOSECONDS_PER_SECOND);
	};

	if(!m_write_position || !previous_rate) {
		m_sync_position = 0;
		m_sync_sample = si(sync_time, next_rate);
		m_write_position = si(now, next_rate) - m_sync_sample;
		m_history = 0;
		for(u32 channel = 0; channel != m_channels; channel++)
			std::fill(m_buffer[channel].begin(), m_buffer[channel].begin() + m_write_position, 0);
		return;
	}

	if(!next_rate) {
		m_write_position = m_sync_position = 0;
		return;
	}

	// Compute what will be the new start, sync and write positions (if it fits)
	s64 nsync = si(sync_time, next_rate);
	s64 nwrite = si(now, next_rate);
	s64 pbase = m_sync_sample - m_sync_position; // Beware, pbase can be negative at startup due to history size
	u64 nbase = (pbase <= 0) ? 0 : muldivupu_64(pbase, next_rate, previous_rate);

	if(nbase > nsync)
		nbase = nsync;

	u32 space = m_buffer[0].size();
	if(nwrite - nbase > space) {
		nbase = nwrite - space;
		if(nbase > nsync)
			fatalerror("Stream buffer too small, can't proceed, rate change %d -> %d, space=%d\n", previous_rate, next_rate, space);
	}

	u64 ppos = muldivu_64(nbase, previous_rate, next_rate);
	if(ppos > pbase + m_write_position)
		fatalerror("Something went very wrong, ppos=%d, pbase=%d, pbase+wp=%d\n", ppos, pbase, pbase + m_write_position);

	double step = double(previous_rate) / double(next_rate);
	double pdec = double(nbase % next_rate) * next_rate / previous_rate;
	pdec -= floor(pdec);
	u32 pindex = ppos - pbase;
	u32 nend = nwrite - nbase;

	// Warning: don't try to be too clever, the m_buffer storage is
	// registered in the save state system, so it must not move or
	// change size

	std::vector<S> copy(m_write_position);
	for(u32 channel = 0; channel != m_channels; channel++) {
		std::copy(m_buffer[channel].begin(), m_buffer[channel].begin() + m_write_position, copy.begin());

		// Interpolate the buffer contents

		for(u32 nindex = 0; nindex != nend; nindex++) {
			u32 pi0 = std::clamp(pindex, 0U, m_write_position - 1);
			u32 pi1 = std::clamp(pindex + 1, 0U, m_write_position - 1);
			m_buffer[channel][nindex] = copy[pi0] * (1-pdec) + copy[pi1] * pdec;

			pdec += step;
			if(pdec >= 1) {
				int s = s32(pdec);
				pindex += s;
				pdec -= s;
			}
		}
	}

	m_sync_sample = nsync;
	m_sync_position = m_sync_sample - nbase;
	m_write_position = nend;

	// history and the associated resizes are taken into account later
}

template class emu::detail::output_buffer_flat<sound_stream::sample_t>;
template class emu::detail::output_buffer_interleaved<s16>;


// Not inline because with the unique_ptr it would require audio_effect in emu.h

sound_manager::effect_step::effect_step(u32 buffer_size, u32 channels) : m_buffer(buffer_size, channels)
{
}


//**// Streams and routes

sound_stream::sound_stream(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_delegate callback, sound_stream_flags flags) :
	m_device(device),
	m_output_buffer(0, outputs),
	m_sample_rate((sample_rate == SAMPLE_RATE_INPUT_ADAPTIVE || sample_rate == SAMPLE_RATE_OUTPUT_ADAPTIVE || sample_rate == SAMPLE_RATE_ADAPTIVE) ? 0 : sample_rate),
	m_input_count(inputs),
	m_output_count(outputs),
	m_input_adaptive(sample_rate == SAMPLE_RATE_INPUT_ADAPTIVE || sample_rate == SAMPLE_RATE_ADAPTIVE),
	m_output_adaptive(sample_rate == SAMPLE_RATE_OUTPUT_ADAPTIVE || sample_rate == SAMPLE_RATE_ADAPTIVE),
	m_synchronous((flags & STREAM_SYNCHRONOUS) != 0),
	m_started(false),
	m_in_update(false),
	m_sync_timer(nullptr),
	m_callback(std::move(callback))
{
	sound_assert(outputs > 0 || inputs > 0);

	// create a name
	m_name = m_device.name();
	m_name += " '";
	m_name += m_device.tag();
	m_name += "'";

	// create an update timer for synchronous streams
	if(synchronous())
		m_sync_timer = m_device.timer_alloc(FUNC(sound_stream::sync_update), this);

	// create the gain vectors
	m_input_channel_gain.resize(m_input_count, 1.0);
	m_output_channel_gain.resize(m_output_count, 1.0);
	m_user_output_channel_gain.resize(m_output_count, 1.0);
	m_user_output_gain = 1.0;
}

sound_stream::~sound_stream()
{
}

void sound_stream::add_bw_route(sound_stream *source, int output, int input, float gain)
{
	m_bw_routes.emplace_back(route_bw(source, output, input, gain));
}

void sound_stream::add_fw_route(sound_stream *target, int input, int output)
{
	m_fw_routes.emplace_back(route_fw(target, input, output));
}

bool sound_stream::set_route_gain(sound_stream *source, int source_channel, int target_channel, float gain)
{
	for(auto &r : m_bw_routes)
		if(r.m_source == source && r.m_output == source_channel && r.m_input == target_channel) {
			r.m_gain = gain;
			return true;
		}
	return false;
}

std::vector<sound_stream *> sound_stream::sources() const
{
	std::vector<sound_stream *> streams;
	for(const route_bw &route : m_bw_routes) {
		sound_stream *stream = route.m_source;
		for(const sound_stream *s : streams)
			if(s == stream)
				goto already;
		streams.push_back(stream);
	already:;
	}
	return streams;
}

std::vector<sound_stream *> sound_stream::targets() const
{
	std::vector<sound_stream *> streams;
	for(const route_fw &route : m_fw_routes) {
		sound_stream *stream = route.m_target;
		for(const sound_stream *s : streams)
			if(s == stream)
				goto already;
		streams.push_back(stream);
	already:;
	}
	return streams;
}

void sound_stream::register_state()
{
	// create a unique tag for saving
	m_state_tag = string_format("%d", m_device.machine().sound().unique_id());
	auto &save = m_device.machine().save();

	save.save_item(&m_device, "stream.sound_stream", m_state_tag.c_str(), 0, NAME(m_sync_time));
	save.save_item(&m_device, "stream.sound_stream", m_state_tag.c_str(), 0, NAME(m_sample_rate));
	if(m_input_count)
		save.save_item(&m_device, "stream.sound_stream", m_state_tag.c_str(), 0, NAME(m_input_channel_gain));
	if(m_output_count)
		save.save_item(&m_device, "stream.sound_stream", m_state_tag.c_str(), 0, NAME(m_output_channel_gain));
	// user gains go to .cfg files, not state files

	m_output_buffer.register_save_state(m_device, "stream.sound_stream.output_buffer", m_state_tag.c_str());

	for(unsigned int i=0; i != m_bw_routes.size(); i++)
		save.save_item(&m_device, "stream.sound_stream", m_state_tag.c_str(), i, m_bw_routes[i].m_gain, "route_gain");
}


void sound_stream::compute_dependants()
{
	m_dependant_streams.clear();
	for(const route_bw &r : m_bw_routes)
		r.m_source->add_dependants(m_dependant_streams);
}

void sound_stream::add_dependants(std::vector<sound_stream *> &deps)
{
	for(const route_bw &r : m_bw_routes)
		r.m_source->add_dependants(deps);
	for(sound_stream *dep : deps)
		if(dep == this)
			return;
	deps.push_back(this);
}


//**// Stream sample rate

void sound_stream::set_sample_rate(u32 new_rate)
{
	m_input_adaptive = m_output_adaptive = false;
	internal_set_sample_rate(new_rate);
}

void sound_stream::internal_set_sample_rate(u32 new_rate)
{
	if(new_rate == m_sample_rate)
		return;

	if(m_started) {
		update();
		m_output_buffer.resample(m_sample_rate, new_rate, m_sync_time, m_device.machine().time());
		m_sample_rate = new_rate;
		for(const route_fw &r : m_fw_routes)
			r.m_target->create_resamplers();
		create_resamplers();
		lookup_history_sizes();

	} else
		m_sample_rate = new_rate;
}

bool sound_stream::try_solving_frequency()
{
	if(frequency_is_solved())
		return false;

	if(input_adaptive() && !output_adaptive()) {
		u32 freq = 0;
		for(const route_bw &r : m_bw_routes) {
			if(!r.m_source->frequency_is_solved())
				return false;
			if(freq < r.m_source->sample_rate())
				freq = r.m_source->sample_rate();
		}
		m_sample_rate = freq;
		return true;

	} else if(output_adaptive() && !input_adaptive()) {
		u32 freq = 0;
		for(const route_fw &r : m_fw_routes) {
			if(!r.m_target->frequency_is_solved())
				return false;
			if(freq < r.m_target->sample_rate())
				freq = r.m_target->sample_rate();
		}
		m_sample_rate = freq;
		return true;

	} else {
		u32 freqbw = 0;
		for(const route_bw &r : m_bw_routes) {
			if(!r.m_source->frequency_is_solved()) {
				freqbw = 0;
				break;
			}
			if(freqbw < r.m_source->sample_rate())
				freqbw = r.m_source->sample_rate();
		}
		u32 freqfw = 0;
		for(const route_fw &r : m_fw_routes) {
			if(!r.m_target->frequency_is_solved()) {
				freqfw = 0;
				break;
			}
			if(freqfw < r.m_target->sample_rate())
				freqfw = r.m_target->sample_rate();
		}
		if(!freqbw && !freqfw)
			return false;

		m_sample_rate = (freqfw > freqbw) ? freqfw : freqbw;
		return true;
	}
}


//**// Stream flow and updates

void sound_stream::init()
{
	// Ensure the buffer size is non-zero, since a stream can be started at any time
	u32 bsize = m_sample_rate ? m_sample_rate : 48000;
	m_input_buffer.resize(m_input_count);
	for(auto &b : m_input_buffer)
		b.resize(bsize);

	m_output_buffer.set_buffer_size(bsize);

	m_samples_to_update = 0;
	m_started = true;
	if(synchronous())
		reprime_sync_timer();
}

u64 sound_stream::get_current_sample_index() const
{
	attotime now = m_device.machine().time();
	return now.m_seconds * m_sample_rate + muldivu_64(now.m_attoseconds, m_sample_rate, ATTOSECONDS_PER_SECOND);
}

void sound_stream::update()
{
	if(!is_active() || m_in_update || m_device.machine().phase() <= machine_phase::RESET)
		return;

	// Find out where we are and how much we have to do
	u64 idx = get_current_sample_index();
	m_samples_to_update = idx - m_output_buffer.write_sample() + 1; // We want to include the current sample, hence the +1

	if(m_samples_to_update > 0) {
		m_in_update = true;

		// If there's anything to do, well, do it, starting with the dependencies
		for(auto &stream : m_dependant_streams)
			stream->update_nodeps();

		do_update();
		m_in_update = false;
	}
	m_samples_to_update = 0;
}

void sound_stream::update_nodeps()
{
	if(!is_active() || m_in_update || m_device.machine().phase() <= machine_phase::RESET)
		return;

	// Find out where we are and how much we have to do
	u64 idx = get_current_sample_index();
	m_samples_to_update = idx - m_output_buffer.write_sample() + 1; // We want to include the current sample, hence the +1

	if(m_samples_to_update > 0) {
		m_in_update = true;

		// If there's anything to do, well, do it
		do_update();
		m_in_update = false;
	}
	m_samples_to_update = 0;
}

void sound_stream::create_resamplers()
{
	if(!is_active()) {
		for(auto &r : m_bw_routes)
			r.m_resampler = nullptr;
		return;
	}

	for(auto &r : m_bw_routes)
		if(r.m_source->is_active() && r.m_source->sample_rate() != m_sample_rate)
			r.m_resampler = m_device.machine().sound().get_resampler(r.m_source->sample_rate(), m_sample_rate);
		else
			r.m_resampler = nullptr;
}

void sound_stream::lookup_history_sizes()
{
	u32 history = 0;
	for(auto &r : m_fw_routes) {
		u32 h = r.m_target->get_history_for_bw_route(this, r.m_output);
		if(h > history)
			history = h;
	}

	m_output_buffer.set_history(history);
}

u32 sound_stream::get_history_for_bw_route(const sound_stream *source, u32 channel) const
{
	u32 history = 0;
	for(auto &r : m_bw_routes)
		if(r.m_source == source && r.m_output == channel && r.m_resampler) {
			u32 h = r.m_resampler->history_size();
			if(h > history)
				history = h;
		}
	return history;
}

void sound_stream::do_update()
{
	// Mix in all the inputs (if any)
	if(m_input_count) {
		for(auto &b : m_input_buffer)
			std::fill(b.begin(), b.begin() + m_samples_to_update, 0.0);
		for(const auto &r : m_bw_routes) {
			if(!r.m_source->is_active())
				continue;

			float gain = r.m_source->m_user_output_gain * r.m_source->m_output_channel_gain[r.m_output] * r.m_source->m_user_output_channel_gain[r.m_output] * r.m_gain * m_input_channel_gain[r.m_input];
			auto &db = m_input_buffer[r.m_input];
			if(r.m_resampler)
				r.m_resampler->apply(r.m_source->m_output_buffer, db, m_output_buffer.write_sample(), r.m_output, gain, m_samples_to_update);

			else {
				const sample_t *sb = r.m_source->m_output_buffer.ptrs(r.m_output, m_output_buffer.write_sample() - r.m_source->m_output_buffer.sync_sample());
				for(u32 i = 0; i != m_samples_to_update; i++)
					db[i] += sb[i] * gain;
			}
		}
	}

	// Prepare the output space (if any)
	m_output_buffer.prepare_space(m_samples_to_update);

	// Call the callback
	m_callback(*this);

	// Update the indexes
	m_output_buffer.commit(m_samples_to_update);
}

void sound_stream::sync(attotime now)
{
	m_sync_time = now;
	m_output_buffer.sync();
}




attotime sound_stream::sample_to_time(u64 index) const
{
	attotime res = attotime::zero;
	res.m_seconds = index / m_sample_rate;
	res.m_attoseconds = muldivupu_64(index % m_sample_rate, ATTOSECONDS_PER_SECOND, m_sample_rate);
	return res;
}


//**// Synchronous stream updating

void sound_stream::reprime_sync_timer()
{
	if(!is_active())
		return;

	u64 next_sample = m_output_buffer.write_sample();
	attotime next_time = sample_to_time(next_sample);
	next_time.m_attoseconds += ATTOSECONDS_PER_NANOSECOND; // Go to the next nanosecond
	m_sync_timer->adjust(next_time - m_device.machine().time());
}

void sound_stream::sync_update(s32)
{
	update();
	reprime_sync_timer();
}


//**// Sound manager and stream allocation
sound_manager::sound_manager(running_machine &machine) :
	m_machine(machine),
	m_update_timer(nullptr),
	m_last_sync_time(attotime::zero),
#ifndef SOUND_DISABLE_THREADING
	m_effects_thread(nullptr),
#endif
	m_effects_done(false),
	m_master_gain(1.0),
	m_muted(0),
	m_nosound_mode(machine.osd().no_sound()),
	m_unique_id(0),
	m_wavfile(),
	m_resampler_type(RESAMPLER_LOFI),
	m_resampler_hq_latency(0.005),
	m_resampler_hq_length(400),
	m_resampler_hq_phases(200)
{
	// register callbacks
	machine.configuration().config_register(
			"mixer",
			configuration_manager::load_delegate(&sound_manager::config_load, this),
			configuration_manager::save_delegate(&sound_manager::config_save, this));
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&sound_manager::pause, this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(&sound_manager::resume, this));
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&sound_manager::reset, this));
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&sound_manager::stop_recording, this));

	// register global states
	machine.save().save_item(NAME(m_last_sync_time));

	// start the periodic update flushing timer
	m_update_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(sound_manager::update), this));
	m_update_timer->adjust(STREAMS_UPDATE_ATTOTIME, 0, STREAMS_UPDATE_ATTOTIME);

	// mark the generation as "just starting, waiting for config loading"
	m_osd_info.m_generation = 0xfffffffe;
}

sound_manager::~sound_manager()
{
#ifndef SOUND_DISABLE_THREADING
	if(m_effects_thread) {
		m_effects_done = true;
		m_effects_condition.notify_all();
		m_effects_thread->join();
		m_effects_thread = nullptr;
	}
#endif
}

sound_stream *sound_manager::stream_alloc(device_t &device, u32 inputs, u32 outputs, u32 sample_rate, stream_update_delegate callback, sound_stream_flags flags)
{
	m_stream_list.push_back(std::make_unique<sound_stream>(device, inputs, outputs, sample_rate, callback, flags));
	return m_stream_list.back().get();
}


//**// Sound system initialization

void sound_manager::before_devices_init()
{
	// Inform the targets of the existence of the routes
	for(device_sound_interface &sound : sound_interface_enumerator(machine().root_device()))
		sound.sound_before_devices_init();

	m_machine.save().register_postload(save_prepost_delegate(FUNC(sound_manager::postload), this));
}

void sound_manager::postload()
{
#ifndef SOUND_DISABLE_THREADING
	std::unique_lock<std::mutex> dlock(m_effects_data_mutex);
#endif
	m_effects_prev_time = m_effects_cur_time = machine().time();
}

void sound_manager::after_devices_init()
{
	// Link all the streams together
	for(device_sound_interface &sound : sound_interface_enumerator(machine().root_device()))
		sound.sound_after_devices_init();

	// Resolve the frequencies
	int need_to_solve = 0;
	for(auto &stream : m_stream_list)
		if(!stream->frequency_is_solved())
			need_to_solve ++;

	while(need_to_solve) {
		int prev_need_to_solve = need_to_solve;
		for(auto &stream : m_stream_list)
			if(!stream->frequency_is_solved() && stream->try_solving_frequency())
				need_to_solve --;
		if(need_to_solve == prev_need_to_solve)
			break;
	}

	if(need_to_solve) {
		u32 def = machine().sample_rate();
		for(auto &stream : m_stream_list)
			if(!stream->frequency_is_solved())
				stream->internal_set_sample_rate(def);
	}

	// Have all streams create their buffers and other initializations
	for(auto &stream : m_stream_list)
		stream->init();

	// Detect loops and order streams for full update at the same time
	//  Check the number of sources for each stream
	std::map<sound_stream *, int> depcounts;
	for(auto &stream : m_stream_list)
		depcounts[stream.get()] = stream->sources().size();

	//  Start from all the ones that don't depend on anything
	std::vector<sound_stream *> ready_streams;
	for(auto &dpc : depcounts)
		if(dpc.second == 0)
			ready_streams.push_back(dpc.first);

	//  Handle all the ready streams in a lifo matter (better for cache when generating sound)
	while(!ready_streams.empty()) {
		sound_stream *stream = ready_streams.back();
		//   add the stream to the update order
		m_ordered_streams.push_back(stream);
		ready_streams.resize(ready_streams.size() - 1);
		//   reduce the depcount for all the streams that depend on the updated stream
		for(sound_stream *target : stream->targets())
			if(!--depcounts[target])
				//   when the depcount is zero, a stream is ready to be updated
				ready_streams.push_back(target);
	}

	//  If not all streams ended up in the sorted list, we have a loop
	if(m_ordered_streams.size() != m_stream_list.size()) {
		//  Apply the same algorithm from the other side to the
		//  remaining streams to only keep the ones in the loop

		std::map<sound_stream *, int> inverted_depcounts;
		for(auto &dpc : depcounts)
			if(dpc.second)
				inverted_depcounts[dpc.first] = dpc.first->targets().size();
		for(auto &dpc : inverted_depcounts)
			if(dpc.second == 0)
				ready_streams.push_back(dpc.first);
		while(!ready_streams.empty()) {
			sound_stream *stream = ready_streams.back();
			ready_streams.resize(ready_streams.size() - 1);
			for(sound_stream *source : stream->sources())
				if(!--inverted_depcounts[source])
					ready_streams.push_back(source);
		}
		std::string stream_names;
		for(auto &dpc : inverted_depcounts)
			if(dpc.second)
				stream_names += ' ' + dpc.first->name();
		fatalerror("Loop detected in stream routes:%s", stream_names);
	}

	if(VERBOSE & LOG_ORDER) {
		LOG_OUTPUT_FUNC("Order:\n");
		for(sound_stream *s : m_ordered_streams)
			LOG_OUTPUT_FUNC("- %s (%d)\n", s->name().c_str(), s->sample_rate());
	}

	// Registrations for state saving
	for(auto &stream : m_stream_list)
		stream->register_state();

	// Compute all the per-stream orders for update()
	for(auto &stream : m_stream_list)
		stream->compute_dependants();

	// Create the default effect chain
	for(u32 effect = 0; effect != audio_effect::COUNT; effect++)
		m_default_effects.emplace_back(audio_effect::create(effect, machine().sample_rate(), nullptr));

	// Inventory speakers and microphones
	m_outputs_count = 0;
	for(speaker_device &dev : speaker_device_enumerator(machine().root_device())) {
		dev.set_id(m_speakers.size());
		m_speakers.emplace_back(speaker_info(dev, machine().sample_rate(), m_outputs_count));
		for(u32 effect = 0; effect != audio_effect::COUNT; effect++)
			m_speakers.back().m_effects[effect].m_effect.reset(audio_effect::create(effect, machine().sample_rate(), m_default_effects[effect].get()));
		m_outputs_count += dev.inputs();
	}

	for(microphone_device &dev : microphone_device_enumerator(machine().root_device())) {
		dev.set_id(m_microphones.size());
		m_microphones.emplace_back(microphone_info(dev));
	}

	// Allocate the buffer to pass for recording
	m_record_buffer.resize(m_outputs_count * machine().sample_rate(), 0);
	m_record_samples = 0;

	// Create resamplers and setup history
	rebuild_all_resamplers();

	m_effects_done = false;

#ifndef SOUND_DISABLE_THREADING
	if(m_nosound_mode)
		m_effects_thread = nullptr;
	else
		m_effects_thread = std::make_unique<std::thread>([this]{ run_effects(); });
#endif
}


//**// Effects, input and output management

void sound_manager::input_get(int id, sound_stream &stream)
{
	u32 dest_samples = stream.samples();
	u64 dest_start_pos = stream.start_index();
	u64 dest_end_pos = dest_start_pos + dest_samples;
	u32 skip = stream.output_count();


	for(const auto &step : m_microphones[id].m_input_mixing_steps) {
		if(step.m_mode == mixing_step::CLEAR || step.m_mode == mixing_step::COPY)
				fatalerror("Impossible step encountered in input\n");

		auto &istream = m_osd_input_streams[step.m_osd_index];

		u64 source_start_pos;
		u64 source_end_pos;
		if(!istream.m_resampler) {
			source_start_pos = dest_start_pos;
			source_end_pos = dest_end_pos;
		} else {
			source_start_pos = muldivu_64(dest_start_pos, istream.m_rate, machine().sample_rate());
			source_end_pos = muldivu_64(dest_end_pos, istream.m_rate, machine().sample_rate());
		}

		if(istream.m_buffer.write_sample() < source_end_pos) {
			u32 needed = source_end_pos - istream.m_buffer.write_sample();
			istream.m_buffer.prepare_space(needed);
			machine().osd().sound_stream_source_update(istream.m_id, istream.m_buffer.ptrw(0, 0), needed);
			istream.m_buffer.commit(needed);
		}

		if(istream.m_resampler) {
			istream.m_resampler->apply(istream.m_buffer, stream, step.m_osd_channel, step.m_device_channel, step.m_linear_volume);

		} else {
			const s16 *src = istream.m_buffer.ptrs(step.m_osd_channel, source_start_pos - istream.m_buffer.sync_sample());
			float gain = step.m_linear_volume / 32768.0;
			for(u32 sample = 0; sample != dest_samples; sample++) {
				stream.add(step.m_device_channel, sample, *src * gain);
				src += skip;
			}
		}
	}
}

void sound_manager::output_push(int id, sound_stream &stream)
{
	auto &spk = m_speakers[id];
	auto &out = spk.m_buffer;
	auto &inp = stream.m_input_buffer;
	int samples = stream.samples();
	int channels = stream.input_count();
	out.prepare_space(samples);
	for(int channel = 0; channel != channels; channel ++)
		std::copy(inp[channel].begin(), inp[channel].begin() + samples, out.ptrw(channel, 0));
	out.commit(samples);

	m_record_samples = samples;
	s16 *outb = m_record_buffer.data() + spk.m_first_output;
	for(int channel = 0; channel != channels; channel ++) {
		s16 *outb1 = outb;
		const float *inb = inp[channel].data();
		for(int sample = 0; sample != samples; sample++) {
			*outb1 = std::clamp(int(*inb++ * 32768), -32768, 32767);
			outb1 += m_outputs_count;
		}
		outb++;
	}
}

void sound_manager::run_effects()
{
#ifndef SOUND_DISABLE_THREADING
	std::unique_lock<std::mutex> dlock(m_effects_data_mutex);
	for(;;) {
		m_effects_condition.wait(dlock);
		if(m_effects_done)
			return;

		std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
		// Copy the data to the effects threads, expanding as needed
		// when -speed is in use
		double sf = machine().video().speed_factor();
		if(sf == 1000) {
			for(auto &si : m_speakers) {
				int samples = si.m_buffer.available_samples();
				int channels = si.m_buffer.channels();
				auto &eb = si.m_effects_buffer;
				eb.prepare_space(samples);
				if(m_muted)
					for(int channel = 0; channel != channels; channel ++)
						std::fill(si.m_effects_buffer.ptrw(channel, 0), si.m_effects_buffer.ptrw(channel, 0) + samples, 0);
				else
					for(int channel = 0; channel != channels; channel ++)
						std::copy(si.m_buffer.ptrs(channel, 0), si.m_buffer.ptrs(channel, 0) + samples, eb.ptrw(channel, 0));
				eb.commit(samples);
			}
		} else {
			sf /= 1000;
			for(auto &si : m_speakers) {
				int source_samples = si.m_buffer.available_samples();
				int channels = si.m_buffer.channels();
				auto &eb = si.m_effects_buffer;
				eb.prepare_space(source_samples / sf + 1);
				int source_sample_index = 0;
				int dest_index = 0;
				double m_phase = si.m_speed_phase;
				for(int channel = 0; channel != channels; channel ++) {
					const sample_t *src = si.m_buffer.ptrs(channel, 0);
					m_phase = si.m_speed_phase;
					if(m_phase >= 1) {
						source_sample_index = int(m_phase);
						m_phase -= int(m_phase);
					} else
						source_sample_index = 0;
					sample_t *dest = eb.ptrw(channel, 0);
					dest_index = 0;
					while(source_sample_index < source_samples) {
						dest[dest_index++] = m_muted ? 0.0 : src[source_sample_index];
						m_phase += sf;
						if(m_phase >= 1) {
							source_sample_index += int(m_phase);
							m_phase -= int(m_phase);
						}
					}
				}
				si.m_speed_phase = m_phase + (source_sample_index - source_samples);
				eb.commit(dest_index);
			}
		}
#ifndef SOUND_DISABLE_THREADING
		dlock.unlock();
#endif

		// Apply the effects
		for(auto &si : m_speakers)
			for(u32 i=0; i != si.m_effects.size(); i++) {
				auto &source = i ? si.m_effects[i-1].m_buffer : si.m_effects_buffer;
				si.m_effects[i].m_effect->apply(source, si.m_effects[i].m_buffer);
				source.sync();
			}

		// Apply the mixing steps
		for(const auto &step : m_output_mixing_steps) {
			if(step.m_mode == mixing_step::CLEAR)
				continue;

			const auto &eb = m_speakers[step.m_device_index].m_effects.back().m_buffer;
			auto &ostream = m_osd_output_streams[step.m_osd_index];
			u32 source_samples = m_speakers[step.m_device_index].m_effects.back().m_buffer.available_samples();

			if(ostream.m_resampler) {
				u64 start_sync = muldivu_64(eb.sync_sample(), ostream.m_rate, machine().sample_rate());
				u64 end_sync = muldivu_64(eb.sync_sample() + source_samples, ostream.m_rate, machine().sample_rate());
				ostream.m_samples = end_sync - start_sync;
				switch(step.m_mode) {
				case mixing_step::COPY: {
					ostream.m_resampler->apply_copy(eb, ostream.m_buffer, step.m_osd_channel, ostream.m_channels, start_sync, step.m_device_channel, step.m_linear_volume * m_master_gain, ostream.m_samples);
					break;
				}

				case mixing_step::ADD: {
					ostream.m_resampler->apply_add(eb, ostream.m_buffer, step.m_osd_channel, ostream.m_channels, start_sync, step.m_device_channel, step.m_linear_volume * m_master_gain, ostream.m_samples);
					break;
				}
				}

			} else {
				ostream.m_samples = source_samples;
				const sample_t *src = eb.ptrs(step.m_device_channel, 0);
				s16 *dest = ostream.m_buffer.data() + step.m_osd_channel;
				u32 skip = ostream.m_channels;
				switch(step.m_mode) {
				case mixing_step::COPY: {
					float gain = 32768 * step.m_linear_volume * m_master_gain;
					for(u32 sample = 0; sample != source_samples; sample++) {
						*dest = std::clamp(int(*src++ * gain), -32768, 32767);
						dest += skip;
					}
					break;
				}

				case mixing_step::ADD: {
					float gain = 32768 * step.m_linear_volume * m_master_gain;
					for(u32 sample = 0; sample != source_samples; sample++) {
						*dest = std::clamp(int(*src++ * gain) + *dest, -32768, 32767);
						dest += skip;
					}
					break;
				}
				}
			}
		}

		for(const auto &step : m_output_mixing_steps)
			if(step.m_mode == mixing_step::CLEAR) {
				auto &ostream = m_osd_output_streams[step.m_osd_index];
				s16 *dest = ostream.m_buffer.data() + step.m_osd_channel;
				u32 skip = ostream.m_channels;
				for(u32 sample = 0; sample != ostream.m_samples; sample++) {
					*dest = 0;
					dest += skip;
				}
			}

		for(auto &si : m_speakers)
			si.m_effects.back().m_buffer.sync();

		machine().osd().sound_begin_update();

		// Send the result to the osd
		for(auto &stream : m_osd_output_streams)
			if(stream.m_samples)
				machine().osd().sound_stream_sink_update(stream.m_id, stream.m_buffer.data(), stream.m_samples);

		machine().osd().sound_end_update();
#ifndef SOUND_DISABLE_THREADING

		dlock.lock();
	}
#endif
}

std::string sound_manager::effect_chain_tag(s32 index) const
{
	return m_speakers[index].m_dev.tag();
}

std::vector<audio_effect *> sound_manager::effect_chain(s32 index) const
{
	std::vector<audio_effect *> res;
	for(const auto &e : m_speakers[index].m_effects)
		res.push_back(e.m_effect.get());
	return res;
}

std::vector<audio_effect *> sound_manager::default_effect_chain() const
{
	std::vector<audio_effect *> res;
	for(const auto &e : m_default_effects)
		res.push_back(e.get());
	return res;
}

void sound_manager::default_effect_changed(u32 entry)
{
	u32 type = m_default_effects[entry]->type();
	for(const auto &s : m_speakers)
		for(const auto &e : s.m_effects)
			if(e.m_effect->type() == type)
				e.m_effect->default_changed();
}




//-------------------------------------------------
//  start_recording - begin audio recording
//-------------------------------------------------

bool sound_manager::start_recording(std::string_view filename)
{
	if(m_wavfile)
		return false;
	m_wavfile = util::wav_open(filename, machine().sample_rate(), m_outputs_count);
	return bool(m_wavfile);
}

bool sound_manager::start_recording()
{
	// open the output WAV file if specified
	char const *const filename = machine().options().wav_write();
	return *filename ? start_recording(filename) : false;
}


//-------------------------------------------------
//  stop_recording - end audio recording
//-------------------------------------------------

void sound_manager::stop_recording()
{
	// close any open WAV file
	m_wavfile.reset();
}


//-------------------------------------------------
//  mute - mute sound output
//-------------------------------------------------

void sound_manager::mute(bool mute, u8 reason)
{
#ifndef SOUND_DISABLE_THREADING
	std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
	if(mute)
		m_muted |= reason;
	else
		m_muted &= ~reason;
}


//-------------------------------------------------
//  reset - reset all sound chips
//-------------------------------------------------

sound_manager::speaker_info::speaker_info(speaker_device &dev, u32 rate, u32 first_output) : m_dev(dev), m_first_output(first_output), m_speed_phase(0), m_buffer(rate, dev.inputs()), m_effects_buffer(rate, dev.inputs())
{
	m_channels = dev.inputs();
	m_stream = dev.stream();
	for(u32 i=0; i != audio_effect::COUNT; i++)
		m_effects.emplace_back(effect_step(rate, dev.inputs()));
}

sound_manager::microphone_info::microphone_info(microphone_device &dev) : m_dev(dev)
{
	m_channels = dev.outputs();
}

void sound_manager::reset()
{
	if(VERBOSE & LOG_GENERAL)
		LOG_OUTPUT_FUNC("Sound reset\n");
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


//**// Configuration management

void sound_manager::config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	if(cfg_type == config_type::FINAL)
		// Note that the config is loaded
		m_osd_info.m_generation = 0xffffffff;

	// If no config file, ignore
	if(!parentnode)
		return;

	switch(cfg_type) {
	case config_type::INIT:
		break;

	case config_type::CONTROLLER:
		break;

	case config_type::DEFAULT: {
		// In the global config, get the default effect chain configuration

		util::xml::data_node const *efl_node = parentnode->get_child("audio_effects");
		if(efl_node) {
			for(util::xml::data_node const *ef_node = efl_node->get_child("effect"); ef_node != nullptr; ef_node = ef_node->get_next_sibling("effect")) {
				unsigned int id = ef_node->get_attribute_int("step", 0);
				std::string type = ef_node->get_attribute_string("type", "");
				if(id >= 1 && id <= m_default_effects.size() && audio_effect::effect_names[m_default_effects[id-1]->type()] == type) {
					m_default_effects[id-1]->config_load(ef_node);
					default_effect_changed(id-1);
				}
			}
		}

		// and the resampler configuration
		util::xml::data_node const *rs_node = parentnode->get_child("resampler");
		if(rs_node) {
			m_resampler_hq_latency = rs_node->get_attribute_float("hq_latency", 0.0050);
			m_resampler_hq_length = rs_node->get_attribute_int("hq_length", 400);
			m_resampler_hq_phases = rs_node->get_attribute_int("hq_phases", 200);

			// this also applies the hq settings if resampler is hq
			set_resampler_type(rs_node->get_attribute_int("type", RESAMPLER_LOFI));
		}
		break;
	}

	case config_type::SYSTEM: {
		// In the per-driver file, get the specific configuration for everything

		// Effects configuration
		for(util::xml::data_node const *efl_node = parentnode->get_child("audio_effects"); efl_node != nullptr; efl_node = efl_node->get_next_sibling("audio_effects")) {
			std::string speaker_tag = efl_node->get_attribute_string("tag", "");
			for(auto &speaker : m_speakers)
				if(speaker.m_dev.tag() == speaker_tag) {
					auto &eff = speaker.m_effects;
					for(util::xml::data_node const *ef_node = efl_node->get_child("effect"); ef_node != nullptr; ef_node = ef_node->get_next_sibling("effect")) {
						unsigned int id = ef_node->get_attribute_int("step", 0);
						std::string type = ef_node->get_attribute_string("type", "");
						if(id >= 1 && id <= m_default_effects.size() && audio_effect::effect_names[eff[id-1].m_effect->type()] == type)
							eff[id-1].m_effect->config_load(ef_node);
					}
					break;
				}
		}

		// All levels
		if(!machine().options().volume()) {
			const util::xml::data_node *lv_node = parentnode->get_child("master_volume");
			if(lv_node)
				m_master_gain = lv_node->get_attribute_float("gain", 1.0);
		}
		else
			m_master_gain = osd::db_to_linear(machine().options().volume());

		for(const util::xml::data_node *lv_node = parentnode->get_child("device_volume"); lv_node != nullptr; lv_node = lv_node->get_next_sibling("device_volume")) {
			std::string device_tag = lv_node->get_attribute_string("device", "");
			device_sound_interface *intf = dynamic_cast<device_sound_interface *>(m_machine.root_device().subdevice(device_tag));
			if(intf)
				intf->set_user_output_gain(lv_node->get_attribute_float("gain", 1.0));
		}

		for(const util::xml::data_node *lv_node = parentnode->get_child("device_channel_volume"); lv_node != nullptr; lv_node = lv_node->get_next_sibling("device_channel_volume")) {
			std::string device_tag = lv_node->get_attribute_string("device", "");
			int channel = lv_node->get_attribute_int("channel", -1);
			device_sound_interface *intf = dynamic_cast<device_sound_interface *>(m_machine.root_device().subdevice(device_tag));
			if(intf && channel >= 0 && channel < intf->outputs())
				intf->set_user_output_gain(channel, lv_node->get_attribute_float("gain", 1.0));
		}


		// Mapping configuration
		m_configs.clear();
		for(util::xml::data_node const *node = parentnode->get_child("sound_map"); node != nullptr; node = node->get_next_sibling("sound_map")) {
			m_configs.emplace_back(config_mapping { node->get_attribute_string("tag", "") });
			auto &config = m_configs.back();
			for(util::xml::data_node const *nmap = node->get_child("node_mapping"); nmap != nullptr; nmap = nmap->get_next_sibling("node_mapping"))
				config.m_node_mappings.emplace_back(std::pair<std::string, float>(nmap->get_attribute_string("node", ""), nmap->get_attribute_float("db", 0)));
			for(util::xml::data_node const *cmap = node->get_child("channel_mapping"); cmap != nullptr; cmap = cmap->get_next_sibling("channel_mapping"))
				config.m_channel_mappings.emplace_back(std::tuple<u32, std::string, u32, float>(cmap->get_attribute_int("guest_channel", 0),
																								cmap->get_attribute_string("node", ""),
																								cmap->get_attribute_int("node_channel", 0),
																								cmap->get_attribute_float("db", 0)));
		}
		break;
	}

	case config_type::FINAL:
		break;
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void sound_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	switch(cfg_type) {
	case config_type::INIT:
		break;

	case config_type::CONTROLLER:
		break;

	case config_type::DEFAULT: {
		// In the global config, save the default effect chain configuration
		util::xml::data_node *const efl_node = parentnode->add_child("audio_effects", nullptr);
		for(u32 ei = 0; ei != m_default_effects.size(); ei++) {
			const audio_effect *e = m_default_effects[ei].get();
			util::xml::data_node *const ef_node = efl_node->add_child("effect", nullptr);
			ef_node->set_attribute_int("step", ei+1);
			ef_node->set_attribute("type", audio_effect::effect_names[e->type()]);
			e->config_save(ef_node);
		}

		util::xml::data_node *const rs_node = parentnode->add_child("resampler", nullptr);
		rs_node->set_attribute_int("type", m_resampler_type);
		rs_node->set_attribute_float("hq_latency", m_resampler_hq_latency);
		rs_node->set_attribute_int("hq_length", m_resampler_hq_length);
		rs_node->set_attribute_int("hq_phases", m_resampler_hq_phases);
		break;
	}

	case config_type::SYSTEM: {
		// In the per-driver file, save the specific configuration for everything

		// Effects configuration
		for(const auto &speaker : m_speakers) {
			util::xml::data_node *const efl_node = parentnode->add_child("audio_effects", nullptr);
			efl_node->set_attribute("tag", speaker.m_dev.tag());
			for(u32 ei = 0; ei != speaker.m_effects.size(); ei++) {
				const audio_effect *e = speaker.m_effects[ei].m_effect.get();
				util::xml::data_node *const ef_node = efl_node->add_child("effect", nullptr);
				ef_node->set_attribute_int("step", ei+1);
				ef_node->set_attribute("type", audio_effect::effect_names[e->type()]);
				e->config_save(ef_node);
			}
		}

		// All levels
		if(m_master_gain != 1.0 && m_master_gain != osd::db_to_linear(machine().options().volume())) {
			util::xml::data_node *const lv_node = parentnode->add_child("master_volume", nullptr);
			lv_node->set_attribute_float("gain", m_master_gain);
		}
		for(device_sound_interface &snd : sound_interface_enumerator(m_machine.root_device())) {
			// Don't add microphones, speakers or devices without outputs
			if(dynamic_cast<sound_io_device *>(&snd) || !snd.outputs())
				continue;
			if(snd.user_output_gain() != 1.0) {
				util::xml::data_node *const lv_node = parentnode->add_child("device_volume", nullptr);
				lv_node->set_attribute("device", snd.device().tag());
				lv_node->set_attribute_float("gain", snd.user_output_gain());
			}
			for(int channel = 0; channel != snd.outputs(); channel ++)
				if(snd.user_output_gain(channel) != 1.0) {
					util::xml::data_node *const lv_node = parentnode->add_child("device_channel_volume", nullptr);
					lv_node->set_attribute("device", snd.device().tag());
					lv_node->set_attribute_int("channel", channel);
					lv_node->set_attribute_float("gain", snd.user_output_gain(channel));
				}
		}

		// Mapping configuration
		auto output_one = [this, parentnode](sound_io_device &dev) {
			for(const auto &config : m_configs)
				if(config.m_name == dev.tag()) {
					util::xml::data_node *const sp_node = parentnode->add_child("sound_map", nullptr);
					sp_node->set_attribute("tag", dev.tag());
					for(const auto &nmap : config.m_node_mappings) {
						util::xml::data_node *const node = sp_node->add_child("node_mapping", nullptr);
						node->set_attribute("node", nmap.first.c_str());
						node->set_attribute_float("db", nmap.second);
					}
					for(const auto &cmap : config.m_channel_mappings) {
						util::xml::data_node *const node = sp_node->add_child("channel_mapping", nullptr);
						node->set_attribute_int("guest_channel", std::get<0>(cmap));
						node->set_attribute("node", std::get<1>(cmap).c_str());
						node->set_attribute_int("node_channel", std::get<2>(cmap));
						node->set_attribute_float("db", std::get<3>(cmap));
					}
					return;
				}
		};

		for(auto &spk : m_speakers)
			output_one(spk.m_dev);
		for(auto &mic : m_microphones)
			output_one(mic.m_dev);
		break;
	}

	case config_type::FINAL:
		break;
	}
}



//**// Mapping between speakers/microphones and OSD endpoints

sound_manager::config_mapping &sound_manager::config_get_sound_io(sound_io_device *dev)
{
	for(auto &config : m_configs)
		if(config.m_name == dev->tag())
			return config;
	m_configs.emplace_back(config_mapping { dev->tag() });
	return m_configs.back();
}

void sound_manager::config_add_sound_io_connection_node(sound_io_device *dev, std::string name, float db)
{
	internal_config_add_sound_io_connection_node(dev, name, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_add_sound_io_connection_node(sound_io_device *dev, std::string name, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &nmap : config.m_node_mappings)
		if(nmap.first == name)
			return;
	config.m_node_mappings.emplace_back(std::pair<std::string, float>(name, db));
}

void sound_manager::config_add_sound_io_connection_default(sound_io_device *dev, float db)
{
	internal_config_add_sound_io_connection_default(dev, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_add_sound_io_connection_default(sound_io_device *dev, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &nmap : config.m_node_mappings)
		if(nmap.first == "")
			return;
	config.m_node_mappings.emplace_back(std::pair<std::string, float>("", db));
}

void sound_manager::config_remove_sound_io_connection_node(sound_io_device *dev, std::string name)
{
	internal_config_remove_sound_io_connection_node(dev, name);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_remove_sound_io_connection_node(sound_io_device *dev, std::string name)
{
	auto &config = config_get_sound_io(dev);
	for(auto i = config.m_node_mappings.begin(); i != config.m_node_mappings.end(); i++)
		if(i->first == name) {
			config.m_node_mappings.erase(i);
			return;
		}
}

void sound_manager::config_remove_sound_io_connection_default(sound_io_device *dev)
{
	internal_config_remove_sound_io_connection_default(dev);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_remove_sound_io_connection_default(sound_io_device *dev)
{
	auto &config = config_get_sound_io(dev);
	for(auto i = config.m_node_mappings.begin(); i != config.m_node_mappings.end(); i++)
		if(i->first == "") {
			config.m_node_mappings.erase(i);
			return;
		}
}

void sound_manager::config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string name, float db)
{
	internal_config_set_volume_sound_io_connection_node(dev, name, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_set_volume_sound_io_connection_node(sound_io_device *dev, std::string name, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &nmap : config.m_node_mappings)
		if(nmap.first == name) {
			nmap.second = db;
			return;
		}
}

void sound_manager::config_set_volume_sound_io_connection_default(sound_io_device *dev, float db)
{
	internal_config_set_volume_sound_io_connection_default(dev, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_set_volume_sound_io_connection_default(sound_io_device *dev, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &nmap : config.m_node_mappings)
		if(nmap.first == "") {
			nmap.second = db;
			return;
		}
}


void sound_manager::config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db)
{
	internal_config_add_sound_io_channel_connection_node(dev, guest_channel, name, node_channel, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_add_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &cmap : config.m_channel_mappings)
		if(std::get<0>(cmap) == guest_channel && std::get<1>(cmap) == name && std::get<2>(cmap) == node_channel)
			return;
	config.m_channel_mappings.emplace_back(std::tuple<u32, std::string, u32, float>(guest_channel, name, node_channel, db));
}

void sound_manager::config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db)
{
	internal_config_add_sound_io_channel_connection_default(dev, guest_channel, node_channel, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_add_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &cmap : config.m_channel_mappings)
		if(std::get<0>(cmap) == guest_channel && std::get<1>(cmap) == "" && std::get<2>(cmap) == node_channel)
			return;
	config.m_channel_mappings.emplace_back(std::tuple<u32, std::string, u32, float>(guest_channel, "", node_channel, db));
}

void sound_manager::config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel)
{
	internal_config_remove_sound_io_channel_connection_node(dev, guest_channel, name, node_channel);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_remove_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel)
{
	auto &config = config_get_sound_io(dev);
	for(auto i = config.m_channel_mappings.begin(); i != config.m_channel_mappings.end(); i++)
		if(std::get<0>(*i) == guest_channel && std::get<1>(*i) == name && std::get<2>(*i) == node_channel) {
			config.m_channel_mappings.erase(i);
			return;
		}
}

void sound_manager::config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel)
{
	internal_config_remove_sound_io_channel_connection_default(dev, guest_channel, node_channel);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_remove_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel)
{
	auto &config = config_get_sound_io(dev);
	for(auto i = config.m_channel_mappings.begin(); i != config.m_channel_mappings.end(); i++)
		if(std::get<0>(*i) == guest_channel && std::get<1>(*i) == "" && std::get<2>(*i) == node_channel) {
			config.m_channel_mappings.erase(i);
			return;
		}
}

void sound_manager::config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db)
{
	internal_config_set_volume_sound_io_channel_connection_node(dev, guest_channel, name, node_channel, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_set_volume_sound_io_channel_connection_node(sound_io_device *dev, u32 guest_channel, std::string name, u32 node_channel, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &cmap : config.m_channel_mappings)
		if(std::get<0>(cmap) == guest_channel && std::get<1>(cmap) == name && std::get<2>(cmap) == node_channel) {
			std::get<3>(cmap) = db;
			return;
		}
}

void sound_manager::config_set_volume_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db)
{
	internal_config_set_volume_sound_io_channel_connection_default(dev, guest_channel, node_channel, db);
	m_osd_info.m_generation --;
	mapping_update();
}

void sound_manager::internal_config_set_volume_sound_io_channel_connection_default(sound_io_device *dev, u32 guest_channel, u32 node_channel, float db)
{
	auto &config = config_get_sound_io(dev);
	for(auto &cmap : config.m_channel_mappings)
		if(std::get<0>(cmap) == guest_channel && std::get<1>(cmap) == "" && std::get<2>(cmap) == node_channel) {
			std::get<3>(cmap) = db;
			return;
		}
}

void sound_manager::startup_cleanups()
{
	auto osd_info = machine().osd().sound_get_information();

	// for every sound_io device that does not have a configuration entry, add a
	// mapping to default
	auto default_one = [this](sound_io_device &dev) {
		for(const auto &config : m_configs)
			if(config.m_name == dev.tag())
				return;
		m_configs.emplace_back(config_mapping { dev.tag() });
		m_configs.back().m_node_mappings.emplace_back(std::pair<std::string, float>("", 0.0));
	};

	for(sound_io_device &dev : speaker_device_enumerator(machine().root_device()))
		default_one(dev);
	for(sound_io_device &dev : microphone_device_enumerator(machine().root_device()))
		default_one(dev);

	// If there's no default sink replace all the default sink config
	// entries into the first sink available
	if(!osd_info.m_default_sink) {
		std::string first_sink_name;
		for(const auto &node : osd_info.m_nodes)
			if(node.m_sinks) {
				first_sink_name = node.name();
				break;
			}

		if(first_sink_name != "")
			for(auto &config : m_configs) {
				for(auto &nmap : config.m_node_mappings)
					if(nmap.first == "")
						nmap.first = first_sink_name;
				for(auto &cmap : config.m_channel_mappings)
					if(std::get<1>(cmap) == "")
						std::get<1>(cmap) = first_sink_name;
			}
	}


	// If there's no default source replace all the default source config
	// entries into the first source available
	if(!osd_info.m_default_source) {
		std::string first_source_name;
		for(const auto &node : osd_info.m_nodes)
			if(node.m_sources) {
				first_source_name = node.name();
				break;
			}

		if(first_source_name != "")
			for(auto &config : m_configs) {
				for(auto &nmap : config.m_node_mappings)
					if(nmap.first == "")
						nmap.first = first_source_name;
				for(auto &cmap : config.m_channel_mappings)
					if(std::get<1>(cmap) == "")
						std::get<1>(cmap) = first_source_name;
			}
	}
}

template<bool is_output, typename S> void sound_manager::apply_osd_changes(std::vector<S> &streams)
{
	// Apply host system volume and routing changes to the internal structures
	for(S &stream : streams) {
		u32 sidx;
		for(sidx = 0; sidx != m_osd_info.m_streams.size() && m_osd_info.m_streams[sidx].m_id != stream.m_id; sidx++);
		// If the stream has been lost, mark it lost and continue.  It will be cleared in update_osd_streams.
		if(sidx == m_osd_info.m_streams.size()) {
			stream.m_id = 0;
			continue;
		}

		// Check if the target and/or the volumes changed
		bool node_changed = stream.m_node != m_osd_info.m_streams[sidx].m_node;
		bool volume_changed = !std::equal(stream.m_volumes.begin(), stream.m_volumes.end(), m_osd_info.m_streams[sidx].m_volumes.begin(), m_osd_info.m_streams[sidx].m_volumes.end());

		if(node_changed || volume_changed) {
			// Check if a node change is just tracking the system default
			bool system_default_tracking = node_changed && stream.m_is_system_default && m_osd_info.m_streams[sidx].m_node == (is_output ? m_osd_info.m_default_sink : m_osd_info.m_default_source);

			// Find the config entry for the sound_io
			config_mapping *config = nullptr;
			for(auto &conf : m_configs)
				if(conf.m_name == stream.m_dev->tag()) {
					config = &conf;
					break;
				}
			if(!config)
				continue;

			// Retrieve the old node name, and, if it's different, the new node name
			std::string old_node_name = stream.m_node_name;
			std::string new_node_name;
			if(node_changed) {
				for(const auto &node : m_osd_info.m_nodes)
					if(node.m_id == m_osd_info.m_streams[sidx].m_node) {
						new_node_name = node.name();
						break;
					}
				// That's really, really not supposed to happen
				if(new_node_name.empty())
					continue;
			} else
				new_node_name = old_node_name;

			// Separate the cases on full mapping vs. channel mapping
			if(!stream.m_is_channel_mapping) {
				// Full mapping
				// Find the index of the config mapping entry that generated the stream, if there's still one.
				// Note that a default system stream has the empty string as a name
				u32 index;
				for(index = 0; index != config->m_node_mappings.size(); index++)
					if(config->m_node_mappings[index].first == old_node_name)
						break;
				if(index == config->m_node_mappings.size())
					continue;

				// If the target node changed, write it down
				if(node_changed) {
					if(!system_default_tracking) {
						config->m_node_mappings[index].first = new_node_name;
						stream.m_node_name = new_node_name;
						stream.m_is_system_default = false;
					}
					stream.m_node = m_osd_info.m_streams[sidx].m_node;
				}

				// If the volume changed, there are two
				// possibilities: either the channels split, or
				// they didn't.
				if(volume_changed) {
					// Check is all the channel volumes are the same
					float new_volume = m_osd_info.m_streams[sidx].m_volumes[0];
					bool same = true;
					for(u32 i = 1; i != m_osd_info.m_streams[sidx].m_volumes.size(); i++)
						if(m_osd_info.m_streams[sidx].m_volumes[i] != new_volume) {
							same = false;
							break;
						}
					if(same) {
						// All the same volume, just note down the new volume
						stream.m_volumes = m_osd_info.m_streams[sidx].m_volumes;
						config->m_node_mappings[index].second = new_volume;

					} else {
						const osd::audio_info::node_info *node = nullptr;
						for(const auto &n : m_osd_info.m_nodes)
							if(n.m_id == stream.m_node) {
								node = &n;
								break;
							}
						for(u32 channel = 0; channel != stream.m_channels; channel++) {
							std::vector<u32> targets = find_channel_mapping(stream.m_dev->get_position(channel), node);
							for(u32 tchannel : targets)
								if(stream.m_node_name == "")
									internal_config_add_sound_io_channel_connection_default(stream.m_dev, channel, tchannel, m_osd_info.m_streams[sidx].m_volumes[tchannel]);
								else
									internal_config_add_sound_io_channel_connection_node(stream.m_dev, channel, stream.m_node_name, tchannel, m_osd_info.m_streams[sidx].m_volumes[tchannel]);
						}
						config->m_node_mappings.erase(config->m_node_mappings.begin() + index);
					}
				}
			} else {
				// Channel mapping
				for(u32 channel = 0; channel != stream.m_channels; channel++) {
					if(stream.m_unused_channels_mask & (1 << channel))
						continue;

					// Find the index of the config mapping entry that generated the stream channel, if there's still one.
					// Note that a default system stream has the empty string as a name
					u32 index;
					for(index = 0; index != config->m_channel_mappings.size(); index++)
						if(std::get<1>(config->m_channel_mappings[index]) == old_node_name &&
						   std::get<2>(config->m_channel_mappings[index]) == channel)
							break;
					if(index == config->m_channel_mappings.size())
						continue;

					// If the target node changed, write it down
					if(node_changed) {
						if(!system_default_tracking) {
							std::get<1>(config->m_channel_mappings[index]) = new_node_name;
							stream.m_node_name = new_node_name;
							stream.m_is_system_default = false;
						}
						stream.m_node = m_osd_info.m_streams[sidx].m_node;
					}

					// If the volume changed, write in down too
					if(volume_changed) {
						std::get<3>(config->m_channel_mappings[index]) = m_osd_info.m_streams[sidx].m_volumes[channel];
						stream.m_volumes[channel] = m_osd_info.m_streams[sidx].m_volumes[channel];
					}
				}
			}
		}
	}
}

void sound_manager::osd_information_update()
{
	// Get a snapshot of the current information
	m_osd_info = machine().osd().sound_get_information();

	// Analyze the streams to see if anything changed, but only in the
	// split stream case.
	if(machine().osd().sound_split_streams_per_source()) {
		apply_osd_changes<false, osd_input_stream >(m_osd_input_streams );
		apply_osd_changes<true,  osd_output_stream>(m_osd_output_streams);
	}
}

void sound_manager::generate_mapping()
{
	auto find_node = [this](std::string name) -> u32 {
		for(const auto &node : m_osd_info.m_nodes)
			if(node.name() == name)
				return node.m_id;
		return 0;
	};

	m_mappings.clear();
	for(speaker_info &speaker : m_speakers) {
		auto &config = config_get_sound_io(&speaker.m_dev);
		m_mappings.emplace_back(mapping { &speaker.m_dev });
		auto &omap = m_mappings.back();

		std::vector<std::string> node_to_remove;
		for(auto &nmap : config.m_node_mappings) {
			if(nmap.first == "") {
				if(m_osd_info.m_default_sink)
					omap.m_node_mappings.emplace_back(mapping::node_mapping { m_osd_info.m_default_sink, nmap.second, true });
			} else {
				u32 node_id = find_node(nmap.first);
				if(node_id != 0)
					omap.m_node_mappings.emplace_back(mapping::node_mapping { node_id, nmap.second, false });
				else
					node_to_remove.push_back(nmap.first);
			}
		}

		for(auto &nmap: node_to_remove)
			internal_config_remove_sound_io_connection_node(&speaker.m_dev, nmap);

		std::vector<std::tuple<u32, std::string, u32>> channel_map_to_remove;
		for(auto &cmap : config.m_channel_mappings) {
			if(std::get<1>(cmap) == "") {
				if(m_osd_info.m_default_sink)
					omap.m_channel_mappings.emplace_back(mapping::channel_mapping { std::get<0>(cmap), m_osd_info.m_default_sink, std::get<2>(cmap), std::get<3>(cmap), true });
			} else {
				u32 node_id = find_node(std::get<1>(cmap));
				if(node_id != 0)
					omap.m_channel_mappings.emplace_back(mapping::channel_mapping { std::get<0>(cmap), node_id, std::get<2>(cmap), std::get<3>(cmap), false });
				else
					channel_map_to_remove.push_back(std::tuple<u32, std::string, u32>(std::get<0>(cmap), std::get<1>(cmap), std::get<2>(cmap)));
			}
		}

		for(auto &cmap : channel_map_to_remove)
			internal_config_remove_sound_io_channel_connection_node(&speaker.m_dev, std::get<0>(cmap), std::get<1>(cmap), std::get<2>(cmap));
	}

	for(microphone_info &mic : m_microphones) {
		auto &config = config_get_sound_io(&mic.m_dev);
		m_mappings.emplace_back(mapping { &mic.m_dev });
		auto &omap = m_mappings.back();

		std::vector<std::string> node_to_remove;
		for(auto &nmap : config.m_node_mappings) {
			if(nmap.first == "") {
				if(m_osd_info.m_default_source)
					omap.m_node_mappings.emplace_back(mapping::node_mapping { m_osd_info.m_default_source, nmap.second, true });
			} else {
				u32 node_id = find_node(nmap.first);
				if(node_id != 0)
					omap.m_node_mappings.emplace_back(mapping::node_mapping { node_id, nmap.second, false });
				else
					node_to_remove.push_back(nmap.first);
			}
		}

		for(auto &nmap: node_to_remove)
			internal_config_remove_sound_io_connection_node(&mic.m_dev, nmap);

		std::vector<std::tuple<u32, std::string, u32>> channel_map_to_remove;
		for(auto &cmap : config.m_channel_mappings) {
			if(std::get<1>(cmap) == "") {
				if(m_osd_info.m_default_source)
					omap.m_channel_mappings.emplace_back(mapping::channel_mapping { std::get<0>(cmap), m_osd_info.m_default_source, std::get<2>(cmap), std::get<3>(cmap), true });
			} else {
				u32 node_id = find_node(std::get<1>(cmap));
				if(node_id != 0)
					omap.m_channel_mappings.emplace_back(mapping::channel_mapping { std::get<0>(cmap), node_id, std::get<2>(cmap), std::get<3>(cmap), false });
				else
					channel_map_to_remove.push_back(std::tuple<u32, std::string, u32>(std::get<0>(cmap), std::get<1>(cmap), std::get<2>(cmap)));
			}
		}

		for(auto &cmap : channel_map_to_remove)
			internal_config_remove_sound_io_channel_connection_node(&mic.m_dev, std::get<0>(cmap), std::get<1>(cmap), std::get<2>(cmap));
	}
}

// Find where to map a sound_io channel into a node's channels depending on their positions

std::vector<u32> sound_manager::find_channel_mapping(const std::array<double, 3> &position, const osd::audio_info::node_info *node)
{
	std::vector<u32> result;
	if(position[0] == 0 && position[1] == 0 && position[2] == 0)
		return result;
	double best_dist = -1;
	for(u32 port = 0; port != node->m_port_positions.size(); port++)
		if(sound_io_device::mapping_allowed(node->m_port_positions[port])) {
			double dx = position[0] - node->m_port_positions[port][0];
			double dy = position[1] - node->m_port_positions[port][1];
			double dz = position[2] - node->m_port_positions[port][2];
			double dist = dx*dx + dy*dy + dz*dz;
			if(best_dist == -1 || dist < best_dist) {
				best_dist = dist;
				result.clear();
				result.push_back(port);
			} else if(best_dist == dist)
				result.push_back(port);
		}
	return result;
}


void sound_manager::update_osd_streams()
{
#ifndef SOUND_DISABLE_THREADING
	std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
	auto current_input_streams = std::move(m_osd_input_streams);
	auto current_output_streams = std::move(m_osd_output_streams);
	m_osd_input_streams.clear();
	m_osd_output_streams.clear();

	// Find the index of a sound_io_device in the speaker_info vector or the microphone_info vector

	auto find_sound_io_index = [this](sound_io_device *dev) -> u32 {
		for(u32 si = 0; si != m_speakers.size(); si++)
			if(&m_speakers[si].m_dev == dev)
				return si;
		for(u32 si = 0; si != m_microphones.size(); si++)
			if(&m_microphones[si].m_dev == dev)
				return si;
		return 0; // Can't happen
	};


	// Find a pointer to a node_info from the node id
	auto find_node_info = [this](u32 node) -> const osd::audio_info::node_info * {
		for(const auto &ni : m_osd_info.m_nodes) {
			if(ni.m_id == node)
				return &ni;
		}
		// Can't happen
		return nullptr;
	};

	// Two possible mapping methods depending on the osd capabilities

	for(auto &m : m_microphones)
		m.m_input_mixing_steps.clear();
	m_output_mixing_steps.clear();

	auto &osd = machine().osd();
	if(osd.sound_split_streams_per_source()) {
		auto get_input_stream_for_node_and_device = [this, &current_input_streams] (const osd::audio_info::node_info *node, sound_io_device *dev, u32 rate, bool is_system_default, bool is_channel_mapping = false) -> u32 {
			// Check if the osd stream already exists to pick it up in case.
			// Clear the id in the current_streams structure to show it has been picked up, reset the unused mask.
			// Clear the volumes
			// m_dev will already be correct

			for(auto &os : current_input_streams)
				if(os.m_id && os.m_node == node->m_id && os.m_dev == dev && os.m_rate == rate) {
					u32 sid = m_osd_input_streams.size();
					m_osd_input_streams.emplace_back(std::move(os));
					os.m_id = 0;
					auto &nos = m_osd_input_streams[sid];
					nos.m_is_channel_mapping = is_channel_mapping;
					nos.m_unused_channels_mask = util::make_bitmask<u32>(node->m_sources);
					nos.m_volumes.clear();
					nos.m_is_system_default = is_system_default;
					return sid;
				}

			// If none exists, create one
			u32 sid = m_osd_input_streams.size();
			m_osd_input_streams.emplace_back(osd_input_stream(node->m_id, is_system_default ? "" : node->m_name, node->m_sources, rate, is_system_default, dev));
			osd_input_stream &nos = m_osd_input_streams.back();
			nos.m_id = machine().osd().sound_stream_source_open(node->m_id, dev->tag(), rate);
			nos.m_is_channel_mapping = is_channel_mapping;
			nos.m_buffer.set_sync_sample(rate_and_last_sync_to_index(rate));
			return sid;
		};

		auto get_output_stream_for_node_and_device = [this, &current_output_streams] (const osd::audio_info::node_info *node, sound_io_device *dev, u32 rate, bool is_system_default, bool is_channel_mapping = false) -> u32 {
			// Check if the osd stream already exists to pick it up in case.
			// Clear the id in the current_streams structure to show it has been picked up, reset the unused mask.
			// Clear the volumes
			// m_dev will already be correct

			for(auto &os : current_output_streams)
				if(os.m_id && os.m_node == node->m_id && os.m_dev == dev) {
					u32 sid = m_osd_output_streams.size();
					m_osd_output_streams.emplace_back(std::move(os));
					os.m_id = 0;
					auto &nos = m_osd_output_streams[sid];
					nos.m_is_channel_mapping = is_channel_mapping;
					nos.m_volumes.clear();
					nos.m_unused_channels_mask = util::make_bitmask<u32>(node->m_sinks);
					nos.m_is_system_default = is_system_default;
					return sid;
				}

			// If none exists, create one
			u32 sid = m_osd_output_streams.size();
			m_osd_output_streams.emplace_back(osd_output_stream(node->m_id, is_system_default ? "" : node->m_name, node->m_sinks, rate, is_system_default, dev));
			osd_output_stream &nos = m_osd_output_streams.back();
			nos.m_id = machine().osd().sound_stream_sink_open(node->m_id, dev->tag(), rate);
			nos.m_is_channel_mapping = is_channel_mapping;
			return sid;
		};

		auto get_input_stream_for_node_and_channel = [this, &get_input_stream_for_node_and_device] (const osd::audio_info::node_info *node, u32 node_channel, sound_io_device *dev, u32 rate, bool is_system_default) -> u32 {
			// First check if there's an active stream
			for(u32 sid = 0; sid != m_osd_input_streams.size(); sid++) {
				auto &os = m_osd_input_streams[sid];
				if(os.m_node == node->m_id && os.m_dev == dev && os.m_rate == rate && os.m_unused_channels_mask & (1 << node_channel) && os.m_is_channel_mapping)
					return sid;
			}

			// Otherwise use the default method
			return get_input_stream_for_node_and_device(node, dev, rate, is_system_default, true);
		};


		auto get_output_stream_for_node_and_channel = [this, &get_output_stream_for_node_and_device] (const osd::audio_info::node_info *node, u32 node_channel, sound_io_device *dev, u32 rate, bool is_system_default) -> u32 {
			// First check if there's an active stream with the correct channel not used yet
			for(u32 sid = 0; sid != m_osd_output_streams.size(); sid++) {
				auto &os = m_osd_output_streams[sid];
				if(os.m_node == node->m_id && os.m_dev == dev && os.m_rate == rate && os.m_unused_channels_mask & (1 << node_channel) && os.m_is_channel_mapping)
					return sid;
			}

			// Otherwise use the default method
			return get_output_stream_for_node_and_device(node, dev, rate, is_system_default, true);
		};

		// Create/retrieve streams to apply the decided mapping
		for(const auto &omap : m_mappings) {
			u32 dev_index = find_sound_io_index(omap.m_dev);
			bool is_output = omap.m_dev->is_output();
			if(is_output) {
				std::vector<mixing_step> &mixing_steps = m_output_mixing_steps;
				u32 dchannels = omap.m_dev->inputs();
				for(const auto &nm : omap.m_node_mappings) {
					const auto *node = find_node_info(nm.m_node);
					u32 osd_index = get_output_stream_for_node_and_device(node, omap.m_dev, node->resolve_rate(machine().sample_rate()), nm.m_is_system_default);
					auto &stream = m_osd_output_streams[osd_index];
					u32 umask = stream.m_unused_channels_mask;
					float linear_volume = 1.0;

					if(osd.sound_external_per_channel_volume()) {
						stream.m_volumes.clear();
						stream.m_volumes.resize(stream.m_channels, nm.m_db);

					} else
						linear_volume = osd::db_to_linear(nm.m_db);

					for(u32 channel = 0; channel != dchannels; channel++) {
						std::vector<u32> targets = find_channel_mapping(omap.m_dev->get_position(channel), node);
						for(u32 tchannel : targets) {
							// If the channel is output and in the to
							// clear mask, use load, otherwise use add.
							// Apply the volume too if needed
							mixing_steps.emplace_back(mixing_step {
														  (umask & (1 << tchannel)) ? mixing_step::COPY : mixing_step::ADD,
															  osd_index,
															  tchannel,
															  dev_index,
															  channel,
															  linear_volume
															  });
							umask &= ~(1 << tchannel);
						}
					}
					stream.m_unused_channels_mask = umask;
				}

				for(const auto &cm : omap.m_channel_mappings) {
					const auto *node = find_node_info(cm.m_node);
					u32 osd_index = get_output_stream_for_node_and_channel(node, cm.m_node_channel, omap.m_dev, node->resolve_rate(machine().sample_rate()), cm.m_is_system_default);
					auto &stream = m_osd_output_streams[osd_index];
					float linear_volume = 1.0;

					if(osd.sound_external_per_channel_volume()) {
						if(stream.m_volumes.empty())
							stream.m_volumes.resize(stream.m_channels, -96);
						stream.m_volumes[cm.m_node_channel] = cm.m_db;

					} else
						linear_volume = osd::db_to_linear(cm.m_db);

					mixing_steps.emplace_back(mixing_step {
												  (stream.m_unused_channels_mask & (1 << cm.m_node_channel)) ?
													  mixing_step::COPY : mixing_step::ADD,
													  osd_index,
													  cm.m_node_channel,
													  dev_index,
													  cm.m_guest_channel,
													  linear_volume
													  });
					stream.m_unused_channels_mask &= ~(1 << cm.m_node_channel);
				}


			} else {
				std::vector<mixing_step> &mixing_steps = m_microphones[dev_index].m_input_mixing_steps;
				u32 dchannels = omap.m_dev->outputs();
				for(const auto &nm : omap.m_node_mappings) {
					const auto *node = find_node_info(nm.m_node);
					u32 osd_index = get_input_stream_for_node_and_device(node, omap.m_dev, node->resolve_rate(machine().sample_rate()), nm.m_is_system_default);
					auto &stream = m_osd_input_streams[osd_index];
					u32 umask = stream.m_unused_channels_mask;
					float linear_volume = 1.0;

					if(osd.sound_external_per_channel_volume()) {
						stream.m_volumes.clear();
						stream.m_volumes.resize(stream.m_channels, nm.m_db);

					} else
						linear_volume = osd::db_to_linear(nm.m_db);

					for(u32 channel = 0; channel != dchannels; channel++) {
						std::vector<u32> targets = find_channel_mapping(omap.m_dev->get_position(channel), node);
						for(u32 tchannel : targets) {
							// If the channel is output and in the to
							// clear mask, use load, otherwise use add.
							// Apply the volume too if needed
							mixing_steps.emplace_back(mixing_step {
														  mixing_step::ADD,
															  osd_index,
															  tchannel,
															  dev_index,
															  channel,
															  linear_volume
															  });
							umask &= ~(1 << tchannel);
						}
					}
					stream.m_unused_channels_mask = umask;
				}

				for(const auto &cm : omap.m_channel_mappings) {
					const auto *node = find_node_info(cm.m_node);
					u32 osd_index = get_input_stream_for_node_and_channel(node, cm.m_node_channel, omap.m_dev, node->resolve_rate(machine().sample_rate()), cm.m_is_system_default);
					auto &stream = m_osd_input_streams[osd_index];
					float linear_volume = 1.0;

					if(osd.sound_external_per_channel_volume()) {
						if(stream.m_volumes.empty())
							stream.m_volumes.resize(stream.m_channels, -96);
						stream.m_volumes[cm.m_node_channel] = cm.m_db;

					} else
						linear_volume = osd::db_to_linear(cm.m_db);

					mixing_steps.emplace_back(mixing_step {
												  mixing_step::ADD,
													  osd_index,
													  cm.m_node_channel,
													  dev_index,
													  cm.m_guest_channel,
													  linear_volume
													  });
					stream.m_unused_channels_mask &= ~(1 << cm.m_node_channel);
				}
			}
		}

	} else {
		// All sources need to be merged per-destination, max one stream per destination

		std::map<u32, u32> input_stream_per_node, output_stream_per_node;

		// Retrieve or create the one osd stream for a given
		// destination.  First check if we already have it, then
		// whether it was previously created, then otherwise create
		// it.

		auto get_input_stream_for_node = [this, &current_input_streams, &input_stream_per_node] (const osd::audio_info::node_info *node, u32 rate, bool is_system_default) -> u32 {
			// Pick up the existing stream if there's one
			auto si = input_stream_per_node.find(node->m_id);
			if(si != input_stream_per_node.end())
				return si->second;

			// Create the default unused mask
			u32 channels = node->m_sources;
			u32 umask = util::make_bitmask<u32>(channels);

			// Check if the osd stream already exists to pick it up in case.
			// Clear the id in the current_streams structure to show it has been picked up, reset the unused mask.
			// m_speaker will already be nullptr, m_source_channels and m_volumes empty.
			// Do not change the rate, obviously.

			for(auto &os : current_input_streams)
				if(os.m_id && os.m_node == node->m_id) {
					u32 sid = m_osd_input_streams.size();
					m_osd_input_streams.emplace_back(std::move(os));
					os.m_id = 0;
					m_osd_input_streams.back().m_unused_channels_mask = umask;
					m_osd_input_streams.back().m_is_system_default = is_system_default;
					input_stream_per_node[node->m_id] = sid;
					return sid;
				}

			// If none exists, create one
			u32 sid = m_osd_input_streams.size();
			m_osd_input_streams.emplace_back(osd_input_stream(node->m_id, is_system_default ? "" : node->m_name, channels, rate, is_system_default, nullptr));
			osd_input_stream &stream = m_osd_input_streams.back();
			stream.m_id = machine().osd().sound_stream_source_open(node->m_id, machine().system().name, rate);
			stream.m_buffer.set_sync_sample(rate_and_last_sync_to_index(rate));
			input_stream_per_node[node->m_id] = sid;
			return sid;
		};

		auto get_output_stream_for_node = [this, &current_output_streams, &output_stream_per_node] (const osd::audio_info::node_info *node, u32 rate, bool is_system_default) -> u32 {
			// Pick up the existing stream if there's one
			auto si = output_stream_per_node.find(node->m_id);
			if(si != output_stream_per_node.end())
				return si->second;

			// Create the default unused mask
			u32 channels = node->m_sinks;
			u32 umask = util::make_bitmask<u32>(channels);

			// Check if the osd stream already exists to pick it up in case.
			// Clear the id in the current_streams structure to show it has been picked up, reset the unused mask.
			// m_speaker will already be nullptr, m_source_channels and m_volumes empty.
			// Do not change the rate, obviously.

			for(auto &os : current_output_streams)
				if(os.m_id && os.m_node == node->m_id) {
					u32 sid = m_osd_output_streams.size();
					m_osd_output_streams.emplace_back(std::move(os));
					os.m_id = 0;
					m_osd_output_streams.back().m_unused_channels_mask = umask;
					m_osd_output_streams.back().m_is_system_default = is_system_default;
					output_stream_per_node[node->m_id] = sid;
					return sid;
				}

			// If none exists, create one
			u32 sid = m_osd_output_streams.size();
			m_osd_output_streams.emplace_back(osd_output_stream(node->m_id, is_system_default ? "" : node->m_name, channels, rate, is_system_default, nullptr));
			osd_output_stream &stream = m_osd_output_streams.back();
			stream.m_id = machine().osd().sound_stream_sink_open(node->m_id, machine().system().name, rate);
			output_stream_per_node[node->m_id] = sid;
			return sid;
		};


		// Create/retrieve streams to apply the decided mapping

		for(const auto &omap : m_mappings) {
			u32 dev_index = find_sound_io_index(omap.m_dev);
			bool is_output = omap.m_dev->is_output();
			if(is_output) {
				u32 channels = m_speakers[dev_index].m_channels;
				std::vector<mixing_step> &mixing_steps = m_output_mixing_steps;
				for(const auto &nm : omap.m_node_mappings) {
					const auto *node = find_node_info(nm.m_node);
					u32 osd_index = get_output_stream_for_node(node, node->resolve_rate(machine().sample_rate()), nm.m_is_system_default);
					u32 umask = m_osd_output_streams[osd_index].m_unused_channels_mask;
					float linear_volume = osd::db_to_linear(nm.m_db);

					for(u32 channel = 0; channel != channels; channel++) {
						std::vector<u32> targets = find_channel_mapping(omap.m_dev->get_position(channel), node);
						for(u32 tchannel : targets) {
							// If the channel is in the to clear mask, use load, otherwise use add
							// Apply the volume too
							mixing_steps.emplace_back(mixing_step {
														  (umask & (1 << tchannel)) ? mixing_step::COPY : mixing_step::ADD,
															  osd_index,
															  tchannel,
															  dev_index,
															  channel,
															  linear_volume
															  });
							umask &= ~(1 << tchannel);
						}
					}
					m_osd_output_streams[osd_index].m_unused_channels_mask = umask;
				}

				for(const auto &cm : omap.m_channel_mappings) {
					const auto *node = find_node_info(cm.m_node);
					u32 osd_index = get_output_stream_for_node(node, node->resolve_rate(machine().sample_rate()), false);
					u32 umask = m_osd_output_streams[osd_index].m_unused_channels_mask;

					// If the channel is in the to clear mask, use load, otherwise use add
					// Apply the volume too
					mixing_steps.emplace_back(mixing_step {
												  (umask & (1 << cm.m_node_channel)) ? mixing_step::COPY : mixing_step::ADD,
													  osd_index,
													  cm.m_node_channel,
													  dev_index,
													  cm.m_guest_channel,
													  osd::db_to_linear(cm.m_db)
													  });
					m_osd_output_streams[osd_index].m_unused_channels_mask = umask & ~(1 << cm.m_node_channel);
				}

			} else {
				u32 channels = m_microphones[dev_index].m_channels;
				std::vector<mixing_step> &mixing_steps = m_microphones[dev_index].m_input_mixing_steps;
				for(const auto &nm : omap.m_node_mappings) {
					const auto *node = find_node_info(nm.m_node);
					u32 osd_index = get_input_stream_for_node(node, node->resolve_rate(machine().sample_rate()), nm.m_is_system_default);
					float linear_volume = osd::db_to_linear(nm.m_db);

					for(u32 channel = 0; channel != channels; channel++) {
						std::vector<u32> targets = find_channel_mapping(omap.m_dev->get_position(channel), node);
						for(u32 tchannel : targets) {
							// If the channel is in the to clear mask, use load, otherwise use add
							// Apply the volume too
							mixing_steps.emplace_back(mixing_step {
														  mixing_step::ADD,
															  osd_index,
															  tchannel,
															  dev_index,
															  channel,
															  linear_volume
															  });
							m_osd_input_streams[osd_index].m_unused_channels_mask &= ~(1 << tchannel);
						}
					}
				}

				for(const auto &cm : omap.m_channel_mappings) {
					const auto *node = find_node_info(cm.m_node);
					u32 osd_index = get_input_stream_for_node(node, node->resolve_rate(machine().sample_rate()), false);

					// If the channel is in the to clear mask, use load, otherwise use add
					// Apply the volume too
					mixing_steps.emplace_back(mixing_step {
												  mixing_step::ADD,
													  osd_index,
													  cm.m_node_channel,
													  dev_index,
													  cm.m_guest_channel,
													  osd::db_to_linear(cm.m_db)
													  });
					m_osd_input_streams[osd_index].m_unused_channels_mask &= ~(1 << cm.m_node_channel);
				}
			}
		}
	}

	// Add a clear step for all output streams that need it
	// Also set the volumes if supported
	for(u32 stream_index = 0; stream_index != m_osd_output_streams.size(); stream_index++) {
		auto &stream = m_osd_output_streams[stream_index];
		if(stream.m_unused_channels_mask) {
			for(u32 channel = 0; channel != stream.m_channels; channel ++)
				if(stream.m_unused_channels_mask & (1 << channel))
					m_output_mixing_steps.emplace_back(mixing_step { mixing_step::CLEAR, stream_index, channel, 0, 0, 0.0 });
		}
		if(!stream.m_volumes.empty())
			osd.sound_stream_set_volumes(stream.m_id, stream.m_volumes);
	}

	// If supported, set the volumes for the input streams
	for(u32 stream_index = 0; stream_index != m_osd_input_streams.size(); stream_index++) {
		auto &stream = m_osd_input_streams[stream_index];
		if(!stream.m_volumes.empty())
			osd.sound_stream_set_volumes(stream.m_id, stream.m_volumes);
	}

	// Close all previous streams that haven't been picked up
	for(const auto &stream : current_input_streams)
		if(stream.m_id)
			machine().osd().sound_stream_close(stream.m_id);
	for(const auto &stream : current_output_streams)
		if(stream.m_id)
			machine().osd().sound_stream_close(stream.m_id);

	rebuild_all_stream_resamplers();
}

void sound_manager::mapping_update()
{
	// fffffffe means the config is not loaded yet, so too early
	// ffffffff means the config is loaded but the defaults are not setup yet
	if(m_nosound_mode)
		return;

	if(m_osd_info.m_generation == 0xfffffffe)
		return;
	if(m_osd_info.m_generation == 0xffffffff)
		startup_cleanups();

	auto &osd = machine().osd();
	while(m_osd_info.m_generation != osd.sound_get_generation()) {
		osd_information_update();

		if(VERBOSE & LOG_OSD_INFO) {
			LOG_OUTPUT_FUNC("OSD information:\n");
			LOG_OUTPUT_FUNC("- generation %u\n", m_osd_info.m_generation);
			LOG_OUTPUT_FUNC("- default sink %u\n", m_osd_info.m_default_sink);
			LOG_OUTPUT_FUNC("- default source %u\n", m_osd_info.m_default_source);
			LOG_OUTPUT_FUNC("- nodes:\n");
			for(const auto &node : m_osd_info.m_nodes) {
				LOG_OUTPUT_FUNC("  * %3u %s [%d %d-%d]\n", node.m_id, node.name().c_str(), node.m_rate.m_default_rate, node.m_rate.m_min_rate, node.m_rate.m_max_rate);
				uint32_t port_count = node.m_sinks;
				if(port_count < node.m_sources)
					port_count = node.m_sources;
				for(uint32_t port = 0; port != port_count; port++)
					LOG_OUTPUT_FUNC("      %s %s [%g %g %g]\n",
							(port < node.m_sinks) ? ((port < node.m_sources) ? "<>" : ">") : "<",
							node.m_port_names[port],
							node.m_port_positions[port][0],
							node.m_port_positions[port][1],
							node.m_port_positions[port][2]);
			}
			LOG_OUTPUT_FUNC("- streams:\n");
			for(const auto &stream : m_osd_info.m_streams) {
				LOG_OUTPUT_FUNC("  * %3u node %u", stream.m_id, stream.m_node);
				if(!stream.m_volumes.empty()) {
					LOG_OUTPUT_FUNC(" volumes");
					for(float v : stream.m_volumes)
						LOG_OUTPUT_FUNC(" %g", v);
				}
				LOG_OUTPUT_FUNC("\n");
			}
		}

		generate_mapping();

		if(VERBOSE & LOG_MAPPING) {
			LOG_OUTPUT_FUNC("MAPPING:\n");
			for(const auto &omap : m_mappings) {
				LOG_OUTPUT_FUNC("- sound_io %s\n", omap.m_dev->tag());
				for(const auto &nm : omap.m_node_mappings)
					LOG_OUTPUT_FUNC("  * node %u volume %g%s\n", nm.m_node, nm.m_db, nm.m_is_system_default ? " (default)" : "");
				for(const auto &cm : omap.m_channel_mappings)
					LOG_OUTPUT_FUNC("  * channel %u <-> node %u:%i volume %g\n", cm.m_guest_channel, cm.m_node, cm.m_node_channel, cm.m_db);
			}
		}

		update_osd_streams();

		if(VERBOSE & LOG_OSD_STREAMS) {
			LOG_OUTPUT_FUNC("OSD input streams:\n");
			for(const auto &os : m_osd_input_streams) {
				if(machine().osd().sound_split_streams_per_source()) {
					LOG_OUTPUT_FUNC("- %3u %s node %u", os.m_id, os.m_dev ? os.m_dev->tag() : "-", os.m_node);
					if(!os.m_is_channel_mapping)
						LOG_OUTPUT_FUNC(" channels");
					if(machine().osd().sound_external_per_channel_volume()) {
						LOG_OUTPUT_FUNC(" dB");
						for(u32 i = 0; i != os.m_channels; i++)
							LOG_OUTPUT_FUNC(" %g", os.m_volumes[i]);
					}
					LOG_OUTPUT_FUNC("\n");
				} else
					LOG_OUTPUT_FUNC("- %3u node %u\n", os.m_id, os.m_node);
			}
			LOG_OUTPUT_FUNC("Input mixing steps:\n");
			for(const auto &m : m_microphones) {
				LOG_OUTPUT_FUNC("  %s:\n", m.m_dev.tag());
				for(const auto &ms : m.m_input_mixing_steps) {
					static const char *const modes[5] = { "clear", "copy", "add" };
					LOG_OUTPUT_FUNC("  - %s osd %u:%u -> device %u:%u level %g\n", modes[ms.m_mode], ms.m_osd_index, ms.m_osd_channel, ms.m_device_index, ms.m_device_channel, ms.m_linear_volume);
				}
			}
			LOG_OUTPUT_FUNC("OSD output streams:\n");
			for(const auto &os : m_osd_output_streams) {
				if(machine().osd().sound_split_streams_per_source()) {
					LOG_OUTPUT_FUNC("- %3u %s node %u", os.m_id, os.m_dev ? os.m_dev->tag() : "-", os.m_node);
					if(!os.m_is_channel_mapping)
						LOG_OUTPUT_FUNC(" channels");
					if(machine().osd().sound_external_per_channel_volume()) {
						LOG_OUTPUT_FUNC(" dB");
						for(u32 i = 0; i != os.m_channels; i++)
							LOG_OUTPUT_FUNC(" %g", os.m_volumes[i]);
					}
					LOG_OUTPUT_FUNC("\n");
				} else
					LOG_OUTPUT_FUNC("- %3u node %u\n", os.m_id, os.m_node);
			}
			LOG_OUTPUT_FUNC("Output mixing steps:\n");
			for(const auto &ms : m_output_mixing_steps) {
				static const char *const modes[5] = { "clear", "copy", "add" };
				LOG_OUTPUT_FUNC("- %s device %u:%u -> osd %u:%u level %g\n", modes[ms.m_mode], ms.m_device_index, ms.m_device_channel, ms.m_osd_index, ms.m_osd_channel, ms.m_linear_volume);
			}
		}
	}
}




//**// Global sound system update

u64 sound_manager::rate_and_time_to_index(attotime time, u32 sample_rate) const
{
	return time.m_seconds * sample_rate + muldivu_64(time.m_attoseconds, sample_rate,  ATTOSECONDS_PER_SECOND);
}

void sound_manager::update(s32)
{
	auto profile = g_profiler.start(PROFILER_SOUND);

	mapping_update();
	streams_update();

	m_last_sync_time = machine().time();
}

void sound_manager::streams_update()
{
	attotime now = machine().time();
	{
#ifndef SOUND_DISABLE_THREADING
		std::unique_lock<std::mutex> dlock(m_effects_data_mutex);
#endif
		for(auto &si : m_speakers)
			si.m_buffer.sync();

		m_effects_prev_time = m_effects_cur_time;
		m_effects_cur_time = now;

		for(sound_stream *stream : m_ordered_streams)
			stream->update_nodeps();
#ifndef SOUND_DISABLE_THREADING
		m_effects_condition.notify_all();
#else
		run_effects();
#endif
	}


	// Send the hooked samples to lua
	{
		std::map<std::string, std::vector<std::pair<const float *, int>>> sound_data;
		for(device_sound_interface &sound : sound_interface_enumerator(machine().root_device()))
			if(sound.get_sound_hook()) {
				std::vector<std::pair<const float *, int>> buffers;
				if(sound.device().type() == SPEAKER) {
					const emu::detail::output_buffer_flat<sample_t> &buffer = m_speakers[static_cast<speaker_device &>(sound.device()).get_id()].m_buffer;
					int samples = buffer.available_samples();
					for(int channel = 0; channel != sound.inputs(); channel++)
						buffers.emplace_back(std::make_pair(buffer.ptrs(channel, 0), samples));

				} else {
					for(int channel = 0; channel != sound.outputs(); channel++) {
						std::pair<sound_stream *, int> info = sound.output_to_stream_output(channel);
						const emu::detail::output_buffer_flat<sample_t> &buffer = info.first->m_output_buffer;
						buffers.emplace_back(std::make_pair(buffer.ptrs(info.second, 0), buffer.available_samples()));
					}
				}
				sound_data.emplace(sound.device().tag(), std::move(buffers));
			}

		emulator_info::sound_hook(sound_data);
	}

	for(sound_stream *stream : m_ordered_streams)
		if(stream->device().type() != SPEAKER)
			stream->sync(now);

	for(osd_input_stream &stream : m_osd_input_streams)
		stream.m_buffer.sync();

	machine().osd().add_audio_to_recording(m_record_buffer.data(), m_record_samples);
	machine().video().add_sound_to_recording(m_record_buffer.data(), m_record_samples);
	if(m_wavfile)
		util::wav_add_data_16(*m_wavfile, m_record_buffer.data(), m_record_samples * m_outputs_count);
}

//**// Resampler management
const audio_resampler *sound_manager::get_resampler(u32 fs, u32 ft)
{
	auto key = std::make_pair(fs, ft);
	auto i = m_resamplers.find(key);
	if(i != m_resamplers.end())
		return i->second.get();

	audio_resampler *res;
	if(m_resampler_type == RESAMPLER_HQ)
		res = new audio_resampler_hq(fs, ft, m_resampler_hq_latency, m_resampler_hq_length, m_resampler_hq_phases);
	else
		res = new audio_resampler_lofi(fs, ft);
	m_resamplers[key].reset(res);
	return res;
}

void sound_manager::rebuild_all_stream_resamplers()
{
	u32 edge_rate = machine().sample_rate();
	for(auto &stream : m_osd_input_streams)
		if(stream.m_rate != edge_rate) {
			stream.m_resampler = get_resampler(stream.m_rate, edge_rate);
			stream.m_buffer.set_history(stream.m_resampler->history_size());
		} else {
			stream.m_resampler = nullptr;
			stream.m_buffer.set_history(0);
		}
	for(auto &stream : m_osd_output_streams)
		if(stream.m_rate != edge_rate)
			stream.m_resampler = get_resampler(edge_rate, stream.m_rate);
		else
			stream.m_resampler = nullptr;

	for(auto &spk : m_speakers)
		spk.m_effects.back().m_buffer.set_history(0);

	for(const auto &step : m_output_mixing_steps)
		if(step.m_mode != mixing_step::CLEAR) {
			auto &stream = m_osd_output_streams[step.m_osd_index];
			if(stream.m_resampler)
				m_speakers[step.m_device_index].m_effects.back().m_buffer.set_history(stream.m_resampler->history_size());
		}
}

void sound_manager::rebuild_all_resamplers()
{
	m_resamplers.clear();

	for(auto &stream : m_stream_list)
		stream->create_resamplers();

	for(auto &stream : m_stream_list)
		stream->lookup_history_sizes();
}

void sound_manager::set_resampler_type(u32 type)
{
	if(type != m_resampler_type) {
#ifndef SOUND_DISABLE_THREADING
		std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
		m_resampler_type = type;
		rebuild_all_resamplers();
		rebuild_all_stream_resamplers();
	}
}

void sound_manager::set_resampler_hq_latency(double latency)
{
	if(latency != m_resampler_hq_latency) {
#ifndef SOUND_DISABLE_THREADING
		std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
		m_resampler_hq_latency = latency;
		rebuild_all_resamplers();
		rebuild_all_stream_resamplers();
	}
}

void sound_manager::set_resampler_hq_length(u32 length)
{
	if(length != m_resampler_hq_length) {
#ifndef SOUND_DISABLE_THREADING
		std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
		m_resampler_hq_length = length;
		rebuild_all_resamplers();
		rebuild_all_stream_resamplers();
	}
}

void sound_manager::set_resampler_hq_phases(u32 phases)
{
	if(phases != m_resampler_hq_phases) {
#ifndef SOUND_DISABLE_THREADING
		std::unique_lock<std::mutex> lock(m_effects_mutex);
#endif
		m_resampler_hq_phases = phases;
		rebuild_all_resamplers();
		rebuild_all_stream_resamplers();
	}
}

const char *sound_manager::resampler_type_names(u32 type) const
{
	using util::lang_translate;

	if(type == RESAMPLER_HQ)
		return _("HQ");
	else
		return _("LoFi");
}
