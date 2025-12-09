// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_SOUND_T6W28_H
#define MAME_SOUND_T6W28_H

#pragma once

class t6w28_device : public device_t, public device_sound_interface
{
public:
	t6w28_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	void set_enable(bool enable);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	void set_gain(int gain);

private:
	sound_stream *m_channel;
	int m_sample_rate;
	int m_vol_table[16];    /* volume table         */
	int32_t m_register[16];   /* registers */
	int32_t m_last_register[2];   /* last register written */
	int32_t m_volume[8];  /* volume of voice 0-2 and noise */
	uint32_t m_rng[2];        /* noise generator      */
	int32_t m_noise_mode[2];  /* active noise mode */
	int32_t m_feedback_mask;     /* mask for feedback */
	int32_t m_whitenoise_taps;   /* mask for white noise taps */
	int32_t m_whitenoise_invert; /* white noise invert flag */
	int32_t m_period[8];
	int32_t m_count[8];
	int32_t m_output[8];
	bool m_enabled;
};

DECLARE_DEVICE_TYPE(T6W28, t6w28_device)

#endif // MAME_SOUND_T6W28_H
