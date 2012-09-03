#pragma once

#ifndef __GALELCO_H__
#define __GALELCO_H__

#include "devlegcy.h"

typedef struct _gaelcosnd_interface gaelcosnd_interface;
struct _gaelcosnd_interface
{
	const char *gfxregion;	/* shared gfx region name */
	int banks[4];			/* start of each ROM bank */
};

WRITE16_DEVICE_HANDLER( gaelcosnd_w );
READ16_DEVICE_HANDLER( gaelcosnd_r );

class gaelco_gae1_device : public device_t,
                                  public device_sound_interface
{
public:
	gaelco_gae1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	gaelco_gae1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~gaelco_gae1_device() { global_free(m_token); }

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

extern const device_type GAELCO_GAE1;

class gaelco_cg1v_device : public gaelco_gae1_device
{
public:
	gaelco_cg1v_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type GAELCO_CG1V;


#endif /* __GALELCO_H__ */
