#pragma once

#ifndef __MSM5232_H__
#define __MSM5232_H__

#include "devlegcy.h"

struct msm5232_interface
{
	double capacity[8]; /* in Farads, capacitors connected to pins: 24,25,26,27 and 37,38,39,40 */
	devcb_write_line gate_handler_cb; /* callback called when the GATE output pin changes state */
};

DECLARE_WRITE8_DEVICE_HANDLER( msm5232_w );

void msm5232_set_clock(device_t *device, int clock);

class msm5232_device : public device_t,
									public device_sound_interface
{
public:
	msm5232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~msm5232_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type MSM5232;


#endif /* __MSM5232_H__ */
