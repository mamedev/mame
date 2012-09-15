#pragma once

#ifndef __C6280_H__
#define __C6280_H__

#include "devlegcy.h"

struct c6280_interface
{
	const char *	cpu;
};

/* Function prototypes */
WRITE8_DEVICE_HANDLER( c6280_w );
READ8_DEVICE_HANDLER( c6280_r );

class c6280_device : public device_t,
                                  public device_sound_interface
{
public:
	c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~c6280_device() { global_free(m_token); }

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

extern const device_type C6280;


#endif /* __C6280_H__ */
