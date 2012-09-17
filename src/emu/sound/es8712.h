#pragma once

#ifndef __ES8712_H__
#define __ES8712_H__

#include "devlegcy.h"

/* An interface for the ES8712 ADPCM chip */

void es8712_play(device_t *device);
void es8712_set_bank_base(device_t *device, int base);
void es8712_set_frequency(device_t *device, int frequency);

DECLARE_WRITE8_DEVICE_HANDLER( es8712_w );

class es8712_device : public device_t,
                                  public device_sound_interface
{
public:
	es8712_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~es8712_device() { global_free(m_token); }

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

extern const device_type ES8712;


#endif /* __ES8712_H__ */
