/**********************************************************************

    speaker.h
    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/

#pragma once

#ifndef __SOUND_SPEAKER_H__
#define __SOUND_SPEAKER_H__

#include "devlegcy.h"

#define SPEAKER_TAG		"speaker"

typedef struct _speaker_interface speaker_interface;
struct _speaker_interface
{
	int num_level;	/* optional: number of levels (if not two) */
	const INT16 *levels;	/* optional: pointer to level lookup table */
};

void speaker_level_w (device_t *device, int new_level);

class speaker_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	speaker_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~speaker_sound_device() { global_free(m_token); }

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

extern const device_type SPEAKER_SOUND;


#endif /* __SPEAKER_H__ */
