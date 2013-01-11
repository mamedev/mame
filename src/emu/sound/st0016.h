#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__

#include "devlegcy.h"

struct st0016_interface
{
	UINT8 **p_soundram;
};

DECLARE_READ8_DEVICE_HANDLER( st0016_snd_r );
DECLARE_WRITE8_DEVICE_HANDLER( st0016_snd_w );

class st0016_device : public device_t,
									public device_sound_interface
{
public:
	st0016_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~st0016_device() { global_free(m_token); }

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

extern const device_type ST0016;


#endif /* __ST0016_H__ */
