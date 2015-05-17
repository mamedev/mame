// license:GPL-2.0+
// copyright-holders:Ron Fries,Dan Boris
#pragma once

#ifndef __TIAINTF_H__
#define __TIAINTF_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_TIA_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TIA, _clock)
#define MCFG_SOUND_TIA_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, TIA, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tia_device

class tia_device : public device_t,
					public device_sound_interface
{
public:
	tia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tia_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( tia_sound_w );

private:
	sound_stream *m_channel;
	void *m_chip;
};

extern const device_type TIA;


#endif /* __TIAINTF_H__ */
