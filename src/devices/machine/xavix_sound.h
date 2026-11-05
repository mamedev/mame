// license:BSD-3-Clause
// copyright-holders:ramacat, David Haywood, MagikalUnicorn
#ifndef MAME_MACHINE_XAVIX_SOUND_H
#define MAME_MACHINE_XAVIX_SOUND_H

#pragma once

#include <array>
#include <iterator>
#include <span>
#include <utility>

class xavix_sound_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto sample_read_callback() { return m_sample_read_cb.bind(); }
	auto irq_callback() { return m_irq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;
	template <typename T> void set_host_space(T &&tag, int spacenum) { m_host_space.set_tag(std::forward<T>(tag), spacenum); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	static constexpr unsigned VOICE_COUNT = 16;
	static constexpr unsigned TEMPO_TIMER_COUNT = 4;
	static constexpr unsigned OUTPUT_LANE_COUNT = 4;
	static constexpr unsigned STREAM_TICKS_PER_SAMPLE = 16;
	static constexpr unsigned CLOCKS_PER_DIVIDER_TICK = 8;
	static constexpr u8 SAMPLE_END = 0x80;
	static constexpr u8 MULTIPLEX_VOICE_ORDER[] = { 0x0, 0xa, 0x7, 0xd, 0xc, 0x6, 0xb, 0x1, 0x4, 0xe, 0x3, 0x9, 0x8, 0x2, 0xf, 0x5 };
	static constexpr u8 PARALLEL_VOICE_ORDER[] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };
	static_assert(std::size(MULTIPLEX_VOICE_ORDER) == VOICE_COUNT);
	static_assert(std::size(PARALLEL_VOICE_ORDER) == VOICE_COUNT);

	enum register_index : unsigned
	{
		WAVE_CONTROL = 0,
		WAVE_POSITION,
		WAVE_LOOP,
		WAVE_DATA,
		ENVELOPE_CONTROL,
		ENVELOPE_ADDRESS_LEFT,
		ENVELOPE_ADDRESS_RIGHT,
		ENVELOPE_STATE,
		VOLUME,
		VOLUME_LATCH,
		PITCH_COUNTER,
		TEMPO_COUNTER,
		REGISTER_COUNT
	};

	enum descriptor_offset : unsigned
	{
		DESCRIPTOR_WAVE_CONTROL = 0x00,
		DESCRIPTOR_WAVE_POSITION = 0x02,
		DESCRIPTOR_WAVE_LOOP = 0x04,
		DESCRIPTOR_SAMPLE_BANK = 0x06,
		DESCRIPTOR_ENVELOPE_CONTROL = 0x08,
		DESCRIPTOR_ENVELOPE_LEFT = 0x0a,
		DESCRIPTOR_ENVELOPE_RIGHT = 0x0c,
		DESCRIPTOR_ENVELOPE_BANK = 0x0e
	};

	enum control_phase : unsigned
	{
		PRESET_VOLUME_PHASE = 0,
		LOAD_WAVE_CONTROL_PHASE = 1,
		CLEAR_VOLUME_LATCH_PHASE = 2,
		LOAD_WAVE_POSITION_PHASE = 3,
		CLEAR_PITCH_COUNTER_PHASE = 4,
		LOAD_WAVE_LOOP_PHASE = 5,
		TEMPO_PHASE = 6,
		BEGIN_ENVELOPE_PHASE = 7,
		LOAD_ENVELOPE_CONTROL_PHASE = 9,
		UPDATE_LEFT_ENVELOPE_PHASE = 10,
		LOAD_LEFT_ENVELOPE_ADDRESS_PHASE = 11,
		WRITE_LEFT_ENVELOPE_ADDRESS_PHASE = 12,
		UPDATE_RIGHT_ENVELOPE_PHASE = 13,
		FINISH_ENVELOPE_PHASE = 15
	};

	enum command_state : u8
	{
		COMMAND_STOPPED = 0,
		COMMAND_START_PENDING,
		COMMAND_UPDATE_PENDING,
		COMMAND_RUNNING
	};

	enum wave_mode : u8
	{
		WAVE_DIRECT = 0,
		WAVE_NOISE,
		WAVE_LOOPING_SAMPLE,
		WAVE_ONE_SHOT_SAMPLE
	};

	enum envelope_mode : u8
	{
		ENVELOPE_DIRECT = 0,
		ENVELOPE_LOOPING,
		ENVELOPE_ONE_SHOT,
		ENVELOPE_DECAY
	};

	enum output_channel : unsigned
	{
		OUTPUT_LEFT = 0,
		OUTPUT_RIGHT,
		OUTPUT_CHANNEL_COUNT
	};

	struct voice_state
	{
		u16 wave_control = 0;
		u16 wave_position = 0; // sample address or noise shift state
		u16 wave_loop = 0; // sample restart address or noise feedback
		u16 wave_data = 0; // sample in high byte, bank in low byte
		u16 envelope_control = 0;
		u16 envelope_address[OUTPUT_CHANNEL_COUNT]{};
		u16 envelope_state = 0; // counter in high byte, bank in low byte
		u16 volume = 0; // stereo volume words pack left low, right high
		u16 volume_latch = 0;
		u16 pitch_counter = 0;
		u16 tempo_counter = 0;
		u8 command = COMMAND_STOPPED;
		bool output_ready = false;
		bool active = false;
		bool parameters_pending = false;

		void advance_command();
	};

	struct voice_output
	{
		bool active = false;
		s8 last_active_clock = -1; // inclusive cutoff as clocks count down; -1 means no cutoff
		u8 gain = 0;
		u8 volume[OUTPUT_CHANNEL_COUNT]{};
		u8 sample = 0;

		bool active_at(unsigned clock) const
		{
			return active && (last_active_clock < 0 || clock >= unsigned(last_active_clock));
		}
	};

	struct envelope_update
	{
		bool carry = false;
		envelope_mode mode = ENVELOPE_DIRECT;
		std::array<u8, OUTPUT_CHANNEL_COUNT> volume{};
	};

	struct output_lane
	{
		s16 sample = 0;
		u8 gain = 0;
		u8 volume[OUTPUT_CHANNEL_COUNT]{};
	};

	struct stereo_output
	{
		s64 left = 0;
		s64 right = 0;
	};

	struct output_stage
	{
		u8 sample_latch = 0;
		u8 gain_latch = 0;
		output_lane lane[OUTPUT_LANE_COUNT]{};
		bool mono_swap = false;
		u8 master_volume = 0;
		u8 mixer_control = 0x02;
		u8 dac_control = 0;

		void reset_controls();
		void mix_slot(unsigned voice, const voice_output &output, u8 rate_divider, std::span<const int> events, stereo_output &held, stereo_output &total);
		stereo_output held_output() const;
		unsigned voice_limit() const { return BIT(mixer_control, 5) ? 4 : BIT(mixer_control, 4) ? 8 : VOICE_COUNT; }
		bool parallel() const { return BIT(dac_control, 7); }
		s32 amplifier_gain() const;

	private:
		// This calibration maps the centred analogue voltage to doubled PCM units,
		// allowing interval clipping without changing the final rounding step.
		static constexpr s64 CLIP_MIN_X2 = -((1'650'000LL - 400'000) * 3'000 / (255 * 255));
		static constexpr s64 CLIP_MAX_X2 = (2'600'000LL - 1'650'000) * 3'000 / (255 * 255);
		static constexpr s32 GAIN[8] = { 2, 4, 8, 12, 16, 20, 20, 20 };
	};

	u8 voice_register_r(offs_t offset);
	void voice_register_w(offs_t offset, u8 data);
	u8 voice_command_r(offs_t offset);
	void voice_command_w(offs_t offset, u8 data);
	u8 voice_status_r(offs_t offset);
	u8 master_volume_r();
	void master_volume_w(u8 data);
	u8 work_page_r();
	void work_page_w(u8 data);
	u8 rate_divider_r();
	void rate_divider_w(u8 data);
	u8 mixer_control_r();
	void mixer_control_w(u8 data);
	u8 tempo_rate_r(offs_t offset);
	void tempo_rate_w(offs_t offset, u8 data);
	u8 irq_status_r();
	void irq_status_w(u8 data);
	u8 dac_control_r();
	void dac_control_w(u8 data);
	void synchronize();
	void update_descriptor_tap();
	void update_irq();

	u16 &voice_register(unsigned voice, unsigned index);
	u8 descriptor_byte_r(unsigned voice, unsigned field) { return m_descriptors.read_byte((u16(m_work_page) << 8) | (voice << 4) | field); }
	u16 descriptor_word_r(unsigned voice, unsigned field) { return u16(descriptor_byte_r(voice, field)) | (u16(descriptor_byte_r(voice, field + 1)) << 8); }
	void descriptor_byte_w(unsigned voice, unsigned field, u8 data) { m_descriptors.write_byte((u16(m_work_page) << 8) | (voice << 4) | field, data); }

	void render_frame(std::span<const int> event_clocks);
	voice_output advance_wave(unsigned voice);
	void run_control_phase(unsigned voice, unsigned phase, envelope_update &envelope);
	void advance_envelope(unsigned voice, output_channel channel, envelope_update &envelope, u16 decay_source);
	static constexpr u16 decay_pair(u16 value);

	void reschedule_tempo_timers(u8 timer_mask = 0x0f);
	TIMER_CALLBACK_MEMBER(tempo_timer_tick);

	sound_stream *m_stream;
	emu_timer *m_tempo_timer[TEMPO_TIMER_COUNT];

	voice_state m_voice[VOICE_COUNT];
	u8 m_voice_read_latch;
	u8 m_tempo_rate[TEMPO_TIMER_COUNT];
	u8 m_irq_status;
	u8 m_work_page;
	u8 m_rate_divider;
	u8 m_control_slot;
	u16 m_frame_phase;
	s32 m_output[OUTPUT_CHANNEL_COUNT];
	output_stage m_output_stage;

	address_space_config m_descriptor_config;
	memory_access<14, 0, 0, ENDIANNESS_LITTLE>::cache m_descriptors;
	required_address_space m_host_space;
	memory_passthrough_handler m_descriptor_write_tap;
	devcb_read8 m_sample_read_cb;
	devcb_write_line m_irq_cb;
};

DECLARE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device)

#endif // MAME_MACHINE_XAVIX_SOUND_H
