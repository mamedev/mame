#pragma once

#ifndef __S14001A_H__
#define __S14001A_H__

#include "devlegcy.h"

int s14001a_bsy_r(device_t *device);        /* read BUSY pin */
void s14001a_reg_w(device_t *device, int data);     /* write to input latch */
void s14001a_rst_w(device_t *device, int data);     /* write to RESET pin */
void s14001a_set_clock(device_t *device, int clock);     /* set VSU-1000 clock */
void s14001a_set_volume(device_t *device, int volume);    /* set VSU-1000 volume control */

class s14001a_device : public device_t,
									public device_sound_interface
{
public:
	s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s14001a_device() { global_free(m_token); }

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

extern const device_type S14001A;


#endif /* __S14001A_H__ */
