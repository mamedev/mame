// license:BSD-3-Clause
// copyright-holders: Devin Acker

/***************************************************************************
    NEC/Casio uPD931 synthesis chip
***************************************************************************/

#ifndef MAME_SOUND_UPD931_H
#define MAME_SOUND_UPD931_H

#pragma once

#include <array>

class upd931_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	upd931_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto &set_master(bool master) { m_master = master; return *this; }

	auto filter_cb() { return m_filter_cb.bind(); }
	auto sync_cb() { return m_sync_cb.bind(); }

	void i1_w(int state);
	void i2_w(int state);
	void i3_w(int state);

	void db_w(u8 data);
	u8 db_r();

	void sync_w(int state);

protected:
	device_memory_interface::space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	static constexpr unsigned PITCH_SHIFT = 15;
	static constexpr unsigned VOLUME_SHIFT = 15;
	static constexpr unsigned VOLUME_MAX = (0xff << VOLUME_SHIFT);

	static constexpr unsigned CLOCKS_PER_SAMPLE = 8;
	static constexpr unsigned RETRIG_RATE = 0x60000;

	enum
	{
		FLAG_DECAY1         = 0, // bits 0-2 = decay1 rate
		FLAG_ATTACK2        = 3, // bits 3-5 = attack2 rate
		FLAG_ATTACK1        = 6, // bits 6-8 = attack1 rate
		FLAG_DECAY2_LEVEL   = 9, // bit 9 = decay1->decay2 transition point
		FLAG_DECAY2         = 10, // bit 10 = decay2 rate
		FLAG_RETRIGGER      = 11, // bit 11 = retrigger note during decay (mandolin effect)
		FLAG_ENV_SPLIT      = 12, // bit 12 = wave A envelope split(?)
		FLAG_ATTACK2_B      = 13, // bit 13 = wave B fades out during attack2
		FLAG_ATTACK2_A      = 14, // bit 14 = wave A fades in during attack2
		FLAG_DECAY2_DISABLE = 15, // bit 15 = disable decay2
		FLAG_ENV_SHIFT      = 16, // bit 16-17 = envelope/vibrato rate shift
		FLAG_MIRROR         = 19, // bit 19 = mirror waveform on alternate cycles
		FLAG_INVERT         = 20, // bit 20 = invert waveform on alternate cycles
		FLAG_MODE_B         = 21, // bit 21-22 = wave B output mode
		FLAG_MODE_A         = 23, // bit 23-24 = wave A output mode
		FLAG_WAVE_SEL       = 25, // bit 25 = wave data input select
	};

	enum
	{
		ENV_IDLE,
		ENV_ATTACK1,
		ENV_ATTACK2,
		ENV_DECAY1,
		ENV_DECAY2,
		ENV_RELEASE
	};

	struct voice_t
	{
		u8 m_note = 0;
		u8 m_octave = 0;
		u16 m_pitch = 0;
		u32 m_pitch_counter = 0;
		s8 m_timbre_shift = 0;

		u8 m_wave_pos = 0;
		s8 m_wave_out[2] = {0};

		u8 m_env_state = ENV_IDLE;
		u32 m_env_counter = 0;
		u32 m_env_level[2] = {0};
		u8 m_force_release = 0;
	};

	void io_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_tick);

	void note_w(offs_t offset, u8 data);
	void octave_w(offs_t offset, u8 data);
	void wave_pos_w(u8 data);
	void wave_data_w(u8 data);
	void flags_w(u8 data);
	void status_latch_w(offs_t offset, u8 data);
	void vibrato_w(u8 data);
	void sustain_w(u8 data);
	void note_on_w(offs_t offset, u8 data);

	void note_on(voice_t &voice);
	void reset_timer();
	void update_env(voice_t &voice);
	void update_wave(voice_t &voice);

	address_space_config m_io_config;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	sound_stream *m_stream;
	emu_timer *m_retrig_timer;

	devcb_write8 m_filter_cb;
	devcb_write_line m_sync_cb;

	bool m_master;

	u16 m_pitch[12*6 + 1]; // valid note range = C1 to C7

	u8 m_db;
	u8 m_i1, m_i2, m_i3;

	u8 m_addr, m_data;
	u8 m_status;

	// data global to all voices
	u8 m_wave[2][16];
	u8 m_wave_pos;
	u8 m_vibrato;
	u8 m_sustain;
	u8 m_reverb;

	u32 m_flags;
	u32 m_last_clock;

	std::array<voice_t, 8> m_voice;
};

DECLARE_DEVICE_TYPE(UPD931, upd931_device)

#endif // MAME_SOUND_UPD931_H
