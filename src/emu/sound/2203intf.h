#pragma once

#ifndef __2203INTF_H__
#define __2203INTF_H__

#include "devlegcy.h"

#include "ay8910.h"

void ym2203_update_request(void *param);

struct ym2203_interface
{
	const ay8910_interface ay8910_intf;
	devcb_write_line irqhandler;
};

DECLARE_READ8_DEVICE_HANDLER( ym2203_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym2203_w );

DECLARE_READ8_DEVICE_HANDLER( ym2203_status_port_r );
DECLARE_READ8_DEVICE_HANDLER( ym2203_read_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym2203_control_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2203_write_port_w );

class ym2203_device : public device_t,
                                  public device_sound_interface
{
public:
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ym2203_device() { global_free(m_token); }

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

extern const device_type YM2203;


#endif /* __2203INTF_H__ */
