// license:BSD-3-Clause
// copyright-holders:ramacat, David Haywood
#ifndef MAME_TVGAMES_XAVIX_SOUND_H
#define MAME_TVGAMES_XAVIX_SOUND_H

#pragma once

class xavix_sound_device : public device_t, public device_sound_interface
{
public:
	xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_regs_callback() { return m_readregs_cb.bind(); }
	auto write_regs_callback() { return m_writeregs_cb.bind(); }   // <-- for vm1 LA/RA mirroring
	auto read_samples_callback() { return m_readsamples_cb.bind(); }

	// control API
	void enable_voice(int voice, bool update_only);
	void disable_voice(int voice);
	bool is_voice_enabled(int voice);
	uint32_t sample_ram_read(uint32_t page, uint16_t offset) const;

	// rate handling
	void set_tempo(int index, uint8_t value);
	void set_cyclerate(uint8_t value);

	// register handlers (from the CPU map)
	uint8_t sound_volume_r();
	void sound_volume_w(uint8_t data);
	uint8_t sound_mixer_r();
	void sound_mixer_w(uint8_t data);
	uint8_t dac_control_r();
	void dac_control_w(uint8_t data);

	// helpers
	void set_dac_gain(uint8_t amp_data);
	void set_output_mode(bool mono);
	void set_mastervol(uint8_t data);
	attotime tempo_period(uint8_t tempo) const;
	double tempo_frequency(uint8_t tempo) const;
	double tempo_tick_hz(uint8_t tempo) const;
	uint32_t  phase_step_per_tick(uint32_t rate) const;
	uint32_t envelope_period_ticks(uint8_t tp) const;

	void set_default_tempo(uint8_t tempo) { m_tempo_reset_value = tempo; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// voice state
	struct xavix_voice
	{
		uint8_t   enabled = 0;
		uint32_t  position = 0;
		uint32_t  loop_position = 0;
		uint32_t  start_position = 0;

		uint32_t  env_pos_left = 0;
		uint32_t  env_pos_right = 0;
		uint8_t   env_bank = 0;
		uint8_t   env_mode = 0;

		uint8_t   bank = 0;
		uint32_t  rate = 0;
		uint8_t   type = 0;
		uint8_t   vol = 0;

		uint16_t  env_rom_base_left = 0;
		uint16_t  env_rom_base_right = 0;

		uint8_t   env_vol_left = 0xff;
		uint8_t   env_vol_right = 0xff;

		uint32_t  env_period_samples = 0;
		uint32_t  env_countdown = 0;
		uint8_t   env_active_left = 1;
		uint8_t   env_active_right = 1;

		// misc (vm1/vm2 helpers, tickers)
		bool      log_env_started = false;
		bool      log_env_stopped = false;
		bool      log_env_paused = false;

		uint16_t noise_state = 0;
	};

	// mixer state
	struct xavix_mixer
	{
		uint8_t monaural = 0;
		uint8_t capacity = 0;
		uint8_t amp = 2;

		uint8_t dac = 0;
		uint8_t gap = 0;
		uint8_t lead = 0;
		uint8_t lag = 0;

		uint8_t mastervol = 0xff;
		int32_t gain = 2;
	};

	// stream
	sound_stream *m_stream = nullptr;

	// global timing
	uint8_t m_tempo_div[4] = { 0, 0, 0, 0 };
	uint8_t m_cyclerate_div = 1;
	uint32_t m_sequencer_rate_hz;

	// callbacks
	devcb_read8  m_readregs_cb;
	devcb_write8 m_writeregs_cb;     // used to mirror vm1 phase to LA/RA
	devcb_read8  m_readsamples_cb;

	xavix_mixer m_mix;
	xavix_voice m_voice[16];

	// helpers
	uint32_t tempo_to_period_ticks(uint8_t tp) const;
	uint8_t decay(uint8_t x);
	void step_pitch(int voice);
	void step_envelope(int voice);
	uint8_t fetch_env_byte(int voice, int channel, uint32_t idx);
	uint8_t fetch_env_byte_direct(int voice, int channel, uint16_t addr);

	// config
	uint8_t m_tempo_reset_value; // set from the xavix.cpp side to 0x80 for most games, 0x40 for the music games
};

DECLARE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device)

#endif // MAME_TVGAMES_XAVIX_SOUND_H
