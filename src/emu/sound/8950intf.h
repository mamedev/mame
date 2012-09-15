#pragma once

#ifndef __8950INTF_H__
#define __8950INTF_H__

#include "devlegcy.h"

struct y8950_interface
{
	void (*handler)(device_t *device, int linestate);

	read8_device_func keyboardread;
	write8_device_func keyboardwrite;
	read8_device_func portread;
	write8_device_func portwrite;
};

READ8_DEVICE_HANDLER( y8950_r );
WRITE8_DEVICE_HANDLER( y8950_w );

READ8_DEVICE_HANDLER( y8950_status_port_r );
READ8_DEVICE_HANDLER( y8950_read_port_r );
WRITE8_DEVICE_HANDLER( y8950_control_port_w );
WRITE8_DEVICE_HANDLER( y8950_write_port_w );

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
};

extern const device_type Y8950;


#endif /* __8950INTF_H__ */
