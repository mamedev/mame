#pragma once

#ifndef __HC55516_H__
#define __HC55516_H__

#include "devlegcy.h"


/* sets the digit (0 or 1) */
void hc55516_digit_w(device_t *device, int digit);

/* sets the clock state (0 or 1, clocked on the rising edge) */
void hc55516_clock_w(device_t *device, int state);

/* returns whether the clock is currently LO or HI */
int hc55516_clock_state_r(device_t *device);

class hc55516_device : public device_t,
									public device_sound_interface
{
public:
	hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	hc55516_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~hc55516_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type HC55516;

class mc3417_device : public hc55516_device
{
public:
	mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type MC3417;

class mc3418_device : public hc55516_device
{
public:
	mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type MC3418;


#endif /* __HC55516_H__ */
