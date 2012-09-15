#pragma once

#ifndef __TMS36XX_H__
#define __TMS36XX_H__

#include "devlegcy.h"

/* subtypes */
#define MM6221AA    21      /* Phoenix (fixed melodies) */
#define TMS3615 	15		/* Naughty Boy, Pleiads (13 notes, one output) */
#define TMS3617 	17		/* Monster Bash (13 notes, six outputs) */

/* The interface structure */
struct tms36xx_interface
{
	int subtype;
	double decay[6];	/* decay times for the six harmonic notes */
	double speed;		/* tune speed (meaningful for the TMS3615 only) */
};

/* MM6221AA interface functions */
extern void mm6221aa_tune_w(device_t *device, int tune);

/* TMS3615/17 interface functions */
extern void tms36xx_note_w(device_t *device, int octave, int note);

/* TMS3617 interface functions */
extern void tms3617_enable_w(device_t *device, int enable);

class tms36xx_device : public device_t,
                                  public device_sound_interface
{
public:
	tms36xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tms36xx_device() { global_free(m_token); }

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

extern const device_type TMS36XX;


#endif /* __TMS36XX_H__ */
