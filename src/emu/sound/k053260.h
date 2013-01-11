/*********************************************************

    Konami 053260 PCM/ADPCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K053260_H__
#define __K053260_H__

#include "devlegcy.h"

struct k053260_interface {
	const char *rgnoverride;
	timer_expired_func irq;         /* called on SH1 complete cycle ( clock / 32 ) */
};


DECLARE_WRITE8_DEVICE_HANDLER( k053260_w );
DECLARE_READ8_DEVICE_HANDLER( k053260_r );

class k053260_device : public device_t,
									public device_sound_interface
{
public:
	k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053260_device() { global_free(m_token); }

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

extern const device_type K053260;


#endif /* __K053260_H__ */
