#pragma once

#ifndef __NILE_H__
#define __NILE_H__

#include "devlegcy.h"

DECLARE_WRITE16_DEVICE_HANDLER( nile_snd_w );
DECLARE_READ16_DEVICE_HANDLER( nile_snd_r );
DECLARE_WRITE16_DEVICE_HANDLER( nile_sndctrl_w );
DECLARE_READ16_DEVICE_HANDLER( nile_sndctrl_r );

class nile_device : public device_t,
                                  public device_sound_interface
{
public:
	nile_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nile_device() { global_free(m_token); }

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

extern const device_type NILE;


#endif /* __NILE_H__ */
