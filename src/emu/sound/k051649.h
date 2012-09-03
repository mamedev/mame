#pragma once

#ifndef __K051649_H__
#define __K051649_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( k051649_waveform_w );
READ8_DEVICE_HANDLER ( k051649_waveform_r );
WRITE8_DEVICE_HANDLER( k051649_volume_w );
WRITE8_DEVICE_HANDLER( k051649_frequency_w );
WRITE8_DEVICE_HANDLER( k051649_keyonoff_w );
WRITE8_DEVICE_HANDLER( k051649_test_w );
READ8_DEVICE_HANDLER ( k051649_test_r );

WRITE8_DEVICE_HANDLER( k052539_waveform_w );
READ8_DEVICE_HANDLER ( k052539_waveform_r );

class k051649_device : public device_t,
                                  public device_sound_interface
{
public:
	k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051649_device() { global_free(m_token); }

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

extern const device_type K051649;


#endif /* __K051649_H__ */
