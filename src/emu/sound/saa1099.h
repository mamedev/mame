#pragma once

#ifndef __SAA1099_H__
#define __SAA1099_H__

#include "devlegcy.h"

/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

DECLARE_WRITE8_DEVICE_HANDLER( saa1099_control_w );
DECLARE_WRITE8_DEVICE_HANDLER( saa1099_data_w );

class saa1099_device : public device_t,
                                  public device_sound_interface
{
public:
	saa1099_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~saa1099_device() { global_free(m_token); }

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

extern const device_type SAA1099;


#endif /* __SAA1099_H__ */
