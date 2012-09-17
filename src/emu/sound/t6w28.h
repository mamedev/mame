#pragma once

#ifndef __T6W28_H__
#define __T6W28_H__

#include "devlegcy.h"

DECLARE_WRITE8_DEVICE_HANDLER( t6w28_w );

class t6w28_device : public device_t,
                                  public device_sound_interface
{
public:
	t6w28_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~t6w28_device() { global_free(m_token); }

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

extern const device_type T6W28;


#endif /* __T6W28_H__ */
