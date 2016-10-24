// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
// thanks-to:Shay Green
#ifndef __GBSOUND_H__
#define __GBSOUND_H__


class gameboy_sound_device : public device_t,
							public device_sound_interface
{
public:
	gameboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	gameboy_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	uint8_t sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual uint8_t wave_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) = 0;
	virtual void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) = 0;
	virtual void wave_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) = 0;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

protected:
	enum
	{
		NR10 = 0x00,
		NR11 = 0x01,
		NR12 = 0x02,
		NR13 = 0x03,
		NR14 = 0x04,
		// 0x05
		NR21 = 0x06,
		NR22 = 0x07,
		NR23 = 0x08,
		NR24 = 0x09,
		NR30 = 0x0A,
		NR31 = 0x0B,
		NR32 = 0x0C,
		NR33 = 0x0D,
		NR34 = 0x0E,
		// 0x0F
		NR41 = 0x10,
		NR42 = 0x11,
		NR43 = 0x12,
		NR44 = 0x13,
		NR50 = 0x14,
		NR51 = 0x15,
		NR52 = 0x16,
		// 0x17 - 0x1F
		AUD3W0 = 0x20,
		AUD3W1 = 0x21,
		AUD3W2 = 0x22,
		AUD3W3 = 0x23,
		AUD3W4 = 0x24,
		AUD3W5 = 0x25,
		AUD3W6 = 0x26,
		AUD3W7 = 0x27,
		AUD3W8 = 0x28,
		AUD3W9 = 0x29,
		AUD3WA = 0x2A,
		AUD3WB = 0x2B,
		AUD3WC = 0x2C,
		AUD3WD = 0x2D,
		AUD3WE = 0x2E,
		AUD3WF = 0x2F
	};

	static const unsigned int FRAME_CYCLES = 8192;
	static const int wave_duty_table[4][8];

	sound_stream *m_channel;

	struct SOUND
	{
		/* Common */
		uint8_t  reg[5];
		bool   on;
		uint8_t  channel;
		uint8_t  length;
		uint8_t  length_mask;
		bool   length_counting;
		bool   length_enabled;
		/* Mode 1, 2, 3 */
		uint64_t cycles_left;
		int8_t   duty;
		/* Mode 1, 2, 4 */
		bool   envelope_enabled;
		int8_t   envelope_value;
		int8_t   envelope_direction;
		uint8_t  envelope_time;
		uint8_t  envelope_count;
		int8_t   signal;
		/* Mode 1 */
		uint16_t frequency;
		uint16_t frequency_counter;
		bool   sweep_enabled;
		bool   sweep_neg_mode_used;
		uint8_t  sweep_shift;
		int32_t  sweep_direction;
		uint8_t  sweep_time;
		uint8_t  sweep_count;
		/* Mode 3 */
		uint8_t  level;
		uint8_t  offset;
		uint32_t duty_count;
		int8_t   current_sample;
		bool   sample_reading;
		/* Mode 4 */
		bool   noise_short;
		uint16_t noise_lfsr;
	};

	struct SOUND  m_snd_1;
	struct SOUND  m_snd_2;
	struct SOUND  m_snd_3;
	struct SOUND  m_snd_4;

	struct
	{
		uint8_t on;
		uint8_t vol_left;
		uint8_t vol_right;
		uint8_t mode1_left;
		uint8_t mode1_right;
		uint8_t mode2_left;
		uint8_t mode2_right;
		uint8_t mode3_left;
		uint8_t mode3_right;
		uint8_t mode4_left;
		uint8_t mode4_right;
		uint64_t cycles;
		bool wave_ram_locked;
	} m_snd_control;

	uint8_t m_snd_regs[0x30];
	attotime m_last_updated;
	emu_timer *m_timer;

	virtual void apu_power_off() = 0;
	void sound_w_internal(int offset, uint8_t data);
	void update_square_channel(struct SOUND &snd, uint64_t cycles);
	virtual void update_wave_channel(struct SOUND &snd, uint64_t cycles) = 0;
	void update_noise_channel(struct SOUND &snd, uint64_t cycles);
	int32_t calculate_next_sweep(struct SOUND &snd);
	void apply_next_sweep(struct SOUND &snd);
	void tick_length(struct SOUND &snd);
	void tick_sweep(struct SOUND &snd);
	void tick_envelope(struct SOUND &snd);
	void update_state();
	bool dac_enabled(struct SOUND &snd);
	virtual void corrupt_wave_ram() { };
	uint64_t noise_period_cycles();
	void timer_callback(void *ptr, int32_t param);
};


class dmg_apu_device : public gameboy_sound_device
{
public:
	dmg_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t wave_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void wave_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	virtual void apu_power_off() override;
	virtual void corrupt_wave_ram() override;
	virtual void update_wave_channel(struct SOUND &snd, uint64_t cycles) override;
};


class cgb04_apu_device : public gameboy_sound_device
{
public:
	cgb04_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t wave_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void wave_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	virtual void device_reset() override;
	virtual void apu_power_off() override;
	virtual void update_wave_channel(struct SOUND &snd, uint64_t cycles) override;
};


extern const device_type DMG_APU;
//extern const device_type CGB02_APU;
extern const device_type CGB04_APU;
//extern const device_type CGB05_APU;

#endif
