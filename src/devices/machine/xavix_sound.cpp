// license:BSD-3-Clause
// copyright-holders:ramacat, David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX 16-voice sound device. Each voice holds its live registers and command
    state; the device coordinates voice/control scanning, memory callbacks and
    tempo. Each wave update and output slot is followed by one control phase
    for a selected voice. After the full scan, that voice advances its command
    state and the next frame selects the next control voice, preserving
    parameter-load and voice-ending timing.

    The output stage holds shared sample/gain latches, output controls and four
    lanes of sample, gain and stereo volume. It integrates the clipped stereo
    level between latch changes within each slot and averages the frame into
    PCM. A fixed-rate stream retains fractional phase across divider changes,
    and CPU register accesses synchronize pending audio.

***************************************************************************/

#include "emu.h"
#include "xavix_sound.h"

#include <algorithm>
#include <functional>
#include <span>

#define LOG_CFG        (1U << 1)
#define LOG_TEMPO      (1U << 2)
#define LOG_VOICE      (1U << 3)
#define LOG_WAVE       (1U << 4)
#define LOG_CLIP       (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device, "xavix_sound", "XaviX Sound")

xavix_sound_device::xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_work_page(0x02)
	, m_descriptor_config("descriptors", ENDIANNESS_LITTLE, 8, 14)
	, m_host_space(*this, finder_base::DUMMY_TAG, -1, 8)
	, m_sample_read_cb(*this, 0x80)
	, m_irq_cb(*this)
{
}

void xavix_sound_device::map(address_map &map)
{
	map(0x000, 0x17f).rw(FUNC(xavix_sound_device::voice_register_r), FUNC(xavix_sound_device::voice_register_w));
	map(0x1f0, 0x1f3).rw(FUNC(xavix_sound_device::voice_command_r), FUNC(xavix_sound_device::voice_command_w));
	map(0x1f4, 0x1f5).r(FUNC(xavix_sound_device::voice_status_r));
	map(0x1f6, 0x1f6).rw(FUNC(xavix_sound_device::master_volume_r), FUNC(xavix_sound_device::master_volume_w));
	map(0x1f7, 0x1f7).rw(FUNC(xavix_sound_device::work_page_r), FUNC(xavix_sound_device::work_page_w));
	map(0x1f8, 0x1f8).rw(FUNC(xavix_sound_device::rate_divider_r), FUNC(xavix_sound_device::rate_divider_w));
	map(0x1f9, 0x1f9).rw(FUNC(xavix_sound_device::mixer_control_r), FUNC(xavix_sound_device::mixer_control_w));
	map(0x1fa, 0x1fd).rw(FUNC(xavix_sound_device::tempo_rate_r), FUNC(xavix_sound_device::tempo_rate_w));
	map(0x1fe, 0x1fe).rw(FUNC(xavix_sound_device::irq_status_r), FUNC(xavix_sound_device::irq_status_w));
	map(0x1ff, 0x1ff).rw(FUNC(xavix_sound_device::dac_control_r), FUNC(xavix_sound_device::dac_control_w));
}

device_memory_interface::space_config_vector xavix_sound_device::memory_space_config() const
{
	return { { AS_PROGRAM, &m_descriptor_config } };
}

void xavix_sound_device::device_start()
{
	space(AS_PROGRAM).cache(m_descriptors);

	// Each divider tick spans eight input clocks.  At the reset divider, the
	// stream runs at one frame per sample (about 167.8 kHz with the NTSC clock).
	m_stream = stream_alloc(0, OUTPUT_CHANNEL_COUNT, clock() / (STREAM_TICKS_PER_SAMPLE * CLOCKS_PER_DIVIDER_TICK));
	for (unsigned index = 0; index < std::size(m_tempo_timer); index++)
		m_tempo_timer[index] = timer_alloc(FUNC(xavix_sound_device::tempo_timer_tick), this);

	save_item(STRUCT_MEMBER(m_voice, wave_control));
	save_item(STRUCT_MEMBER(m_voice, wave_position));
	save_item(STRUCT_MEMBER(m_voice, wave_loop));
	save_item(STRUCT_MEMBER(m_voice, wave_data));
	save_item(STRUCT_MEMBER(m_voice, envelope_control));
	save_item(STRUCT_MEMBER(m_voice, envelope_address));
	save_item(STRUCT_MEMBER(m_voice, envelope_state));
	save_item(STRUCT_MEMBER(m_voice, volume));
	save_item(STRUCT_MEMBER(m_voice, volume_latch));
	save_item(STRUCT_MEMBER(m_voice, pitch_counter));
	save_item(STRUCT_MEMBER(m_voice, tempo_counter));
	save_item(STRUCT_MEMBER(m_voice, command));
	save_item(STRUCT_MEMBER(m_voice, output_ready));
	save_item(STRUCT_MEMBER(m_voice, active));
	save_item(STRUCT_MEMBER(m_voice, parameters_pending));
	save_item(NAME(m_voice_read_latch));
	save_item(NAME(m_tempo_rate));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_work_page));
	save_item(NAME(m_rate_divider));
	save_item(NAME(m_control_slot));
	save_item(NAME(m_frame_phase));
	save_item(NAME(m_output));
	save_item(NAME(m_output_stage.sample_latch));
	save_item(NAME(m_output_stage.gain_latch));
	save_item(STRUCT_MEMBER(m_output_stage.lane, sample));
	save_item(STRUCT_MEMBER(m_output_stage.lane, gain));
	save_item(STRUCT_MEMBER(m_output_stage.lane, volume));
	save_item(NAME(m_output_stage.mono_swap));
	save_item(NAME(m_output_stage.master_volume));
	save_item(NAME(m_output_stage.mixer_control));
	save_item(NAME(m_output_stage.dac_control));
	update_descriptor_tap();
}

void xavix_sound_device::device_reset()
{
	// Soft reset retains the voice register words and output lane holds.
	for (voice_state &voice : m_voice)
	{
		voice.command = COMMAND_STOPPED;
		voice.output_ready = false;
		voice.active = false;
		voice.parameters_pending = false;
	}

	m_voice_read_latch = 0;
	std::ranges::fill(m_tempo_rate, 0);
	m_irq_status = 0;
	m_work_page = 0x02; // rad_bb relies on this default without initializing the register
	m_rate_divider = 0x0f;
	m_control_slot = 0;
	m_frame_phase = 0;
	std::ranges::fill(m_output, 0);
	m_output_stage.reset_controls();
	update_descriptor_tap();

	for (emu_timer *timer : m_tempo_timer)
		timer->adjust(attotime::never);
	update_irq();

	LOGMASKED(LOG_CFG, "[cfg] reset divider=%02x amp=%u gain=%d\n",
		m_rate_divider, BIT(m_output_stage.mixer_control, 0, 3), m_output_stage.amplifier_gain());
}

void xavix_sound_device::device_post_load()
{
	update_descriptor_tap();
}

//-------------------------------------------------
//  Register Interface
//-------------------------------------------------

void xavix_sound_device::synchronize()
{
	if (m_stream)
		m_stream->update();
}

void xavix_sound_device::update_descriptor_tap()
{
	m_descriptor_write_tap.remove();
	offs_t const base = offs_t(m_work_page) << 8;
	m_descriptor_write_tap = m_host_space->install_write_tap(base, base | 0xff, "sound_descriptors",
		[this](offs_t offset, u8 &data, u8 mem_mask) { synchronize(); });
}

u16 &xavix_sound_device::voice_register(unsigned voice, unsigned index)
{
	voice_state &state = m_voice[voice];
	switch (index)
	{
	case WAVE_CONTROL: return state.wave_control;
	case WAVE_POSITION: return state.wave_position;
	case WAVE_LOOP: return state.wave_loop;
	case WAVE_DATA: return state.wave_data;
	case ENVELOPE_CONTROL: return state.envelope_control;
	case ENVELOPE_ADDRESS_LEFT: return state.envelope_address[OUTPUT_LEFT];
	case ENVELOPE_ADDRESS_RIGHT: return state.envelope_address[OUTPUT_RIGHT];
	case ENVELOPE_STATE: return state.envelope_state;
	case VOLUME: return state.volume;
	case VOLUME_LATCH: return state.volume_latch;
	case PITCH_COUNTER: return state.pitch_counter;
	case TEMPO_COUNTER: return state.tempo_counter;
	default: throw emu_fatalerror("%s: invalid voice register %u", tag(), index);
	}
}

u8 xavix_sound_device::voice_register_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		synchronize();
	const unsigned index = offset >> 5;
	const unsigned voice = offset & (VOICE_COUNT - 1);
	if (index >= REGISTER_COUNT)
		return 0;

	const u8 data = voice_register(voice, index) >> (BIT(offset, 4) ? 8 : 0);
	// All voice registers share a one-access read pipeline.
	return machine().side_effects_disabled() ? data : std::exchange(m_voice_read_latch, data);
}

void xavix_sound_device::voice_register_w(offs_t offset, u8 data)
{
	synchronize();
	const unsigned index = offset >> 5;
	const unsigned voice = offset & (VOICE_COUNT - 1);
	if (index >= REGISTER_COUNT)
		return;

	u16 &value = voice_register(voice, index);
	const unsigned shift = BIT(offset, 4) * 8;
	// The shared read latch captures the byte before the write completes.
	m_voice_read_latch = value >> shift;
	value = (value & ~(0xff << shift)) | (u16(data) << shift);

	if (index == TEMPO_COUNTER && voice < std::size(m_tempo_timer))
		reschedule_tempo_timers(1U << voice);
}

u8 xavix_sound_device::voice_command_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		synchronize();
	const unsigned first = BIT(offset, 0) ? 8 : 0;
	u8 result = 0;
	for (unsigned bit = 0; bit < 8; bit++)
	{
		const command_state command = command_state(m_voice[first + bit].command);
		const bool asserted = offset < 2
			? command != COMMAND_STOPPED
			: command == COMMAND_UPDATE_PENDING;
		result |= asserted ? u8(1U << bit) : 0;
	}
	return result;
}

void xavix_sound_device::voice_command_w(offs_t offset, u8 data)
{
	synchronize();
	const unsigned first = BIT(offset, 0) ? 8 : 0;

	for (unsigned bit = 0; bit < 8; bit++)
	{
		voice_state &voice = m_voice[first + bit];
		if (offset < 2)
		{
			if (!BIT(data, bit))
				voice.command = COMMAND_STOPPED;
			else if (voice.command == COMMAND_STOPPED)
				voice.command = COMMAND_START_PENDING;
		}
		else if (BIT(data, bit) && voice.command == COMMAND_RUNNING)
		{
			voice.command = COMMAND_UPDATE_PENDING;
		}
	}

	LOGMASKED(LOG_VOICE, "[voice] command offset=%u data=%02x\n", unsigned(offset), data);
}

u8 xavix_sound_device::voice_status_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		synchronize();
	const unsigned first = BIT(offset, 0) ? 8 : 0;
	u8 result = 0;
	for (unsigned bit = 0; bit < 8; bit++)
	{
		const voice_state &voice = m_voice[first + bit];
		result |= (voice.active || voice.parameters_pending) ? u8(1U << bit) : 0;
	}
	return result;
}

u8 xavix_sound_device::work_page_r()
{
	return m_work_page;
}

void xavix_sound_device::work_page_w(u8 data)
{
	synchronize();
	data &= 0x3f;
	if (m_work_page != data)
	{
		m_work_page = data;
		update_descriptor_tap();
	}
}

u8 xavix_sound_device::tempo_rate_r(offs_t offset)
{
	return m_tempo_rate[offset];
}

void xavix_sound_device::tempo_rate_w(offs_t offset, u8 data)
{
	synchronize();
	m_tempo_rate[offset] = data;
	reschedule_tempo_timers(1U << offset);
	LOGMASKED(LOG_TEMPO, "[tempo] rate %u=%02x\n", unsigned(offset), data);
}

u8 xavix_sound_device::irq_status_r()
{
	// UK e-kara carts check the upper nibble for sound-timer IRQ source.
	if (!machine().side_effects_disabled())
		synchronize();
	return m_irq_status;
}

void xavix_sound_device::irq_status_w(u8 data)
{
	// Bring the timer state current before changing pending/enables; retain counter phase.
	synchronize();
	const u8 old_enable = m_irq_status & 0x0f;
	const u8 new_enable = data & 0x0f;
	m_irq_status = (m_irq_status & 0xf0) | new_enable;
	reschedule_tempo_timers();
	const u8 clear_mask = data & 0xf0;
	m_irq_status &= ~clear_mask;
	LOGMASKED(LOG_TEMPO, "[irq] status_w %02x old_en=%02x new_en=%02x clear=%02x pending=%02x\n",
		data, old_enable, new_enable, clear_mask >> 4, m_irq_status >> 4);
	update_irq();
}

void xavix_sound_device::update_irq()
{
	const u8 enable = m_irq_status & 0x0f;
	const u8 pending = m_irq_status >> 4;
	m_irq_cb((enable & pending) != 0);
	LOGMASKED(LOG_TEMPO, "[irq] line %s enable=%02x pending=%02x\n",
		(enable & pending) ? "assert" : "clear", enable, pending);
}

u8 xavix_sound_device::rate_divider_r()
{
	return m_rate_divider;
}

void xavix_sound_device::rate_divider_w(u8 data)
{
	synchronize();
	m_rate_divider = data;
	reschedule_tempo_timers();
	LOGMASKED(LOG_TEMPO, "[rate] divider=%02x\n", data);
}

u8 xavix_sound_device::master_volume_r()
{
	if (!machine().side_effects_disabled())
		synchronize();
	return m_output_stage.master_volume;
}

void xavix_sound_device::master_volume_w(u8 data)
{
	synchronize();
	m_output_stage.master_volume = data;
}

u8 xavix_sound_device::mixer_control_r()
{
	if (!machine().side_effects_disabled())
		synchronize();
	return m_output_stage.mixer_control;
}

void xavix_sound_device::mixer_control_w(u8 data)
{
	synchronize();
	m_output_stage.mixer_control = data & 0xb7;
}

u8 xavix_sound_device::dac_control_r()
{
	if (!machine().side_effects_disabled())
		synchronize();
	return m_output_stage.dac_control;
}

void xavix_sound_device::dac_control_w(u8 data)
{
	synchronize();
	m_output_stage.dac_control = data;
	reschedule_tempo_timers();
}

//-------------------------------------------------
//  Voice Rendering
//-------------------------------------------------

void xavix_sound_device::sound_stream_update(sound_stream &stream)
{
	// Register writes synchronize first, so every frame in this update uses the same latch clocks.
	const u8 dac_control = m_output_stage.dac_control;
	const int sample_clock = 3 - BIT(dac_control, 0, 2);
	std::array<int, 6> events{ 13, 9, 3 + int(BIT(dac_control, 2, 3)), 3, sample_clock, sample_clock + int(BIT(dac_control, 5, 2)) };
	std::ranges::sort(events, std::greater<int>());
	const std::span<const int> event_clocks(events.begin(), std::unique(events.begin(), events.end()));

	for (int output_sample = 0; output_sample < stream.samples(); output_sample++)
	{
		m_frame_phase += STREAM_TICKS_PER_SAMPLE;
		const unsigned divider = unsigned(m_rate_divider) + 1;
		while (m_frame_phase >= divider)
		{
			m_frame_phase -= divider;
			render_frame(event_clocks);
		}

		stream.put_int(OUTPUT_LEFT, output_sample, m_output[OUTPUT_LEFT], 32768);
		stream.put_int(OUTPUT_RIGHT, output_sample, m_output[OUTPUT_RIGHT], 32768);
	}
}

void xavix_sound_device::render_frame(std::span<const int> event_clocks)
{
	const auto &order = m_output_stage.parallel() ? PARALLEL_VOICE_ORDER : MULTIPLEX_VOICE_ORDER;
	const unsigned control_voice = order[m_control_slot & (VOICE_COUNT - 1)];

	stereo_output total;
	stereo_output held = m_output_stage.held_output();
	envelope_update envelope;

	for (unsigned phase = 0; phase < std::size(m_voice); phase++)
	{
		// Capacity affects only output latches; every voice continues advancing.
		const unsigned voice = order[phase];
		const voice_output output = advance_wave(voice);
		m_output_stage.mix_slot(voice, output, m_rate_divider, event_clocks, held, total);
		run_control_phase(control_voice, phase, envelope);
	}

	m_voice[control_voice].advance_command();
	if (m_control_slot == VOICE_COUNT - 1)
		m_output_stage.mono_swap = !m_output_stage.mono_swap;
	m_control_slot = (m_control_slot + 1) & (VOICE_COUNT - 1);

	const s64 frame_clocks = VOICE_COUNT * (s64(m_rate_divider) + 1);
	auto frame_average = [frame_clocks] (s64 value)
	{
		return (value + (value < 0 ? -frame_clocks : frame_clocks)) / (frame_clocks * 2);
	};

	total.left = frame_average(total.left);
	total.right = frame_average(total.right);
	if (total.left > 32767 || total.left < -32768 || total.right > 32767 || total.right < -32768)
		LOGMASKED(LOG_CLIP, "[clip] L=%lld R=%lld\n", (long long)total.left, (long long)total.right);

	m_output[OUTPUT_LEFT] = std::clamp<s64>(total.left, -32768, 32767);
	m_output[OUTPUT_RIGHT] = std::clamp<s64>(total.right, -32768, 32767);
}

xavix_sound_device::voice_output xavix_sound_device::advance_wave(unsigned voice_index)
{
	voice_state &voice = m_voice[voice_index];
	if (!voice.output_ready || !voice.active)
		return {};

	const wave_mode mode = wave_mode(voice.wave_control & 3);
	const u16 pitch = voice.wave_control & 0xfffc;
	const u16 old_counter = voice.pitch_counter & 0xfffc;
	bool carry = old_counter < pitch;
	voice.pitch_counter = old_counter - pitch;

	const u8 raw = voice.wave_data >> 8;
	const u8 bank = voice.wave_data;
	if (mode == WAVE_ONE_SHOT_SAMPLE && raw == SAMPLE_END)
		carry = false;

	const u16 base = (mode != WAVE_NOISE && raw == SAMPLE_END) ? voice.wave_loop : voice.wave_position;
	u16 next_position = base + carry;
	u8 next_raw = raw;

	switch (mode)
	{
	case WAVE_DIRECT:
		next_raw = descriptor_byte_r(voice_index, DESCRIPTOR_WAVE_POSITION);
		break;

	case WAVE_NOISE:
		if (carry)
			next_position = (base >> 1) ^ ((base & 1) ? voice.wave_loop : 0);
		next_raw = (next_position & 1) ? 0x81 : 0x7f;
		break;

	case WAVE_LOOPING_SAMPLE:
	case WAVE_ONE_SHOT_SAMPLE:
		if (carry)
			next_raw = m_sample_read_cb((u32(bank) << 16) | base);
		break;
	}

	voice_output result{ .active = true, .gain = u8(voice.envelope_control & 0x0f), .sample = raw };

	// A one-shot sentinel stops between the shared sample and gain latch clocks.
	if (mode == WAVE_ONE_SHOT_SAMPLE && raw == SAMPLE_END)
	{
		result.last_active_clock = 12;
		voice.active = false;
		LOGMASKED(LOG_WAVE, "[wave] voice %u one-shot end\n", voice_index);
		return result;
	}

	voice.wave_position = next_position;

	const u16 volume = (mode == WAVE_LOOPING_SAMPLE && raw != 0x00 && raw != SAMPLE_END)
		? voice.volume_latch
		: voice.volume;

	// A zero envelope stops after the shared sample/gain latches, before
	// committing the voice's held volume or next sample.
	if (volume == 0)
	{
		result.last_active_clock = 5;
		voice.active = false;
		return result;
	}

	voice.volume_latch = volume;
	result.volume[OUTPUT_LEFT] = u8(volume);
	result.volume[OUTPUT_RIGHT] = u8(volume >> 8);
	voice.wave_data = (u16(next_raw) << 8) | bank;
	return result;
}

void xavix_sound_device::run_control_phase(unsigned voice_index, unsigned phase, envelope_update &envelope)
{
	voice_state &voice = m_voice[voice_index];
	const bool active = voice.active;
	const bool preset = !voice.output_ready && voice.parameters_pending;
	const bool update = voice.active && voice.parameters_pending;

	// A start loads voice parameters; an update reloads wave/envelope controls
	// and loop/noise feedback.  A voice may end between control phases.
	switch (phase)
	{
	case PRESET_VOLUME_PHASE:
		if (preset)
			voice.volume = 0xffff;
		break;

	case LOAD_WAVE_CONTROL_PHASE:
		if (preset || update)
			voice.wave_control = descriptor_word_r(voice_index, DESCRIPTOR_WAVE_CONTROL);
		break;

	case CLEAR_VOLUME_LATCH_PHASE:
		if (preset)
			voice.volume_latch = 0x0000;
		break;

	case LOAD_WAVE_POSITION_PHASE:
		if (preset)
			voice.wave_position = descriptor_word_r(voice_index, DESCRIPTOR_WAVE_POSITION);
		break;

	case CLEAR_PITCH_COUNTER_PHASE:
		if (preset)
			voice.pitch_counter = 0x0000;
		break;

	case LOAD_WAVE_LOOP_PHASE:
		if (preset || update)
			voice.wave_loop = descriptor_word_r(voice_index, DESCRIPTOR_WAVE_LOOP);
		break;

	case TEMPO_PHASE:
		if (BIT(m_irq_status, voice_index & (TEMPO_TIMER_COUNT - 1)))
		{
			const u16 step = u16(m_tempo_rate[voice_index & (TEMPO_TIMER_COUNT - 1)]) << 6;
			const bool borrow = voice.tempo_counter < step;
			voice.tempo_counter -= step;
			if (voice_index < std::size(m_tempo_timer) && borrow)
			{
				m_irq_status |= 0x10 << voice_index;
				LOGMASKED(LOG_TEMPO, "[tempo] %u latch pending=%02x\n", voice_index, m_irq_status >> 4);
				update_irq();
			}
		}
		break;

	case BEGIN_ENVELOPE_PHASE:
	{
		// The envelope mode is sampled before a pending control update.
		envelope.mode = envelope_mode(BIT(voice.envelope_control, 4, 2));
		const u8 rate = voice.envelope_control >> 8;
		const u8 phase_counter = voice.envelope_state >> 8;
		envelope.carry = phase_counter < rate;
		if (active)
			voice.envelope_state = (voice.envelope_state & 0x00ff) | (u16(u8(phase_counter - rate)) << 8);
		if (preset)
			voice.wave_data = descriptor_byte_r(voice_index, DESCRIPTOR_SAMPLE_BANK);
		break;
	}

	case LOAD_ENVELOPE_CONTROL_PHASE:
		if (preset || update)
			voice.envelope_control = descriptor_word_r(voice_index, DESCRIPTOR_ENVELOPE_CONTROL);
		break;

	case UPDATE_LEFT_ENVELOPE_PHASE:
		if (active)
			advance_envelope(voice_index, OUTPUT_LEFT, envelope, voice.envelope_address[OUTPUT_LEFT]);
		break;

	case LOAD_LEFT_ENVELOPE_ADDRESS_PHASE:
		if (preset)
			voice.envelope_address[OUTPUT_LEFT] = descriptor_word_r(voice_index, DESCRIPTOR_ENVELOPE_LEFT);
		break;

	case WRITE_LEFT_ENVELOPE_ADDRESS_PHASE:
		if (active && envelope.mode == ENVELOPE_LOOPING)
			descriptor_byte_w(voice_index, DESCRIPTOR_ENVELOPE_LEFT, u8(voice.envelope_address[OUTPUT_LEFT]));
		break;

	case UPDATE_RIGHT_ENVELOPE_PHASE:
		if (active)
		{
			// Right decay uses stereo volume; left decay uses its address word.
			advance_envelope(voice_index, OUTPUT_RIGHT, envelope, voice.volume);
		}
		if (preset)
			voice.envelope_address[OUTPUT_RIGHT] = descriptor_word_r(voice_index, DESCRIPTOR_ENVELOPE_RIGHT);
		break;

	case FINISH_ENVELOPE_PHASE:
		if (active && envelope.mode == ENVELOPE_LOOPING)
			descriptor_byte_w(voice_index, DESCRIPTOR_ENVELOPE_RIGHT, u8(voice.envelope_address[OUTPUT_RIGHT]));

		if (active && envelope.carry)
		{
			if (envelope.mode == ENVELOPE_DECAY)
			{
				voice.volume = decay_pair(voice.volume);
			}
			else
			{
				const u8 left = u8(voice.volume) ? envelope.volume[OUTPUT_LEFT] : 0;
				const u8 right = u8(voice.volume >> 8) ? envelope.volume[OUTPUT_RIGHT] : 0;
				voice.volume = (u16(right) << 8) | left;
			}
		}
		if (preset)
			voice.envelope_state = descriptor_byte_r(voice_index, DESCRIPTOR_ENVELOPE_BANK);
		break;

	default:
		break;
	}
}

void xavix_sound_device::advance_envelope(unsigned voice_index, output_channel channel, envelope_update &envelope, u16 decay_source)
{
	voice_state &voice = m_voice[voice_index];
	u16 &address = voice.envelope_address[channel];
	u8 &volume = envelope.volume[channel];

	switch (envelope.mode)
	{
	case ENVELOPE_DIRECT:
		volume = descriptor_byte_r(voice_index, channel == OUTPUT_LEFT ? DESCRIPTOR_ENVELOPE_LEFT : DESCRIPTOR_ENVELOPE_RIGHT);
		address += envelope.carry;
		break;

	case ENVELOPE_LOOPING:
	case ENVELOPE_ONE_SHOT:
		if (envelope.carry)
		{
			volume = m_sample_read_cb((u32(voice.envelope_state & 0xff) << 16) | address);
			if (envelope.mode == ENVELOPE_LOOPING)
				address = (address & 0xfff0) | ((address + 1) & 0x000f);
			else if (u8(voice.volume >> (channel * 8)) != 0)
				address++;
		}
		break;

	case ENVELOPE_DECAY:
		address = decay_pair(decay_source);
		break;
	}
}

constexpr u16 xavix_sound_device::decay_pair(u16 value)
{
	const auto decay = [] (u8 value) { return value * 15 / 16; };
	return u16(decay(value >> 8)) << 8 | decay(value);
}

void xavix_sound_device::voice_state::advance_command()
{
	const bool running_command = command == COMMAND_UPDATE_PENDING || command == COMMAND_RUNNING;

	// Command acceptance, activation, and output readiness occur on successive
	// control passes.  The priority matters when a stop or update coincides with
	// a voice reaching one of those boundaries.
	if (command == COMMAND_STOPPED || command == COMMAND_START_PENDING)
	{
		output_ready = false;
		active = false;
		parameters_pending = command == COMMAND_START_PENDING;
	}
	else if (running_command && (active || parameters_pending))
	{
		const bool reload_parameters = command == COMMAND_UPDATE_PENDING && output_ready && active;
		output_ready = active;
		active = true;
		parameters_pending = reload_parameters;
	}

	if (command == COMMAND_START_PENDING || command == COMMAND_UPDATE_PENDING)
		command = COMMAND_RUNNING;
}

//-------------------------------------------------
//  Output Mixing
//-------------------------------------------------

void xavix_sound_device::output_stage::reset_controls()
{
	sample_latch = 0;
	gain_latch = 0;
	mono_swap = true;
	master_volume = 0;
	mixer_control = 0x02;
	dac_control = 0;
}

void xavix_sound_device::output_stage::mix_slot(unsigned voice, const voice_output &output, u8 rate_divider, std::span<const int> events, stereo_output &held, stereo_output &total)
{
	if (voice >= voice_limit())
	{
		total.left += held.left * (int(rate_divider) + 1);
		total.right += held.right * (int(rate_divider) + 1);
		return;
	}

	const int gap_clocks = BIT(dac_control, 5, 2);
	const int gain_clock = 3 + BIT(dac_control, 2, 3);
	const int volume_latch_clock = 3;
	const int sample_clock = 3 - BIT(dac_control, 0, 2);
	const int blank_clock = sample_clock + gap_clocks;

	std::span<output_lane> lanes(lane);
	if (!parallel())
		lanes = lanes.subspan(voice & (OUTPUT_LANE_COUNT - 1), 1);

	// Slot clocks count down to zero; held levels are constant between latch events.
	int cursor = rate_divider;
	for (const int clock : events)
	{
		if (clock > rate_divider)
			continue;

		total.left += held.left * (cursor - clock);
		total.right += held.right * (cursor - clock);

		const bool active = output.active_at(clock);
		const bool latch_gain = clock == gain_clock;
		const bool latch_volume = clock == volume_latch_clock;
		const bool latch_sample = clock == sample_clock || clock == blank_clock;
		if (latch_gain || latch_volume || latch_sample)
		{
			for (output_lane &target : lanes)
			{
				if (latch_gain)
					target.gain = active ? gain_latch : 0;
				if (latch_volume)
				{
					target.volume[OUTPUT_LEFT] = active ? output.volume[OUTPUT_LEFT] : 0;
					target.volume[OUTPUT_RIGHT] = active ? output.volume[OUTPUT_RIGHT] : 0;
					if (BIT(mixer_control, 7) && mono_swap)
						std::swap(target.volume[OUTPUT_LEFT], target.volume[OUTPUT_RIGHT]);
				}
				if (latch_sample)
				{
					const bool blank = !active || (gap_clocks && clock == blank_clock);
					target.sample = blank || sample_latch == SAMPLE_END ? 0 : s8(sample_latch);
				}
			}
			held = held_output();
		}

		// Output lanes see the previous shared latch value when clocks coincide.
		if (clock == 13)
			sample_latch = active ? output.sample : 0;
		if (clock == 9)
			gain_latch = active ? output.gain : 0;

		cursor = clock;
	}

	// Each event clock uses the new lane values; include clock zero.
	total.left += held.left * (cursor + 1);
	total.right += held.right * (cursor + 1);
}

xavix_sound_device::stereo_output xavix_sound_device::output_stage::held_output() const
{
	stereo_output result;
	for (const output_lane &held : lane)
	{
		const s64 scaled = (s64(held.sample) * held.gain * master_volume) / 255;
		result.left += (scaled * held.volume[OUTPUT_LEFT]) / 255;
		result.right += (scaled * held.volume[OUTPUT_RIGHT]) / 255;
	}

	result.left = std::clamp<s64>(result.left * amplifier_gain() * 3, CLIP_MIN_X2, CLIP_MAX_X2);
	result.right = std::clamp<s64>(result.right * amplifier_gain() * 3, CLIP_MIN_X2, CLIP_MAX_X2);
	return result;
}

s32 xavix_sound_device::output_stage::amplifier_gain() const
{
	return GAIN[BIT(mixer_control, 0, 3)];
}

//-------------------------------------------------
//  Tempo Timers
//-------------------------------------------------

void xavix_sound_device::reschedule_tempo_timers(u8 timer_mask)
{
	for (unsigned index = 0; index < std::size(m_tempo_timer); index++)
	{
		if (!BIT(timer_mask, index) || !m_tempo_timer[index])
			continue;

		const u16 step = u16(m_tempo_rate[index]) << 6;
		if (!BIT(m_irq_status, index) || !step)
		{
			m_tempo_timer[index]->adjust(attotime::never, index);
			continue;
		}

		// Audio is normally rendered in batches.  Wake the stream at the next
		// counter borrow so the CPU can observe and acknowledge every timer edge.
		const auto &order = m_output_stage.parallel() ? PARALLEL_VOICE_ORDER : MULTIPLEX_VOICE_ORDER;
		const unsigned target_slot = std::ranges::find(order, index) - std::begin(order);

		const u64 first_service = ((target_slot - m_control_slot) & (VOICE_COUNT - 1)) + 1;
		const u64 frames = first_service + (u64(m_voice[index].tempo_counter) / step) * std::size(m_voice);

		const u64 divider_clocks = frames * (u64(m_rate_divider) + 1);
		// A shorter divider can leave this service overdue. The next stream
		// sample consumes the retained phase; the timer must advance time.
		const u64 clocks_needed = divider_clocks > m_frame_phase ? divider_clocks - m_frame_phase : 1;
		const u64 samples = (clocks_needed + STREAM_TICKS_PER_SAMPLE - 1) / STREAM_TICKS_PER_SAMPLE;
		m_tempo_timer[index]->adjust(attotime::from_ticks(samples, m_stream->sample_rate()), index);
	}
}

TIMER_CALLBACK_MEMBER(xavix_sound_device::tempo_timer_tick)
{
	// Updating the stream performs the counter subtraction and IRQ callback;
	// re-arm from the resulting counter and fractional frame phase.
	synchronize();
	reschedule_tempo_timers(1U << param);
}
