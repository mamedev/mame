#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ST0016_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ST0016, _clock)
#define MCFG_ST0016_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ST0016, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct st0016_interface
{
	UINT8 **p_soundram;
};


// ======================> st0016_device

class st0016_device : public device_t,
						public device_sound_interface
{
public:
	st0016_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~st0016_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_READ8_MEMBER( st0016_snd_r );
	DECLARE_WRITE8_MEMBER( st0016_snd_w );

private:
	sound_stream *m_stream;
	UINT8 **m_sound_ram;
	int m_vpos[8];
	int m_frac[8];
	int m_lponce[8];
	UINT8 m_regs[0x100];
};

extern const device_type ST0016;


#endif /* __ST0016_H__ */
