#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__

#include "devlegcy.h"


struct x1_010_interface
{
	int adr;	/* address */
};


READ8_DEVICE_HANDLER ( seta_sound_r );
WRITE8_DEVICE_HANDLER( seta_sound_w );

READ16_DEVICE_HANDLER ( seta_sound_word_r );
WRITE16_DEVICE_HANDLER( seta_sound_word_w );

void seta_sound_enable_w(device_t *device, int data);

class x1_010_device : public device_t,
                                  public device_sound_interface
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~x1_010_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type X1_010;


#endif /* __X1_010_H__ */
