#pragma once

#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "devlegcy.h"

#include "fm.h"
#include "ay8910.h"

void ym2608_update_request(void *param);

struct ym2608_interface
{
	const ay8910_interface ay8910_intf;
	void ( *handler )( device_t *device, int irq );	/* IRQ handler for the YM2608 */
};

READ8_DEVICE_HANDLER( ym2608_r );
WRITE8_DEVICE_HANDLER( ym2608_w );

READ8_DEVICE_HANDLER( ym2608_read_port_r );
READ8_DEVICE_HANDLER( ym2608_status_port_a_r );
READ8_DEVICE_HANDLER( ym2608_status_port_b_r );

WRITE8_DEVICE_HANDLER( ym2608_control_port_a_w );
WRITE8_DEVICE_HANDLER( ym2608_control_port_b_w );
WRITE8_DEVICE_HANDLER( ym2608_data_port_a_w );
WRITE8_DEVICE_HANDLER( ym2608_data_port_b_w );

class ym2608_device : public device_t,
                                  public device_sound_interface
{
public:
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ym2608_device() { global_free(m_token); }

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

extern const device_type YM2608;


#endif /* __2608INTF_H__ */
