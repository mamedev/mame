// license:???
// copyright-holders:Derrick Renaud, Couriersud
#pragma once

#ifndef __FLT_VOL_H__
#define __FLT_VOL_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_FILTER_VOLUME_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, FILTER_VOLUME, _clock)
#define MCFG_FILTER_VOLUME_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, FILTER_VOLUME, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> filter_volume_device

class filter_volume_device : public device_t,
								public device_sound_interface
{
public:
	filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~filter_volume_device() { }

	void flt_volume_set_volume(float volume);

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	sound_stream*  m_stream;
	int            m_gain;
};

extern const device_type FILTER_VOLUME;


#endif /* __FLT_VOL_H__ */
