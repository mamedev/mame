#pragma once

#ifndef __262INTF_H__
#define __262INTF_H__

#include "devlegcy.h"


typedef struct _ymf262_interface ymf262_interface;
struct _ymf262_interface
{
	void (*handler)(device_t *device, int irq);
};


READ8_DEVICE_HANDLER( ymf262_r );
WRITE8_DEVICE_HANDLER( ymf262_w );

READ8_DEVICE_HANDLER ( ymf262_status_r );
WRITE8_DEVICE_HANDLER( ymf262_register_a_w );
WRITE8_DEVICE_HANDLER( ymf262_register_b_w );
WRITE8_DEVICE_HANDLER( ymf262_data_a_w );
WRITE8_DEVICE_HANDLER( ymf262_data_b_w );


class ymf262_device : public device_t,
                                  public device_sound_interface
{
public:
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ymf262_device() { global_free(m_token); }

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

extern const device_type YMF262;


#endif /* __262INTF_H__ */
