/* C140.h */

#pragma once

#ifndef __C140_H__
#define __C140_H__

#include "devlegcy.h"

READ8_DEVICE_HANDLER( c140_r );
WRITE8_DEVICE_HANDLER( c140_w );

void c140_set_base(device_t *device, void *base);

enum
{
	C140_TYPE_SYSTEM2,
	C140_TYPE_SYSTEM21,
	C140_TYPE_ASIC219
};

struct c140_interface {
    int banking_type;
};

class c140_device : public device_t,
                                  public device_sound_interface
{
public:
	c140_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~c140_device() { global_free(m_token); }

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

extern const device_type C140;


#endif /* __C140_H__ */
