/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#pragma once

#ifndef __IREMGA20_H__
#define __IREMGA20_H__

#include "devlegcy.h"

DECLARE_WRITE8_DEVICE_HANDLER( irem_ga20_w );
DECLARE_READ8_DEVICE_HANDLER( irem_ga20_r );

class iremga20_device : public device_t,
                                  public device_sound_interface
{
public:
	iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~iremga20_device() { global_free(m_token); }

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

extern const device_type IREMGA20;


#endif /* __IREMGA20_H__ */
