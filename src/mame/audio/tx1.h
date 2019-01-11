#ifndef MAME_AUDIO_TX1_H
#define MAME_AUDIO_TX1_H

#pragma once

class tx1_sound_device : public device_t, public device_sound_interface
{
public:
	tx1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tx1_sound_device() {}

	DECLARE_READ8_MEMBER( pit8253_r );
	DECLARE_WRITE8_MEMBER( pit8253_w );
	DECLARE_WRITE8_MEMBER( ay8910_a_w );
	DECLARE_WRITE8_MEMBER( ay8910_b_w );

protected:
	/*************************************
	 *
	 *  8253 Programmable Interval Timer
	 *
	 *************************************/
	struct pit8253_state
	{
		union
		{
#ifdef LSB_FIRST
			struct { uint8_t LSB; uint8_t MSB; } as8bit;
#else
			struct { uint8_t MSB; uint8_t LSB; } as8bit;
#endif
			uint16_t val;
		} counts[3];

		int idx[3];
	};

	tx1_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	sound_stream *m_stream;
	uint32_t m_freq_to_step;
	uint32_t m_step0;
	uint32_t m_step1;
	uint32_t m_step2;

	pit8253_state m_pit8253;

	uint8_t m_ay_outputa;
	uint8_t m_ay_outputb;

	stream_sample_t m_pit0;
	stream_sample_t m_pit1;
	stream_sample_t m_pit2;

	double m_weights0[4];
	double m_weights1[3];
	double m_weights2[3];
	int m_eng0[4];
	int m_eng1[4];
	int m_eng2[4];

	int m_noise_lfsra;
	int m_noise_lfsrb;
	int m_noise_lfsrc;
	int m_noise_lfsrd;
	int m_noise_counter;
	uint8_t m_ym1_outputa;
	uint8_t m_ym2_outputa;
	uint8_t m_ym2_outputb;
	uint16_t m_eng_voltages[16];
};


class buggyboy_sound_device : public tx1_sound_device
{
public:
	buggyboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( ym1_a_w );
	DECLARE_WRITE8_MEMBER( ym2_a_w );
	DECLARE_WRITE8_MEMBER( ym2_b_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


DECLARE_DEVICE_TYPE(TX1_SOUND, tx1_sound_device)
DECLARE_DEVICE_TYPE(BUGGYBOY_SOUND, buggyboy_sound_device)

#endif // MAME_AUDIO_TX1_H
