#pragma once

#ifndef __NAMCO_H__
#define __NAMCO_H__

#include "devlegcy.h"

struct namco_interface
{
	int voices;     /* number of voices */
	int stereo;     /* set to 1 to indicate stereo (e.g., System 1) */
};

DECLARE_WRITE8_DEVICE_HANDLER( pacman_sound_enable_w );
DECLARE_WRITE8_DEVICE_HANDLER( pacman_sound_w );

void polepos_sound_enable(device_t *device, int enable);
DECLARE_READ8_DEVICE_HANDLER( polepos_sound_r );
DECLARE_WRITE8_DEVICE_HANDLER( polepos_sound_w );

void mappy_sound_enable(device_t *device, int enable);

DECLARE_WRITE8_DEVICE_HANDLER( namcos1_cus30_w );   /* wavedata + sound registers + RAM */
DECLARE_READ8_DEVICE_HANDLER( namcos1_cus30_r );

DECLARE_READ8_DEVICE_HANDLER( namco_snd_sharedram_r );
DECLARE_WRITE8_DEVICE_HANDLER( namco_snd_sharedram_w );

class namco_device : public device_t,
									public device_sound_interface
{
public:
	namco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	namco_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~namco_device() { global_free(m_token); }

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

extern const device_type NAMCO;

class namco_15xx_device : public namco_device
{
public:
	namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type NAMCO_15XX;

class namco_cus30_device : public namco_device
{
public:
	namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type NAMCO_CUS30;



#endif /* __NAMCO_H__ */
