/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#pragma once

#ifndef __SEGAPCM_H__
#define __SEGAPCM_H__

#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGAPCM_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, SEGAPCM, _clock)
#define MCFG_SEGAPCM_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, SEGAPCM, _clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct sega_pcm_interface
{
	int  bank;
};

class segapcm_device : public device_t,
						public device_sound_interface
{
public:
	segapcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~segapcm_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( sega_pcm_w );
	DECLARE_READ8_MEMBER( sega_pcm_r );

private:
	UINT8* m_ram;
	UINT8 m_low[16];
	const UINT8* m_rom;
	int m_bankshift;
	int m_bankmask;
	int m_rgnmask;
	sound_stream* m_stream;
};

extern const device_type SEGAPCM;


#endif /* __SEGAPCM_H__ */
