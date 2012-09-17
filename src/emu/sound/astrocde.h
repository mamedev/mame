#pragma once

#ifndef __ASTROCDE_H__
#define __ASTROCDE_H__

#include "devlegcy.h"

DECLARE_WRITE8_DEVICE_HANDLER( astrocade_sound_w );

class astrocade_device : public device_t,
                                  public device_sound_interface
{
public:
	astrocade_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~astrocade_device() { global_free(m_token); }

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

extern const device_type ASTROCADE;


#endif /* __ASTROCDE_H__ */
