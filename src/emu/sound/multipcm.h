#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

#include "devlegcy.h"

DECLARE_WRITE8_DEVICE_HANDLER( multipcm_w );
DECLARE_READ8_DEVICE_HANDLER( multipcm_r );

void multipcm_set_bank(device_t *device, UINT32 leftoffs, UINT32 rightoffs);

class multipcm_device : public device_t,
                                  public device_sound_interface
{
public:
	multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~multipcm_device() { global_free(m_token); }

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

extern const device_type MULTIPCM;


#endif /* __MULTIPCM_H__ */
