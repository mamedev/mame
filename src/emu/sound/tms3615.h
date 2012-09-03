#pragma once

#ifndef __TMS3615_H__
#define __TMS3615_H__

#include "devlegcy.h"

extern void tms3615_enable_w(device_t *device, int enable);

#define TMS3615_FOOTAGE_8	0
#define TMS3615_FOOTAGE_16	1

class tms3615_device : public device_t,
                                  public device_sound_interface
{
public:
	tms3615_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tms3615_device() { global_free(m_token); }

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

extern const device_type TMS3615;


#endif /* __TMS3615_H__ */
