#pragma once

#ifndef __3812INTF_H__
#define __3812INTF_H__

#include "devlegcy.h"

struct ym3812_interface
{
	void (*handler)(device_t *device, int linestate);
};

DECLARE_READ8_DEVICE_HANDLER( ym3812_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym3812_w );

DECLARE_READ8_DEVICE_HANDLER( ym3812_status_port_r );
DECLARE_READ8_DEVICE_HANDLER( ym3812_read_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym3812_control_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym3812_write_port_w );

class ym3812_device : public device_t,
									public device_sound_interface
{
public:
	ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ym3812_device() { global_free(m_token); }

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

extern const device_type YM3812;


#endif /* __3812INTF_H__ */
