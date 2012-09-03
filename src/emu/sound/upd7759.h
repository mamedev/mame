#pragma once

#ifndef __UPD7759_H__
#define __UPD7759_H__

#include "devlegcy.h"

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software. */

#define UPD7759_STANDARD_CLOCK		XTAL_640kHz

typedef struct _upd7759_interface upd7759_interface;
struct _upd7759_interface
{
	void (*drqcallback)(device_t *device, int param);	/* drq callback (per chip, slave mode only) */
};

void upd7759_set_bank_base(device_t *device, offs_t base);

void upd7759_reset_w(device_t *device, UINT8 data);
void upd7759_start_w(device_t *device, UINT8 data);
int upd7759_busy_r(device_t *device);
WRITE8_DEVICE_HANDLER( upd7759_port_w );

class upd7759_device : public device_t,
                                  public device_sound_interface
{
public:
	upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd7759_device() { global_free(m_token); }

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

extern const device_type UPD7759;


#endif /* __UPD7759_H__ */
