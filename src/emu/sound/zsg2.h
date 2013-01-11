/*
    ZOOM ZSG-2 custom wavetable synthesizer
*/

#pragma once

#ifndef __ZSG2_H__
#define __ZSG2_H__

DECLARE_READ16_DEVICE_HANDLER( zsg2_r );
DECLARE_WRITE16_DEVICE_HANDLER( zsg2_w );

struct zsg2_interface
{
	const char *samplergn;
};

class zsg2_device : public device_t,
									public device_sound_interface
{
public:
	zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~zsg2_device() { global_free(m_token); }

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

extern const device_type ZSG2;


#endif  /* __ZSG2_H__ */
