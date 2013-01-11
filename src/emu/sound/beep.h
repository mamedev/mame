#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

#include "devlegcy.h"

#define BEEPER_TAG      "beeper"

void beep_set_state(device_t *device, int on);
void beep_set_frequency(device_t *device, int frequency);
void beep_set_volume(device_t *device, int volume);

class beep_device : public device_t,
									public device_sound_interface
{
public:
	beep_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~beep_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type BEEP;


#endif /* __BEEP_H__ */
