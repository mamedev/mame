#pragma once

#ifndef __SNKWAVE_H__
#define __SNKWAVE_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( snkwave_w );

class snkwave_device : public device_t,
                                  public device_sound_interface
{
public:
	snkwave_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~snkwave_device() { global_free(m_token); }

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

extern const device_type SNKWAVE;


#endif /* __SNKWAVE_H__ */
