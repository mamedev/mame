#pragma once

#ifndef __VLM5030_H__
#define __VLM5030_H__

#include "devlegcy.h"

struct vlm5030_interface
{
	int memory_size;    /* memory size of speech rom (0=memory region length) */
};

/* set speech rom address */
void vlm5030_set_rom(device_t *device, void *speech_rom);

/* get BSY pin level */
int vlm5030_bsy(device_t *device);
/* latch contoll data */
DECLARE_WRITE8_DEVICE_HANDLER( vlm5030_data_w );
/* set RST pin level : reset / set table address A8-A15 */
void vlm5030_rst (device_t *device, int pin );
/* set VCU pin level : ?? unknown */
void vlm5030_vcu(device_t *device, int pin );
/* set ST pin level  : set table address A0-A7 / start speech */
void vlm5030_st(device_t *device, int pin );

class vlm5030_device : public device_t,
                                  public device_sound_interface
{
public:
	vlm5030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vlm5030_device() { global_free(m_token); }

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

extern const device_type VLM5030;


#endif /* __VLM5030_H__ */
