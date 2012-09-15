/***************************************************************************

    sid6581.h

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#pragma once

#ifndef __SID6581_H__
#define __SID6581_H__

#include "devlegcy.h"


typedef enum
{
	MOS6581,
	MOS8580
} SIDTYPE;

#define MOS6581_INTERFACE(name) \
	const sid6581_interface (name) =

struct sid6581_interface
{
	devcb_read8 in_potx_cb;
	devcb_read8 in_poty_cb;
};

class sid6581_device : public device_t,
                                  public device_sound_interface
{
public:
	sid6581_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sid6581_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~sid6581_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

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

extern const device_type SID6581;

class sid8580_device : public sid6581_device
{
public:
	sid8580_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SID8580;


#endif /* __SID6581_H__ */
