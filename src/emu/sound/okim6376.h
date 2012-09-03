#pragma once

#ifndef __OKIM6376_H__
#define __OKIM6376_H__

#include "devlegcy.h"

/* an interface for the OKIM6376 and similar chips (CPU interface only) */

READ8_DEVICE_HANDLER( okim6376_r );
WRITE8_DEVICE_HANDLER( okim6376_w );

class okim6376_device : public device_t,
                                  public device_sound_interface
{
public:
	okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~okim6376_device() { global_free(m_token); }

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

extern const device_type OKIM6376;


WRITE_LINE_DEVICE_HANDLER( okim6376_st_w );
WRITE_LINE_DEVICE_HANDLER( okim6376_ch2_w );

READ_LINE_DEVICE_HANDLER( okim6376_busy_r );
READ_LINE_DEVICE_HANDLER( okim6376_nar_r );

void okim6376_set_frequency(device_t *device, int frequency);

#endif /* __OKIM6376_H__ */
