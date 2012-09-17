/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#pragma once

#ifndef __QSOUND_H__
#define __QSOUND_H__

#include "devlegcy.h"

#define QSOUND_CLOCK    4000000   /* default 4MHz clock */

DECLARE_WRITE8_DEVICE_HANDLER( qsound_w );
DECLARE_READ8_DEVICE_HANDLER( qsound_r );

class qsound_device : public device_t,
                                  public device_sound_interface
{
public:
	qsound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~qsound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type QSOUND;


#endif /* __QSOUND_H__ */
