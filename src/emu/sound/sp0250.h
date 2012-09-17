#pragma once

#ifndef __SP0250_H__
#define __SP0250_H__

#include "devlegcy.h"

struct sp0250_interface {
	void (*drq_callback)(device_t *device, int state);
};

DECLARE_WRITE8_DEVICE_HANDLER( sp0250_w );
UINT8 sp0250_drq_r(device_t *device);

class sp0250_device : public device_t,
                                  public device_sound_interface
{
public:
	sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~sp0250_device() { global_free(m_token); }

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

extern const device_type SP0250;


#endif /* __SP0250_H__ */
