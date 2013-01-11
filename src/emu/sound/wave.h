#pragma once

#ifndef __WAVE_H__
#define __WAVE_H__

#include "devlegcy.h"

/*****************************************************************************
 *  CassetteWave interface
 *****************************************************************************/

class wave_device : public device_t,
									public device_sound_interface
{
public:
	wave_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
};

extern const device_type WAVE;


#define WAVE_TAG        "wave"
#define WAVE2_TAG       "wave2"


#define MCFG_SOUND_WAVE_ADD(_tag, _cass_tag) \
	MCFG_SOUND_ADD( _tag, WAVE, 0 ) \
	MCFG_DEVICE_CONFIG( _cass_tag )

#endif /* __WAVE_H__ */
