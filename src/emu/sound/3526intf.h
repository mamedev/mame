#pragma once

#ifndef __3526INTF_H__
#define __3526INTF_H__

#include "devlegcy.h"

struct ym3526_interface
{
	devcb_write_line out_int_func;
};

DECLARE_READ8_DEVICE_HANDLER( ym3526_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym3526_w );

DECLARE_READ8_DEVICE_HANDLER( ym3526_status_port_r );
DECLARE_READ8_DEVICE_HANDLER( ym3526_read_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym3526_control_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym3526_write_port_w );

class ym3526_device : public device_t,
									public device_sound_interface
{
public:
	ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ym3526_device() { global_free(m_token); }

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

extern const device_type YM3526;


#endif /* __3526INTF_H__ */
