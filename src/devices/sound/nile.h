// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#pragma once

#ifndef __NILE_H__
#define __NILE_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NILE_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NILE, _clock)
#define MCFG_NILE_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, NILE, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define NILE_VOICES 8

// ======================> nile_device

class nile_device : public device_t,
					public device_sound_interface
{
public:
	nile_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nile_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE16_MEMBER( nile_snd_w );
	DECLARE_READ16_MEMBER( nile_snd_r );
	DECLARE_WRITE16_MEMBER( nile_sndctrl_w );
	DECLARE_READ16_MEMBER( nile_sndctrl_r );

private:
	sound_stream *m_stream;
	UINT8 *m_sound_ram;
	UINT16 m_sound_regs[0x80];
	int m_vpos[NILE_VOICES];
	int m_frac[NILE_VOICES];
	int m_lponce[NILE_VOICES];
	UINT16 m_ctrl;
};

extern const device_type NILE;


#endif /* __NILE_H__ */
