// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    disound.cpp

    Device sound interfaces.

***************************************************************************/

#include "emu.h"

#include "resampler.h"
#include "sound.h"
#include "speaker.h"


//**************************************************************************
//  SOUND STREAMS
//**************************************************************************

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

	// Check if potential overflow, bring data back up front if needed
	u32 buffer_size = m_buffer.size() / m_channels;
	if(m_write_position + samples > buffer_size) {
		u32 source_start = (m_sync_position - m_history) * m_channels;
		u32 source_end = m_write_position * m_channels;
		std::copy(m_buffer.begin() + source_start, m_buffer.begin() + source_end, m_buffer.begin());
		m_write_position -= m_sync_position - m_history;
		m_sync_position = m_history;
	}

	// Clear the destination range
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

	for(unsigned int i = 0; i != m_buffer.size(); i++)
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

	// Check if potential overflow, bring data back up front if needed
	u32 buffer_size = m_buffer[0].size();
	if(m_write_position + samples > buffer_size) {
		u32 source_start = m_sync_position - m_history;
		u32 source_end = m_write_position;
		for(u32 channel = 0; channel != m_channels; channel++)
			std::copy(m_buffer[channel].begin() + source_start, m_buffer[channel].begin() + source_end, m_buffer[channel].begin());
		m_write_position -= source_start;
		m_sync_position = m_history;
	}

	// Clear the destination range
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
	if(inputs == 0 && outputs == 0)
		fatalerror("Device %s requiring to create a stream without inputs or outputs\n", device.tag());

	if(inputs == 0)
		m_input_adaptive = false;
	if(outputs == 0)
		m_output_adaptive = false;

	if(m_sample_rate && m_sample_rate < 1000)
		fatalerror("Device %s requiring to create a stream with too low samplerate %d\n", device.tag(), m_sample_rate);

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
	m_bw_routes.emplace_back(source, output, input, gain);
}

void sound_stream::add_fw_route(sound_stream *target, int input, int output)
{
	m_fw_routes.emplace_back(target, input, output);
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
		streams.emplace_back(stream);
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
		streams.emplace_back(stream);
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

	for(unsigned int i = 0; i != m_bw_routes.size(); i++)
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
	deps.emplace_back(this);
}


//**// Gain management

void sound_stream::set_user_output_gain(float gain)
{
	if(gain == m_user_output_gain)
		return;
	update();
	m_user_output_gain = gain;
}

void sound_stream::set_user_output_gain(s32 output, float gain)
{
	if(gain == m_user_output_channel_gain[output])
		return;
	update();
	m_user_output_channel_gain[output] = gain;
}

void sound_stream::set_input_gain(s32 input, float gain)
{
	if(gain == m_input_channel_gain[input])
		return;
	update();
	m_input_channel_gain[input] = gain;
}

void sound_stream::apply_input_gain(s32 input, float gain)
{
	if(gain == 1.0f)
		return;
	update();
	m_input_channel_gain[input] *= gain;
}

void sound_stream::set_output_gain(s32 output, float gain)
{
	if(gain == m_output_channel_gain[output])
		return;
	update();
	m_output_channel_gain[output] = gain;
}

void sound_stream::apply_output_gain(s32 output, float gain)
{
	if(gain == 1.0f)
		return;
	update();
	m_output_channel_gain[output] *= gain;
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
		if(m_samples_to_update > 0)
			fatalerror("Error: set_sample_rate called while in stream_update\n");

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
		return freq != 0;

	} else if(output_adaptive() && !input_adaptive()) {
		u32 freq = 0;
		for(const route_fw &r : m_fw_routes) {
			if(!r.m_target->frequency_is_solved())
				return false;
			if(freq < r.m_target->sample_rate())
				freq = r.m_target->sample_rate();
		}
		m_sample_rate = freq;
		return freq != 0;

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
	if(!is_active() || m_in_update || m_device.machine().time().is_zero())
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
	if(!is_active() || m_in_update || m_device.machine().time().is_zero())
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


//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sound_interface - constructor
//-------------------------------------------------

device_sound_interface::device_sound_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "sound"),
	m_sound_requested_inputs_mask(0),
	m_sound_requested_outputs_mask(0),
	m_sound_requested_inputs(0),
	m_sound_requested_outputs(0),
	m_sound_hook(false)
{
}


//-------------------------------------------------
//  ~device_sound_interface - destructor
//-------------------------------------------------

device_sound_interface::~device_sound_interface()
{
}


//-------------------------------------------------
//  add_route - send sound output to a consumer
//-------------------------------------------------

device_sound_interface &device_sound_interface::add_route(u32 output, const char *target, double gain, u32 channel)
{
	return add_route(output, device().mconfig().current_device(), target, gain, channel);
}

device_sound_interface &device_sound_interface::add_route(u32 output, device_sound_interface &target, double gain, u32 channel)
{
	return add_route(output, target.device(), DEVICE_SELF, gain, channel);
}

device_sound_interface &device_sound_interface::add_route(u32 output, device_t &base, const char *target, double gain, u32 channel)
{
	assert(!device().started());
	m_route_list.emplace_back(sound_route{ output, channel, float(gain), base, target, nullptr });
	return *this;
}


//-------------------------------------------------
//  stream_alloc - allocate a stream implicitly
//  associated with this device
//-------------------------------------------------

sound_stream *device_sound_interface::stream_alloc(int inputs, int outputs, int sample_rate, sound_stream_flags flags)
{
	sound_stream *stream = device().machine().sound().stream_alloc(*this, inputs, outputs, sample_rate, stream_update_delegate(&device_sound_interface::sound_stream_update, this), flags);
	m_sound_streams.push_back(stream);
	return stream;
}



//-------------------------------------------------
//  inputs - return the total number of inputs
//  forthe given device
//-------------------------------------------------

int device_sound_interface::inputs() const
{
	// scan the list counting streams we own and summing their inputs
	int inputs = 0;
	for(sound_stream *stream : m_sound_streams)
		inputs += stream->input_count();
	return inputs;
}


//-------------------------------------------------
//  outputs - return the total number of outputs
//  forthe given device
//-------------------------------------------------

int device_sound_interface::outputs() const
{
	// scan the list counting streams we own and summing their outputs
	int outputs = 0;
	for(auto *stream : m_sound_streams)
		outputs += stream->output_count();
	return outputs;
}


//-------------------------------------------------
//  input_to_stream_input - convert a device's
//  input index to a stream and the input index
//  on that stream
//-------------------------------------------------

std::pair<sound_stream *, int> device_sound_interface::input_to_stream_input(int inputnum) const
{
	assert(inputnum >= 0);
	int orig_inputnum = inputnum;

	// scan the list looking forstreams owned by this device
	for(auto *stream : m_sound_streams) {
		if(inputnum < stream->input_count())
			return std::make_pair(stream, inputnum);
		inputnum -= stream->input_count();
	}

	fatalerror("Requested input %d on sound device %s which only has %d.", orig_inputnum, device().tag(), inputs());
}


//-------------------------------------------------
//  output_to_stream_output - convert a device's
//  output index to a stream and the output index
//  on that stream
//-------------------------------------------------

std::pair<sound_stream *, int> device_sound_interface::output_to_stream_output(int outputnum) const
{
	assert(outputnum >= 0);
	int orig_outputnum = outputnum;

	// scan the list looking forstreams owned by this device
	for(auto *stream : m_sound_streams) {
		if(outputnum < stream->output_count())
			return std::make_pair(stream, outputnum);
		outputnum -= stream->output_count();
	}

	fatalerror("Requested output %d on sound device %s which only has %d.", orig_outputnum, device().tag(), outputs());
}


//-------------------------------------------------
//  input_gain - return the gain on the given
//  input index of the device
//-------------------------------------------------

float device_sound_interface::input_gain(int inputnum) const
{
	auto [stream, input] = input_to_stream_input(inputnum);
	return stream->input_gain(input);
}


//-------------------------------------------------
//  output_gain - return the gain on the given
//  output index of the device
//-------------------------------------------------

float device_sound_interface::output_gain(int outputnum) const
{
	auto [stream, output] = output_to_stream_output(outputnum);
	return stream->output_gain(output);
}


//-------------------------------------------------
//  user_output_gain - return the user gain for the device
//-------------------------------------------------

float device_sound_interface::user_output_gain() const
{
	if(!outputs())
		fatalerror("Requested user output gain on sound device %s which has no outputs.", device().tag());
	return m_sound_streams.front()->user_output_gain();
}


//-------------------------------------------------
//  user_output_gain - return the user gain on the given
//  output index of the device
//-------------------------------------------------

float device_sound_interface::user_output_gain(int outputnum) const
{
	auto [stream, output] = output_to_stream_output(outputnum);
	return stream->user_output_gain(output);
}


//-------------------------------------------------
//  set_input_gain - set the gain on the given
//  input index of the device
//-------------------------------------------------

void device_sound_interface::set_input_gain(int inputnum, float gain)
{
	auto [stream, input] = input_to_stream_input(inputnum);
	stream->set_input_gain(input, gain);
}


//-------------------------------------------------
//  set_output_gain - set the gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_output_gain(int outputnum, float gain)
{
	// handle ALL_OUTPUTS as a special case
	if(outputnum == ALL_OUTPUTS)
	{
		if(!outputs())
			fatalerror("Requested setting output gain on sound device %s which has no outputs.", device().tag());
		for(auto *stream : m_sound_streams)
			for(int num = 0; num < stream->output_count(); num++)
				stream->set_output_gain(num, gain);
	}

	// look up the stream and stream output index
	else
	{
		auto [stream, output] = output_to_stream_output(outputnum);
		stream->set_output_gain(output, gain);
	}
}

//-------------------------------------------------
//  user_set_output_gain - set the user gain on the device
//-------------------------------------------------

void device_sound_interface::set_user_output_gain(float gain)
{
	if(!outputs())
		fatalerror("Requested setting user output gain on sound device %s which has no outputs.", device().tag());
	for(auto *stream : m_sound_streams)
		stream->set_user_output_gain(gain);
}



//-------------------------------------------------
//  set_user_output_gain - set the user gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_user_output_gain(int outputnum, float gain)
{
	auto [stream, output] = output_to_stream_output(outputnum);
	stream->set_user_output_gain(output, gain);
}


//-------------------------------------------------
//  set_route_gain - set the gain on a route
//-------------------------------------------------

void device_sound_interface::set_route_gain(int source_channel, device_sound_interface *target, int target_channel, float gain)
{
	auto [sstream, schan] = output_to_stream_output(source_channel);
	auto [tstream, tchan] = target->input_to_stream_input(target_channel);
	tstream->update();
	if(tstream->set_route_gain(sstream, schan, tchan, gain))
		return;

	fatalerror("Trying to change the gain on a non-existant route between %s channel %d and %s channel %d\n", device().tag(), source_channel, target->device().tag(), target_channel);
}



//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_sound_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all the routes
	for(sound_route const &route : routes())
	{
		// find a device with the requested tag
		device_t const *const target = route.m_base.get().subdevice(route.m_target);
		if(!target)
			osd_printf_error("Attempting to route sound to non-existent device '%s'\n", route.m_base.get().subtag(route.m_target));

		// if it's not a speaker or a sound device, error
		device_sound_interface const *sound;
		if(target && !target->interface(sound))
			osd_printf_error("Attempting to route sound to a non-sound device '%s' (%s)\n", target->tag(), target->name());
	}
}


//-------------------------------------------------
//  interface_pre_start - make sure all our input
//  devices are started
//-------------------------------------------------

void device_sound_interface::sound_before_devices_init()
{
	for(sound_route &route : routes()) {
		device_t *dev = route.m_base.get().subdevice(route.m_target);
		dev->interface(route.m_interface);
		if(route.m_output != ALL_OUTPUTS) {
			m_sound_requested_outputs_mask |= u64(1) << route.m_output;
			if(m_sound_requested_outputs <= route.m_output)
				m_sound_requested_outputs = route.m_output + 1;
		}
		route.m_interface->sound_request_input(route.m_input);
	}
}

void device_sound_interface::sound_after_devices_init()
{
	for(sound_route &route : routes()) {
		auto [si, ii] = route.m_interface->input_to_stream_input(route.m_input);
		if(!si)
			fatalerror("Requesting sound route to device %s input %d which doesn't exist\n", route.m_interface->device().tag(), route.m_input);
		if(route.m_output != ALL_OUTPUTS) {
			auto [so, io] = output_to_stream_output(route.m_output);
			if(!so)
				fatalerror("Requesting sound route from device %s output %d which doesn't exist\n", device().tag(), route.m_output);
			si->add_bw_route(so, io, ii, route.m_gain);
			so->add_fw_route(si, ii, io);

		} else {
			for(sound_stream *so : m_sound_streams)
				for(int io = 0; io != so->output_count(); io ++) {
					si->add_bw_route(so, io, ii, route.m_gain);
					so->add_fw_route(si, ii, io);
				}
		}
	}
}

void device_sound_interface::sound_request_input(u32 input)
{
	m_sound_requested_inputs_mask |= u64(1) << input;
	if(m_sound_requested_inputs <= input)
		m_sound_requested_inputs = input + 1;
}

device_mixer_interface::device_mixer_interface(const machine_config &mconfig, device_t &device) :
	device_sound_interface(mconfig, device)
{
}

device_mixer_interface::~device_mixer_interface()
{
}

void device_mixer_interface::interface_pre_start()
{
	u32 ni = get_sound_requested_inputs();
	u32 no = get_sound_requested_outputs();
	u32 nc = ni > no ? ni : no;
	for(u32 i = 0; i != nc; i++)
		stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
}

void device_mixer_interface::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
}
