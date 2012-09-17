#pragma once

#ifndef __8950INTF_H__
#define __8950INTF_H__

#include "devlegcy.h"

struct y8950_interface
{
	devcb_write_line handler_cb;
	devcb_read8 keyboardread_cb;
	devcb_write8 keyboardwrite_cb;
	devcb_read8 portread_cb;
	devcb_write8 portwrite_cb;
};

DECLARE_READ8_DEVICE_HANDLER( y8950_r );
DECLARE_WRITE8_DEVICE_HANDLER( y8950_w );

DECLARE_READ8_DEVICE_HANDLER( y8950_status_port_r );
DECLARE_READ8_DEVICE_HANDLER( y8950_read_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( y8950_control_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( y8950_write_port_w );

class y8950_device : public device_t,
                                  public device_sound_interface
{
public:
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~y8950_device() { global_free(m_token); }

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
public:
	devcb_resolved_write_line m_handler;
	devcb_resolved_read8 m_keyboardread;
	devcb_resolved_write8 m_keyboardwrite;
	devcb_resolved_read8 m_portread;
	devcb_resolved_write8 m_portwrite;
};

extern const device_type Y8950;


#endif /* __8950INTF_H__ */
