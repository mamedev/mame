#pragma once

#ifndef __OKIM6258_H__
#define __OKIM6258_H__

#include "devlegcy.h"

/* an interface for the OKIM6258 and similar chips */

typedef struct _okim6258_interface okim6258_interface;
struct _okim6258_interface
{
	int divider;
	int adpcm_type;
	int output_12bits;
};


#define FOSC_DIV_BY_1024	0
#define FOSC_DIV_BY_768		1
#define FOSC_DIV_BY_512		2

#define TYPE_3BITS      	0
#define TYPE_4BITS			1

#define	OUTPUT_10BITS		0
#define	OUTPUT_12BITS		1

void okim6258_set_divider(device_t *device, int val);
void okim6258_set_clock(device_t *device, int val);
int okim6258_get_vclk(device_t *device);

READ8_DEVICE_HANDLER( okim6258_status_r );
WRITE8_DEVICE_HANDLER( okim6258_data_w );
WRITE8_DEVICE_HANDLER( okim6258_ctrl_w );

class okim6258_device : public device_t,
                                  public device_sound_interface
{
public:
	okim6258_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~okim6258_device() { global_free(m_token); }

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

extern const device_type OKIM6258;


#endif /* __OKIM6258_H__ */
