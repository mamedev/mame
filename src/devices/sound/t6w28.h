// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#pragma once

#ifndef __T6W28_H__
#define __T6W28_H__

class t6w28_device : public device_t,
									public device_sound_interface
{
public:
	t6w28_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void set_enable( bool enable );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

extern const device_type T6W28;


#endif /* __T6W28_H__ */
