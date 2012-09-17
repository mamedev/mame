#pragma once

#ifndef __YMF271_H__
#define __YMF271_H__

#include "devlegcy.h"


struct ymf271_interface
{
	devcb_read8 ext_read;		/* external memory read */
	devcb_write8 ext_write;	/* external memory write */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
};

DECLARE_READ8_DEVICE_HANDLER( ymf271_r );
DECLARE_WRITE8_DEVICE_HANDLER( ymf271_w );

class ymf271_device : public device_t,
                                  public device_sound_interface
{
public:
	ymf271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ymf271_device() { global_free(m_token); }

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

extern const device_type YMF271;


#endif /* __YMF271_H__ */
