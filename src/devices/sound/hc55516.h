// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_HC55516_H
#define MAME_SOUND_HC55516_H

#pragma once

class hc55516_device : public device_t, public device_sound_interface
{
public:
	hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* sets the digit (0 or 1) */
	void digit_w(int digit);

	/* sets the clock state (0 or 1, clocked on the rising edge) */
	void clock_w(int state);

	/* returns whether the clock is currently LO or HI */
	int clock_state_r();

protected:
	hc55516_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void start_common(uint8_t _shiftreg_mask, int _active_clock_hi);

	// internal state
	sound_stream *m_channel;
	int     m_active_clock_hi;
	uint8_t   m_shiftreg_mask;

	uint8_t   m_last_clock_state;
	uint8_t   m_digit;
	uint8_t   m_new_digit;
	uint8_t   m_shiftreg;

	int16_t   m_curr_sample;
	int16_t   m_next_sample;

	uint32_t  m_update_count;

	double  m_filter;
	double  m_integrator;

	double  m_charge;
	double  m_decay;
	double  m_leak;

	inline int is_external_oscillator();
	inline int is_active_clock_transition(int clock_state);
	inline int current_clock_state();
	void process_digit();
};


class mc3417_device : public hc55516_device
{
public:
	mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


class mc3418_device : public hc55516_device
{
public:
	mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


DECLARE_DEVICE_TYPE(HC55516, hc55516_device)
DECLARE_DEVICE_TYPE(MC3417,  mc3417_device)
DECLARE_DEVICE_TYPE(MC3418,  mc3418_device)

#endif // MAME_SOUND_HC55516_H
