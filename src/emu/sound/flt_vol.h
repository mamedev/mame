#pragma once

#ifndef __FLT_VOL_H__
#define __FLT_VOL_H__

#include "devlegcy.h"


void flt_volume_set_volume(device_t *device, float volume);

class filter_volume_device : public device_t,
                                  public device_sound_interface
{
public:
	filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~filter_volume_device() { global_free(m_token); }

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

extern const device_type FILTER_VOLUME;


#endif /* __FLT_VOL_H__ */
