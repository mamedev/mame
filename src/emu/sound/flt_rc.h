#pragma once

#ifndef __FLT_RC_H__
#define FLT_RC_H

#include "machine/rescap.h"
#include "devlegcy.h"

#define FLT_RC_LOWPASS		0
#define FLT_RC_HIGHPASS		1
#define FLT_RC_AC			2

/*
 * FLT_RC_LOWPASS:
 *
 * signal >--R1--+--R2--+
 *               |      |
 *               C      R3---> amp
 *               |      |
 *              GND    GND
 *
 * Set C=0 to disable filter
 *
 * FLT_RC_HIGHPASS:
 *
 * signal >--C---+----> amp
 *               |
 *               R1
 *               |
 *              GND
 *
 * Set C = 0 to disable filter
 *
 * FLT_RC_AC:
 *
 * Same as FLT_RC_HIGHPASS, but with standard frequency of 16 HZ
 * This filter may be setup just with
 *
 * MCFG_SOUND_ADD("tag", FILTER_RC, 0)
 * MCFG_SOUND_CONFIG(&flt_rc_ac_default)
 *
 * Default behaviour:
 *
 * Without MCFG_SOUND_CONFIG, a disabled FLT_RC_LOWPASS is created
 *
 */

typedef struct _flt_rc_config flt_rc_config;
struct _flt_rc_config
{
	int	type;
	double	R1;
	double	R2;
	double	R3;
	double	C;
};

extern const flt_rc_config flt_rc_ac_default;

void filter_rc_set_RC(device_t *device, int type, double R1, double R2, double R3, double C);

class filter_rc_device : public device_t,
                                  public device_sound_interface
{
public:
	filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~filter_rc_device() { global_free(m_token); }

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

extern const device_type FILTER_RC;


#endif /* __FLT_RC_H__ */
