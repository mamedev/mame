#pragma once

#ifndef __2413INTF_H__
#define __2413INTF_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( ym2413_w );

WRITE8_DEVICE_HANDLER( ym2413_register_port_w );
WRITE8_DEVICE_HANDLER( ym2413_data_port_w );

class ym2413_device : public device_t,
                                  public device_sound_interface
{
public:
	ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ym2413_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type YM2413;


#endif /* __2413INTF_H__ */
