/* Ricoh RF5C400 emulator */

#pragma once

#ifndef __RF5C400_H__
#define __RF5C400_H__

#include "devlegcy.h"

DECLARE_READ16_DEVICE_HANDLER( rf5c400_r );
DECLARE_WRITE16_DEVICE_HANDLER( rf5c400_w );

class rf5c400_device : public device_t,
									public device_sound_interface
{
public:
	rf5c400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~rf5c400_device() { global_free(m_token); }

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

extern const device_type RF5C400;


#endif /* __RF5C400_H__ */
