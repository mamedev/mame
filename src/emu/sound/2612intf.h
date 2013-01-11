#pragma once

#ifndef __2612INTF_H__
#define __2612INTF_H__

#include "devlegcy.h"

void ym2612_update_request(void *param);

struct ym2612_interface
{
	void (*handler)(device_t *device, int irq);
};

DECLARE_READ8_DEVICE_HANDLER( ym2612_r );
DECLARE_WRITE8_DEVICE_HANDLER( ym2612_w );

DECLARE_READ8_DEVICE_HANDLER( ym2612_status_port_a_r );
DECLARE_READ8_DEVICE_HANDLER( ym2612_status_port_b_r );
DECLARE_READ8_DEVICE_HANDLER( ym2612_data_port_a_r );
DECLARE_READ8_DEVICE_HANDLER( ym2612_data_port_b_r );

DECLARE_WRITE8_DEVICE_HANDLER( ym2612_control_port_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2612_control_port_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2612_data_port_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( ym2612_data_port_b_w );


class ym2612_device : public device_t,
									public device_sound_interface
{
public:
	ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ym2612_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~ym2612_device() { global_free(m_token); }

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

extern const device_type YM2612;



struct ym3438_interface
{
	void (*handler)(device_t *device, int irq);
};


#define ym3438_r                ym2612_r
#define ym3438_w                ym2612_w

#define ym3438_status_port_a_r  ym2612_status_port_a_r
#define ym3438_status_port_b_r  ym2612_status_port_b_r
#define ym3438_data_port_a_r    ym2612_data_port_a_r
#define ym3438_data_port_b_r    ym2612_data_port_b_r

#define ym3438_control_port_a_w ym2612_control_port_a_w
#define ym3438_control_port_b_w ym2612_control_port_b_w
#define ym3438_data_port_a_w    ym2612_data_port_a_w
#define ym3438_data_port_b_w    ym2612_data_port_b_w


class ym3438_device : public ym2612_device
{
public:
	ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type YM3438;


#endif /* __2612INTF_H__ */
