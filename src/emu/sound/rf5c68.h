/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#pragma once

#ifndef __RF5C68_H__
#define __RF5C68_H__

#include "devlegcy.h"

/******************************************/
READ8_DEVICE_HANDLER( rf5c68_r );
WRITE8_DEVICE_HANDLER( rf5c68_w );

READ8_DEVICE_HANDLER( rf5c68_mem_r );
WRITE8_DEVICE_HANDLER( rf5c68_mem_w );

struct rf5c68_interface
{
	void (*sample_end_callback)(device_t* device, int channel);
};

class rf5c68_device : public device_t,
                                  public device_sound_interface
{
public:
	rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~rf5c68_device() { global_free(m_token); }

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

extern const device_type RF5C68;


#endif /* __RF5C68_H__ */
