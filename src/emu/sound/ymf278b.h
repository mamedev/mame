#pragma once

#ifndef __YMF278B_H__
#define __YMF278B_H__

#include "devlegcy.h"

#define YMF278B_STD_CLOCK (33868800)			/* standard clock for OPL4 */


struct ymf278b_interface
{
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
};

READ8_DEVICE_HANDLER( ymf278b_r );
WRITE8_DEVICE_HANDLER( ymf278b_w );

class ymf278b_device : public device_t,
                                  public device_sound_interface
{
public:
	ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ymf278b_device() { global_free(m_token); }

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

extern const device_type YMF278B;


#endif /* __YMF278B_H__ */
