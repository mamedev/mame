#pragma once

#ifndef __VRENDER0_H__
#define __VRENDER0_H__

#include "devlegcy.h"


struct vr0_interface
{
	UINT32 RegBase;
};

void vr0_snd_set_areas(device_t *device,UINT32 *texture,UINT32 *frame);

READ32_DEVICE_HANDLER( vr0_snd_read );
WRITE32_DEVICE_HANDLER( vr0_snd_write );

class vrender0_device : public device_t,
                                  public device_sound_interface
{
public:
	vrender0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vrender0_device() { global_free(m_token); }

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

extern const device_type VRENDER0;


#endif /* __VRENDER0_H__ */
