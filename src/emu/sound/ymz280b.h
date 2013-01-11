/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __YMZ280B_H__
#define __YMZ280B_H__

#include "devlegcy.h"


struct ymz280b_interface
{
	void (*irq_callback)(device_t *device, int state);  /* irq callback */
	devcb_read8 ext_read;           /* external RAM read */
	devcb_write8 ext_write;     /* external RAM write */
};

DECLARE_READ8_DEVICE_HANDLER ( ymz280b_r );
DECLARE_WRITE8_DEVICE_HANDLER( ymz280b_w );

class ymz280b_device : public device_t,
									public device_sound_interface
{
public:
	ymz280b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ymz280b_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type YMZ280B;


#endif /* __YMZ280B_H__ */
