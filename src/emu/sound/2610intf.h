#pragma once

#ifndef __2610INTF_H__
#define __2610INTF_H__

#include "devlegcy.h"
#include "fm.h"


void ym2610_update_request(void *param);

struct ym2610_interface
{
	void ( *handler )( device_t *device, int irq ); /* IRQ handler for the YM2610 */
};

DECLARE_READ8_DEVICE_HANDLER( ym2610_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym2610_w );

DECLARE_READ8_DEVICE_HANDLER( ym2610_status_port_a_r );
DECLARE_READ8_DEVICE_HANDLER( ym2610_status_port_b_r );
DECLARE_READ8_DEVICE_HANDLER( ym2610_read_port_r );

DECLARE_WRITE8_DEVICE_HANDLER( ym2610_control_port_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2610_control_port_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2610_data_port_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2610_data_port_b_w );


class ym2610_device : public device_t,
									public device_sound_interface
{
public:
	ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ym2610_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~ym2610_device() { global_free(m_token); }

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

extern const device_type YM2610;

class ym2610b_device : public ym2610_device
{
public:
	ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type YM2610B;


#endif /* __2610INTF_H__ */
